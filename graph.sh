function INT_cleanup()
{
	pkill sudoku
    exit
}

trap INT_cleanup INT

NGEN=0
gnu_script()
{
	cat <<EOF
	set term eps
	set output "gen-plot.eps"
	plot 'meta.log' using 0:1 title "$@"
EOF
}
bin/sudoku $RANDOM 100 < cases/evil.txt | tee mf.log &
while inotifywait -e close_write meta.log
do
	NGEN=`cat meta.log | wc -l`
	echo $NGEN
	gnu_script $NGEN | gnuplot
	if [[ `jobs -pr | wc -l` == 0 ]]
	then
		break
	fi
done


