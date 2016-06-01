#!/usr/bin/awk -f
BEGIN{
    if (ARGC<2) {
        print "Usage: vvawk.ampl [-v VAR=VALUE] FILE" > "/dev/stderr"
        print "Find amplitude of a column" > "/dev/stderr"
        print "Variables:" > "/dev/stderr"
        print "\tcol -- column number to be processed (default: 2)" > "/dev/stderr"
        print "\tspan -- span of a window, in which the extremum is searched (default: 5)" > "/dev/stderr"
        print "\txmin, xmax -- X-axis range (default: -inf,+inf)" > "/dev/stderr"
        exit 0
    }
    col =  col ? col+0 : 2
    span = span ? span+0. : 5
    xmin = xmin ? xmin+0. : -1e400
    xmax = xmax ? xmax+0. : +1e400
    smax = smin = 0
    nmax = nmin = 0
}

{
    # круговой буффер
    mod = NR%span
    y[mod] = $col
    x[mod] = $1

    # пока буффер не наполнился - наполняем
    if(NR<span)
        next;

    # берём точку по центру окна
    c = (NR-span/2)%span
    yc = y[c]
    xc = x[c]

    # если абсцисса не попала в нужный диапазон - пропускаем
    if(xc<xmin)
        next;
    if(xc>xmax)
        exit;

    # проверяем экстремум
    ismin = ismax = 1
    for (i=0; i<span; i++) {
        if (i==c) continue;
        if (y[i] < yc) ismin = 0
        if (y[i] > yc) ismax = 0
        if (!ismin && !ismax)
            next;
    }

    #printf "%.6e %.6e\n", xc, yc;
    if (ismin) {nmin++; smin += yc;}
    if (ismax) {nmax++; smax += yc;}
}

END {
    amax = (nmax>0) ? smax/nmax : 0;
    amin = (nmin>0) ? smin/nmin : 0;
    printf "%.6e\n", amax - amin;
}
