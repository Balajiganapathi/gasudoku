#! /bin/bash

function INT_cleanup()
{
    kill `jobs -pr`    
    exit
}

trap INT_cleanup INT

PROJDIR=$PWD	# Set to the absolute path of the gasudoku directory.
BIN=$PROJDIR/bin/sudoku$1
CONF=.gaconfig

TESTDIR=$PROJDIR/tests$1
CASEDIR=$PROJDIR/cases

mkdir -p $TESTDIR

if [[ -x $BIN ]]
then
	make `basename $BIN`
fi

MUT_RATES="5 10 15"
CROSS_RATES="0 90 95"
POP_RETAIN="900 950 990"

SEEDS="100 200"

MAX_JOBS=3
N_JOBS=0

TEST_COUNT=0

for s in $SEEDS
do
	for m in $MUT_RATES
	do 
		for c in $CROSS_RATES
		do
			for p in $POP_RETAIN
			do
				OUTDIR=$TESTDIR/$s/$m/$c/$p
				mkdir -p $OUTDIR
				cd $OUTDIR

				sed 's/\(mutation_rate = \).*/\1'$m'/' $PROJDIR/$CONF > $CONF
				sed -i 's/\(single_crossover_rate = \).*/\1'$c'/' $CONF
				sed -i 's/\(pop_retain = \).*/\1'$p'/' $CONF
				sed -i 's/\(seed = \).*/\1'$s'/' $CONF
				
				for puzzle in $CASEDIR/*.txt
				do
					OUT=`basename $puzzle`
					
					N_JOBS=`jobs -pr | wc -l`
					while [[ $N_JOBS -gt $MAX_JOBS ]]
					do
						sleep 10
						N_JOBS=`jobs -pr | wc -l`
					done

					TEST_COUNT=$(( TEST_COUNT + 1))
					echo -e $TEST_COUNT "\t" $PWD

					echo "seed = $s" >$OUT
					$BIN < $puzzle >solution_$OUT 2>>$OUT &
				done # Difficulty loop
			done # Population retain rate
		done # Crossover rate
	done # Mutation rate
done # Seed value

wait

