function INT_cleanup()
{
	pkill sudoku
    exit
}

trap INT_cleanup INT

gnu_script()
{
	cat <<EOF
	set term eps
	set output "gen-plot.eps"
	plot 'meta.log' using 1:2 title "Gen #$@"
EOF
}

NGEN=0
make
bin/sudoku $RANDOM 500 | tee mf.log &
sleep 5

while inotifywait -e modify meta.log
do
	NPOP=`grep -wc gen mf.log`
	echo Total no. of trials so far: $NGEN
	gnu_script $NGEN | gnuplot
	if [[ `jobs -pr | wc -l` == 0 ]]
	then
		break
	fi
done


