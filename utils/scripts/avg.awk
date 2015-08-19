#!/usr/bin/awk -f
BEGIN{
	if (ARGC<2) {
		print "Usage: avg.awk [-v VAR=VALUE] FILE"
		print "Variables:"
		print "\tcol -- column number to be averaged (default: 2)"
		print "\txmin, xmax -- X-axis range (default: -inf,+inf)"
		exit 0
	}
	col =  col ? col+0 : 2
	xmin = xmin ? xmin+0. : -1e400
	xmax = xmax ? xmax+0. : +1e400
}

{
	if($1>xmin && $1<xmax) {
		sum+= $col;
		n++;
	}
}

END {
	printf "%.6e\n", (n>0) ? sum/n : 0;
}
