//
//  main_test.cpp
//  RosterDesNurses
//
//  Created by J��r��my Omer on 04/03/2015.
//  Copyright (c) 2015 J��r��my Omer. All rights reserved.
//

#include "main_test.h"
#include "ReadWrite.h"
#include "Greedy.h"
#include "SubProblem.h"
#include "MyTools.h"

/* scip includes */
#include "objscip/objscip.h"
#include "objscip/objscipdefplugins.h"

void main_test()
{
	testFunction_Antoine();
	// testFunction_Jeremy();
	//testFunction_Samuel();
}

// Function for testing parts of the code (Antoine)
void testFunction_Antoine(){

   // Time the complete execution of the algorithm
   Tools::Timer* timertotal = new Tools::Timer();
   timertotal->init();
   timertotal->start();

   // Create a log file
   string logFile = "logs/test.log";
   Tools::LogOutput logStream(logFile);

   //Create an output file
   string outFile = "outfiles/test.out";
   Tools::LogOutput outStream(outFile);

   string data = "testdatasets/";// testdatasets datasets
   string inst = "n005w4";// n100w4 n030w4 n005w4

   string scenarPath = data + inst + "/Sc-" + inst + ".txt";
   vector<int> numberWeek = {1, 2, 3, 3};
   vector<string> weekPaths(numberWeek.size());
   for(int i=0; i<numberWeek.size(); ++i){
      string path = data + inst + "/WD-" + inst + "-"+std::to_string(numberWeek[i])+".txt";
      weekPaths[i] = path;
   }
   string firstHistoryPath = data + inst + "/H0-" + inst + "-0.txt";

   // Read the input data from files
   Scenario* pScen = ReadWrite::readScenario(scenarPath);
   Demand* pWeekDemand = NULL;
   Preferences preferences;
   for(string demandPath: weekPaths)
      if(pWeekDemand == NULL){
         pWeekDemand = ReadWrite::readWeek(demandPath, pScen);
         preferences = *pScen->pWeekPreferences();
      }
      else{
         pWeekDemand->push_back(ReadWrite::readWeek(demandPath, pScen));
         preferences.push_back(pScen->pWeekPreferences());
         pScen->addAWeek();
      }
   pScen->linkWithDemand(pWeekDemand);
   pScen->linkWithPreferences(preferences);
   ReadWrite::readHistory(firstHistoryPath,pScen);

   // Check that the scenario was read properly
   //
   // logStream << *pScen << std::endl;
   logStream << pScen->toString() << std::endl;
   logStream << pWeekDemand->toString(true) << std::endl;

   // Write the aggregate information on the demand
   //


   // Write aggregate information on the cover capacity of the staff
   // (TBD)

   //Compute initial solution
   //
   Greedy* pGreedy =
      new Greedy(pScen, pWeekDemand,   pScen->pWeekPreferences(), pScen->pInitialState());
   pGreedy->constructiveGreedy();
   outStream << pGreedy->solutionToLogString();

   // Instantiate the solver class as a test
   //
   MasterProblem* pSolverTest =
      new MasterProblem(pScen, pWeekDemand,   pScen->pWeekPreferences(), pScen->pInitialState(), S_BCP, pGreedy->getSolution());
   pSolverTest->solve();

   // Write the solution in an output file
   outStream << pSolverTest->solutionToLogString();

   // Display the total time spent in the algorithm
   timertotal->stop();
   logStream.print("Total time spent in the algorithm : ");
   logStream.print(timertotal->dSinceInit());
   logStream.print("\n");


   //test vrp example of scip
   //   string dataFile = "datasets/vrp/eil22.vrp";
   //   Vrp* vrp = new Vrp(dataFile);

   // free the allocated pointers
   //
   //   delete vrp;
   delete timertotal;
   delete pWeekDemand;
   delete pScen;
   delete pGreedy;
   delete pSolverTest;
}


