#!/usr/bin/awk -f
BEGIN{
    if (ARGC<2) {
        print "Usage: movavg.awk [-v VAR=VALUE] FILE"
        print "Variables:"
        print "\tspan -- number of samples being averaged (default: 20)"
        print "\tcol -- column number to be averaged (default: 2)"
        exit 0
    }
    col =  col ? col+0 : 2
    span = span ? span+0 : 20
}

{
    # circular index
    mod = NR % span
    if (NR<=span) count++
    else sum -= array[mod]
    sum+= $col
    array[mod]=$col
    for (i=1; i<col; i++)
        printf "%s%s", $i, OFS
    print sum/count
}
