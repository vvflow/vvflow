vvplot(1) -- plot Vvflow simulation results
====

## SYNOPSIS

`vvplot` [<OPTIONS>] <INPUT> <TARGET>

## DESCRIPTION

## MAIN OPTIONS

  * -x `XMIN`,`XMAX`, -y `YMIN`,`YMAX` :
    X and Y axis range;
    when XMIN=XMAX or YMIN=YMAX their values will be claculated from --size flag;
    one of XMIN, XMAX, YMIN or YMAX may be NaN (then it is to be calculated from --size flag)
    (default: 0,0)

  * --size `W`x`H` :
    image size;
    either W or H may be zero, but only if both -x and -y are specified
    (default: 1280x720)

  * -B :
    plot bodies

  * -V, --V `SIZE` :
    plot vortex domains with circles of diameter SIZE;
    when SIZE is 0 dots are drawed instead of circles
    (in pixels, default:4)

  * -S, --S `SMIN`,`SMAX`,`SSTEP` :
    plot streamlines at given streamfunction constants
    (default: auto)

  * -P, --P `PMIN`,`PMAX` :
    plot pressure field with color range \[PMIN, PMAX\]
    (default: -1.5,1)

  * -G, --G `GMAX` :
    plot vorticity field with color range \[-GMAX, GMAX\]
    (default: auto)

## OTHER OPTIONS

  * -v, --version :
    print program version and exit

  * -h, --help :
    show this manpage and exit

  * --png, --tar :
    select the output format: PNG image itself or TAR archive
    with gnuplot script and binary data to be plotted
    (default: png)

  * --ref-xy {o,b,f} :
    reference frame: original, body, fluid (default: 'o'); affects options -x, -y

  * --ref-S {o,b,f} :
    streamlines reference frame: original, body, fluid;
    affects option -S
    (default: 'o')

  * --ref-P {s,o,b,f} :
    pressure calculation mode:
    's' - static pressure,
    'o','b','f' - dynamic pressure in original, body, fluid reference frame;
    affects option -P
    (default: 's')

  * --gray, --no-gray :
    plot image in grayscale (default: false)

  * --colorbox, --no-colorbox :
    draw colorbox on the bootom of the plot (default: false)

  * --timelabel, --no-timelabel :
    draw time label in top left corner (default: true)

  * --holder, --no-holder :
    draw body holder (default: false)

  * --spring, --no-spring :
    draw body spring (default: false)

  * --res-hi `RES_HI` :
    horizontal resolution of vorticity and pressure fields (default: 640)

  * --res-lo `RES_LO` :
    horizontal resolution of streamfunction field (default: 256)


## SEE ALSO
  vvcompose(1), vvflow(1), vvxtract(1)
