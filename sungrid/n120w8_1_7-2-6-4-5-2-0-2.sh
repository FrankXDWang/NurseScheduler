#!/bin/bash -l
#$ -cwd
#$ -j y
#$ -o /dev/null
#$ -q idra
#
# optimal script: launch optimal solver and then the validator

prefix="-1"
if [ ! -z "$1" ]
then	prefix=$1
fi
./bin/optimalRoster n120w8 1 8 7 2 6 4 5 2 0 2 n120w8_1_7-2-6-4-5-2-0-2 $prefix $2 $3 $4 $5 > outfiles/Competition/n120w8_1_7-2-6-4-5-2-0-2/${1}Log.txt

instance=n120w8
weeksValue=(7 2 6 4 5 2 0 2 )

demand0="WD-${instance}-"
solutionFile="outfiles/Competition/n120w8_1_7-2-6-4-5-2-0-2/${1}sol-week"
weeks=""
sols=""
i=0

for var in ${weeksValue[*]}
do
demand[$i]="datasets/${instance}/${demand0}${var}.txt"
weeks="${weeks} ${demand[$i]}"
solution[$i]="${solutionFile}${i}.txt"
sols="${sols} ${solution[$i]}"
((i++))
done

java -jar validator.jar --sce datasets/n120w8/Sc-n120w8.txt --his datasets/n120w8/H0-n120w8-1.txt --weeks $weeks --sols $sols > outfiles/Competition/n120w8_1_7-2-6-4-5-2-0-2/${1}ValidatorOutput.txt 

exit 0;
