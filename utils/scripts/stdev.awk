#!/usr/bin/awk -f
BEGIN{
	if (ARGC<2) {
		print "Usage: mse.awk -v VAR=VALUE"
		print "Variables:"
		print "\tcol -- column, which mean square error is applied to (default: 2)"
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
		sum2+= $col**2;
		n++;
	}
}

END {
	nn = (n>0) ? 1.0/n : 0
	printf "%.6e\n", sqrt(sum2*nn-(sum*nn)^2)
}
