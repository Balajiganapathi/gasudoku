#! /bin/bash

function INT_cleanup()
{
	pkill sudoku
	exit
}

trap INT_cleanup INT

PROJDIR=~/devel/gasudoku	# Set to the absolute path of the gasudoku directory.
BIN=$PROJDIR/bin/sudoku
CASEDIR=$PROJDIR/cases
CONF=.gaconfig

> hosts.txt

for i in `seq 1 75`
do
	ssh -xqn 10.105.12.$i 'hostname' >> hosts.txt 2>/dev/null
done

for i in `seq 1 108`
do
	ssh -xqn 10.105.11.$i 'hostname' >> hosts.txt 2>/dev/null
done

bin/sudoku $RANDOM 100 `wc -l hosts.txt` | tee mf.log &
sleep 5

mkdir -p hosts
cd hosts

#NJOBS=`cat requests.txt | wc -l`

tailf requests.txt | 
while read fifo seed fitter_parent single_crossover_rate mutation_rate pop_retain
do
	host=`head -1 hosts.txt`
	sed -i '1d' hosts.txt
	echo $host >> busy.txt
	mkdir $host
	cd $host
	sed 's/\(mutation_rate = \).*/\1'$mutation_rate'/' $PROJDIR/$CONF > $CONF
	sed -i 's/\(single_crossover_rate = \).*/\1'$single_crossover_rate'/' $CONF
	sed -i 's/\(pop_retain = \).*/\1'$pop_retain'/' $CONF
	sed -i 's/\(seed = \).*/\1'$seed'/' $CONF
	sed -i 's/\(fitter_parent = \).*/\1'$fitter_parent'/' $CONF
	ssh -xqf $host "cd ~/devel/gasudoku/hosts/$host; 
		$BIN < $CASEDIR/easy.txt | grep gen | grep -o '[0-9]+' >> easy.txt &;
		$BIN < $CASEDIR/hard.txt | grep gen | grep -o '[0-9]+' >> hard.txt &;
		$BIN < $CASEDIR/medium.txt | grep gen | grep -o '[0-9]+' >> medium.txt &;
		$BIN < $CASEDIR/evil.txt | grep gen | grep -o '[0-9]+' >> evil.txt &;
		wait; cat *.txt > $fifo" &
done

wait


