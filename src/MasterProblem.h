/*
 * MasterProblem.h
 *
 *  Created on: 3 f������vr. 2015
 *      Author: samuel
 */

#ifndef MASTERPROBLEM_H_
#define MASTERPROBLEM_H_

/* Inheritance */
#include "Solver.h"

/* Tools include */
#include "MyTools.h"

/* My includes */
#include "Nurse.h"
#include "Modeler.h"

//-----------------------------------------------------------------------------
//
//  S t r u c t   R o t a t i o n
//
//  A rotation is a set of shifts for a set of consecutive days.
//  It has a cost and a dual cost (tbd).
//
//-----------------------------------------------------------------------------
enum CostType {TOTAL_COST, CONS_SHIFTS_COST, CONS_WORKED_DAYS_COST, COMPLETE_WEEKEND_COST, PREFERENCE_COST, INIT_REST_COST};

struct DualCosts{
public:

   DualCosts(vector< vector<double> > & workCosts, vector<double> & startWorkCosts, vector<double> & endWorkCosts, double workedWeekendCost, bool doNotCopy = true):
      workCosts_(workCosts), startWorkCosts_(startWorkCosts), endWorkCosts_(endWorkCosts), workedWeekendCost_(workedWeekendCost) {

	   if(!doNotCopy){
		   workCosts_.clear();
		   workCosts_ = Tools::appendVectors(workCosts_, workCosts);
		   startWorkCosts_.clear();
		   startWorkCosts_ = Tools::appendVectors(startWorkCosts_, startWorkCosts);
		   endWorkCosts_.clear();
		   endWorkCosts_ = Tools::appendVectors(endWorkCosts_, endWorkCosts);
	   }
   }

   // GETTERS
   //
   inline double dayShiftWorkCost(int day, int shift){return (workCosts_[day][shift]);}
   inline double startWorkCost(int day){return (startWorkCosts_[day]);}
   inline double endWorkCost(int day){return (endWorkCosts_[day]);}
   inline double workedWeekendCost(){return workedWeekendCost_;}


protected:

   // Indexed by : (day, shift) !! 0 = shift 1 !!
    vector< vector<double> > & workCosts_;

    // Indexed by : day
    vector<double> & startWorkCosts_;

    // Indexed by : day
    vector<double> & endWorkCosts_;

    // Reduced cost of the weekends
    double workedWeekendCost_;

};


struct Rotation {

   // Specific constructors and destructors
   //
   Rotation(map<int,int> shifts, LiveNurse* nurse = NULL, double cost = DBL_MAX, double dualCost = DBL_MAX) :
      shifts_(shifts), pNurse_(nurse), cost_(cost),id_(s_count),
      consShiftsCost_(0), consDaysWorkedCost_(0), completeWeekendCost_(0), preferenceCost_(0), initRestCost_(0),
      dualCost_(dualCost), length_(shifts.size())
   {
      ++s_count;
      firstDay_ = 999;
      for(map<int,int>::iterator itS = shifts.begin(); itS != shifts.end(); ++itS)
         if(itS->first < firstDay_) firstDay_ = itS->first;
   };

   Rotation(int firstDay, vector<int> shiftSuccession, LiveNurse* nurse = NULL, double cost = DBL_MAX, double dualCost = DBL_MAX) :
      pNurse_(nurse), cost_(cost),id_(s_count),
      consShiftsCost_(0), consDaysWorkedCost_(0), completeWeekendCost_(0), preferenceCost_(0), initRestCost_(0),
      dualCost_(dualCost), firstDay_(firstDay), length_(shiftSuccession.size())
   {
      ++s_count;
      for(int k=0; k<shiftSuccession.size(); k++) shifts_.insert(pair<int,int>( (firstDay+k) , shiftSuccession[k] ));
   }

   Rotation(Rotation& rotation, LiveNurse* pNurse) :
      pNurse_(pNurse), cost_(rotation.cost_),id_(rotation.id_),
      consShiftsCost_(rotation.consShiftsCost_), consDaysWorkedCost_(rotation.consDaysWorkedCost_),
      completeWeekendCost_(rotation.completeWeekendCost_), preferenceCost_(rotation.preferenceCost_), initRestCost_(rotation.initRestCost_),
      dualCost_(rotation.dualCost_), firstDay_(rotation.firstDay_), length_(rotation.length_), shifts_(rotation.shifts_)
   {
      if(rotation.pNurse_ != pNurse){
         cost_ = DBL_MAX;
         dualCost_ = DBL_MAX;
      }
   }

   ~Rotation(){};

   //count rotations
   //
   static unsigned int s_count;

   //Id of the rotation
   //
   long id_;

   //the nurse
   //
   LiveNurse* pNurse_;

   // Cost
   //
   double cost_;
   double consShiftsCost_ , consDaysWorkedCost_, completeWeekendCost_, preferenceCost_, initRestCost_ ;