// Function for testing parts of the code (Jeremy)
void testFunction_Jeremy(){

   // Time the complete execution of the algorithm
   Tools::Timer* timertotal = new Tools::Timer();
   timertotal->init();
   timertotal->start();

   // Create a log file
   string logFile = "../logfiles/test.log";
   Tools::LogOutput logStream(logFile);


   // Read the input data from files
   Scenario* pScen = ReadWrite::readScenario("datasets/n030w4/Sc-n030w4.txt");
   Demand* pWeekDemand = ReadWrite::readWeek("datasets/n030w4/WD-n030w4-1.txt", pScen);
   ReadWrite::readHistory("datasets/n030w4/H0-n030w4-0.txt",pScen);

   // Check that the scenario was read properly
   //
   // logStream << *pScen << std::endl;
   logStream << pScen->toString() << std::endl;
   logStream << pWeekDemand->toString(true) << std::endl;

   // Write the aggregate information on the demand
   //


   // Write aggregate information on the cover capacity of the staff
   // (TBD)

   // Instantiate the solver class as a test
   //
   Greedy* pSolverTest =
      new Greedy(pScen, pWeekDemand,	pScen->pWeekPreferences(), pScen->pInitialState());
   pSolverTest->constructiveGreedy();

   // Write the solution in the required output format
   string outFile = "outfiles/solution.out";
   Tools::LogOutput outStream(outFile);
   outStream << pSolverTest->solutionToString();

   // Write the solution and advanced information in a more convenient format
   string outLog = "outfiles/log.out";
   Tools::LogOutput outLogStream(outLog);
   outLogStream << pSolverTest->solutionToLogString();

   // Display the total time spent in the algorithm
   timertotal->stop();
   logStream.print("Total time spent in the algorithm : ");
   logStream.print(timertotal->dSinceInit());
   logStream.print("\n");

   // free the allocated pointers
   //
   delete timertotal;
   delete pWeekDemand;
   delete pScen;
   delete pSolverTest;


}

// Function for testing parts of the code (Samuel)
void testFunction_Samuel(){

   Tools::Timer* timertest = new Tools::Timer();
   timertest->init();
   timertest->start();

   string inst = "n100w4";

   string scenar = "/home/samuel/Dropbox/Nurse Rostering Competition/Data/datasets_txt/" + inst + "/Sc-" + inst + ".txt";
   string firstWeek = "/home/samuel/Dropbox/Nurse Rostering Competition/Data/datasets_txt/" + inst + "/WD-" + inst + "-1.txt";
   string firstHistory = "/home/samuel/Dropbox/Nurse Rostering Competition/Data/datasets_txt/" + inst + "/H0-" + inst + "-0.txt";

   Scenario * s = ReadWrite::readScenario(scenar);
   Demand* pWeekDemand = ReadWrite::readWeek(firstWeek,s);
   ReadWrite::readHistory(firstHistory,s);

   cout << s->toString() << endl;

   timertest->stop();

   string logFile = "../logfiles/samuel_test.log";
   Tools::LogOutput logStream(logFile);

   // RqJO : attention, j'ai enlev�� ta surcharge de << parce qu'elle me faisait
   // des segfaults
   logStream << s->toString() << std::endl;
   logStream.print("Total time spent in the algorithm : ");
   logStream.print(timertest->dSinceInit());
   logStream.print("\n");

   for(map<string,Contract*>::const_iterator it = s->contracts_.begin(); it != s->contracts_.end(); ++it){
      const Contract * c = it->second;
      cout << "# " << endl;
      cout << "# " << endl;
      cout << "# +----------------------------------------------------------------------------------------" << endl;
      cout << "# CONTRACT : " << ((Contract *) c)->toString() << endl;
      cout << "# +----------------------------------------------------------------------------------------" << endl;
      SubProblem sp (s, pWeekDemand, c);
      cout << "# +----------------------------------------------------------------------------------------" << endl;
      cout << "# " << endl;
      cout << "# " << endl;
   }

   //SubProblem sp;
   //sp.testGraph_spprc();
}
