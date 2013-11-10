function INT_cleanup()
{
    kill `jobs -pr`    
    exit
}

trap INT_cleanup INT

PROJDIR=/home/bro3886/devel/gasudoku
BIN=$PROJDIR/bin/sudoku
CONF=.gaconfig

TESTDIR=$PROJDIR/tests

MUT_RATE="5 10 15"
CROSS_RATE="0 90 95"
POP_RETAIN="900 950 990"

SEEDS="100 200"

MAX_JOBS=7
N_JOBS=0

TEST_COUNT=0

for s in $SEEDS
do
	for m in $MUT_RATE
	do 
		for c in $CROSS_RATE
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
				for puzzle in $TESTDIR/*.txt
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
				done
			done
		done
	done
done

wait