   // Dual cost as found in the subproblem
   //
   double dualCost_;

   // Shifts to be performed
   //
   map<int,int> shifts_;

   // First worked day
   //
   int firstDay_;

   // Duration
   //
   int length_;

   //Compute the cost of a rotation
   //
   void computeCost(Scenario* pScenario, Preferences* pPreferences, int horizon);

   //Compute the dual cost of a rotation
   //
   void computeDualCost(DualCosts& costs);


   string toString(int nbDays){
      std::cout << "#   | ROTATION: N=" << pNurse_->id_ << "  cost=" << cost_ << "  dualCost=" << dualCost_ << "  firstDay=" << firstDay_ << "  length=" << length_ << std::endl;
      std::cout << "#               |";
      vector<int> allTasks (nbDays);
      for(map<int,int>::iterator itTask = shifts_.begin(); itTask != shifts_.end(); ++itTask)
         allTasks[itTask->first] = itTask->second;
      for(int i=0; i<allTasks.size(); i++){
         if(allTasks[i] < 1) std::cout << " |";
         else std::cout << allTasks[i] << "|";
      }
      std::cout << std::endl;
   }

   //Compare rotations on index
   //
   static bool compareId(const Rotation& rot1, const Rotation& rot2);

   //Compare rotations on cost
   //
   static bool compareCost(const Rotation& rot1, const Rotation& rot2);

   //Compare rotations on dual cost
   //
   static bool compareDualCost(const Rotation& rot1, const Rotation& rot2);
};


//-----------------------------------------------------------------------------
//
//  C l a s s   M a s t e r P r o b l e m
//
// Build and solve the master problem of the column generation scheme
//
//-----------------------------------------------------------------------------

enum MySolverType { S_SCIP, S_BCP, S_CBC };

class MasterProblem : public Solver, public PrintSolution{
   //allows RotationPricer to access all private arguments and methods of MasterProblem
   friend class RotationPricer;
   friend class DiveBranchingRule;
public:
   // Specific constructor and destructor
   MasterProblem(Scenario* pScenario, Demand* pDemand,
      Preferences* pPreferences, vector<State>* pInitState, MySolverType solver);
   ~MasterProblem();

   //solve the rostering problem
   double solve(vector<Roster> solution = {});

   //solve the rostering problem or just the relaxation(root node)
   double solve(vector<Roster> solution, bool rebuild);

   // Solve with parameters
   double solve(SolverParam parameters, vector<Roster> solution = {});

   //Resolve the problem with another demand and keep the same preferences
   //
   double resolve(Demand* pDemand, SolverParam parameters, vector<Roster> solution = {});

   //get the pointer to the model
   Modeler* getModel(){
      return pModel_;
   }

   //override PrintSolution virtual method
   void save(vector<int>& weekIndices, string outdir);


   //get a reference to the rotations
   inline vector< map<MyVar*, Rotation> >& getRotations(){
      return rotations_;
   }

   //get a reference to the restsPerDay_ for a Nurse
   inline vector< vector<MyVar*> >& getRestsPerDay(Nurse* pNurse){
      return restsPerDay_[pNurse->id_];
   }

   /*
    * Solving parameterdoubles
    */
   const char* PB_NAME = "GenCol";
   int solvingTime;

private:
   Modeler* pModel_;
   vector2D positionsPerSkill_;//link positions to skills
   vector2D skillsPerPosition_;//link skills to positions
   MyPricer* pPricer_;//prices the rotations
   MyBranchingRule* pRule_; //choose the variables on which we should branch
   MySolverType solverType_; //which solver is used

   vector< map<MyVar*, Rotation> > rotations_;//stores the variables and the rotations for each nurse
   vector< vector< vector<MyVar*> > > restsPerDay_; //stores all the arcs that are resting on a day for each nurse

   /*
    * Variables
    */
   vector< vector<MyVar*> > columnVars_; //binary variables for the columns

   vector< vector<MyVar*> > restingVars_; //binary variables for the resting arcs in the rotation network
   vector< vector< vector<MyVar*> > > longRestingVars_; //binary variables for the resting arcs in the rotation network

   vector<MyVar*> minWorkedDaysVars_; //count the number of missing worked days per nurse
   vector<MyVar*> maxWorkedDaysVars_; //count the number of exceeding worked days per nurse
   vector<MyVar*> maxWorkedWeekendVars_; //count the number of exceeding worked weekends per nurse

   vector<MyVar*> minWorkedDaysAvgVars_; //count the number of missing worked days from average per nurse
   vector<MyVar*> maxWorkedDaysAvgVars_; // count the number of exceeding worked days from average per nurse
   vector<MyVar*> maxWorkedWeekendAvgVars_; //count the number of exceeding worked weekends from average per nurse

   vector<MyVar*> minWorkedDaysContractAvgVars_; //count the number of missing worked days from average per contract
   vector<MyVar*> maxWorkedDaysContractAvgVars_; // count the number of exceeding worked days from average per contract
   vector<MyVar*> maxWorkedWeekendContractAvgVars_; //count the number of exceeding worked weekends from average per contract

   vector< vector< vector<MyVar*> > > optDemandVars_; //count the number of missing nurse to reach the optimal
   vector< vector< vector<MyVar*> > > numberOfNursesByPositionVars_; // count the number of nurses by position on each day, shift
   vector< vector< vector< vector<MyVar*> > > > skillsAllocVars_; //makes the allocation of the skills

   /*
    * Constraints
    */
   //transmission of the flow on the resting nodes
   //initialization of the flow constraint at the first position of each restFlowCons_[i] (i=nurse)
   vector< vector<MyCons*> > restFlowCons_;
   //transmission of the flow on the working nodes
   //end of the flow constraint at the last position of each workFlowCons_[i] (i=nurse)
   vector< vector<MyCons*> > workFlowCons_;

   vector<MyCons*> minWorkedDaysCons_; //count the number of missing worked days per nurse
   vector<MyCons*> maxWorkedDaysCons_; //count the number of exceeding worked days per nurse
   vector<MyCons*> maxWorkedWeekendCons_; //count the number of exceeding worked weekends per nurse

   vector<MyCons*> minWorkedDaysAvgCons_; //count the number of missing worked days from average per nurse
   vector<MyCons*> maxWorkedDaysAvgCons_; // count the number of exceeding worked days from average per nurse
   vector<MyCons*> maxWorkedWeekendAvgCons_; //count the number of exceeding worked weekends from average per nurse

   vector<MyCons*> minWorkedDaysContractAvgCons_; //count the number of missing worked days from average per contract
   vector<MyCons*> maxWorkedDaysContractAvgCons_; // count the number of exceeding worked days from average per contract
   vector<MyCons*> maxWorkedWeekendContractAvgCons_; //count the number of exceeding worked weekends from average per contract

   vector< vector< vector<MyCons*> > > minDemandCons_; //ensure a minimal coverage per day, per shift, per skill
   vector< vector< vector<MyCons*> > > optDemandCons_; //count the number of missing nurse to reach the optimal
   vector< vector< vector<MyCons*> > > numberOfNursesByPositionCons_; //ensure there are enough nurses for numberOfNursesByPositionVars_
   vector< vector< vector<MyCons*> > > feasibleSkillsAllocCons_; // ensures that each nurse works with the good skill

   // vectors of booleans indicating whether some above constraints are present
   // in the model
   vector<bool> isMinWorkedDaysAvgCons_;
   vector<bool> isMaxWorkedDaysAvgCons_;
   vector<bool> isMaxWorkedWeekendAvgCons_;

   vector<bool> isMinWorkedDaysContractAvgCons_, isMaxWorkedDaysContractAvgCons_, isMaxWorkedWeekendContractAvgCons_;


   /*
    * Methods
    */

   // Initialize the solver at construction
   void initializeSolver(MySolverType solverType);

   // Main method to build the rostering problem for a given input
   void build();

   //Initialization of the rostering problem with/without solution
   void initialize(vector<Roster> solution);

   //solve method to catch execption
   void solveWithCatch();

   //solve a solution in the output
   void storeSolution();

   //Create a new rotation variable
   //add the correct constraints and coefficients for the nurse i working on a rotation
   //if s=-1, the nurse works on all shifts
   //store the rotation in rotations_
   void addRotation(Rotation& rotation, char* baseName);

   //compute and add the last rotation finishing on the day just before the first one
   Rotation computeInitStateRotation(LiveNurse* pNurse);

   //update the demand with a new one of the same size
   //change the rhs of the constraints minDemandCons_ and optDemandCons_
   void updateDemand(Demand* pDemand);

   //get the cost of all shosen rotations in solution sol for a certain CostType
   double getRotationCosts(CostType costType = TOTAL_COST, bool initStateRotation = false);

   /* Build each set of constraints - Add also the coefficient of a column for each set */
   void buildRotationCons();
   int addRotationConsToCol(vector<MyCons*>& cons, vector<double>& coeffs, int i, int k, bool firstDay, bool lastDay);
   void buildMinMaxCons();
   int addMinMaxConsToCol(vector<MyCons*>& cons, vector<double>& coeffs, int i, int nbDays, int nbWeekends);
   void buildSkillsCoverageCons();
   int addSkillsCoverageConsToCol(vector<MyCons*>& cons, vector<double>& coeffs, int i, int k, int s=-1);

   /* Display functions */
   string costsConstrainstsToString();
};

#endif /* MASTERPROBLEM_H_ */
