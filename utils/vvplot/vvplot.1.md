vvplot(1) -- plot Vvflow simulation results
====

## SYNOPSIS

**vvplot** [ _options_ ] _input_ _target_

## DESCRIPTION

## MAIN OPTIONS

  * **-x** _XMIN_,_XMAX_, **-y** _YMIN_,_YMAX_ :
    X and Y axis range;
    when _XMIN_=_XMAX_ or _YMIN_=_YMAX_ their values will be claculated
    from **--size** flag; one of _XMIN_, _XMAX_, _YMIN_ or _YMAX_ may be
    **NaN** (then it is to be calculated from **--size** flag)
    (default: 0,0)

  * **--size** _WxH_ :
    image size;
    either _W_ or _H_ may be zero, but only if both **-x** and **-y**
    are specified
    (default: 1280x720)

  * **-B** :
    plot bodies

  * **-V, --V** _SIZE_ :
    plot vortex domains with circles of diameter _SIZE_;
    when _SIZE_ is 0 dots are drawed instead of circles
    (in pixels, default: 4)

  * **-S, --S** _SMIN_,_SMAX_,_SSTEP_ :
    plot streamlines at given streamfunction constants
    (default: auto)

  * **-P, --P** _PMIN_,_PMAX_ :
    plot pressure field with color range \[_PMIN_, _PMAX_\]
    (default: -1.5,1)

  * **-G, --G** _GMAX_ :
    plot vorticity field with color range \[-_GMAX_, _GMAX_\]
    (default: auto)

  * **-Ux, -Uy, --Ux** _UMIN_,_UMAX_, **--Uy** _UMIN_,_UMAX_ :
    plot velocity field with color range \[UMIN, UMAX\]
    (default: auto)

## OTHER OPTIONS

  * **-v, --version** :
    print program version and exit

  * **-h, --help** :
    print help and exit

  * **--png, --tar** :
    select the output format: PNG image itself or TAR archive
    with gnuplot script and binary data to be plotted
    (default: png)

  * **--ref-xy** {o,b,bx,by,f} :
    reference frame: original, body axis (x, y or both), fluid;
    affects options **-x, -y**
    (default: 'o');

  * **--ref-S** {o,b,f} :
    streamlines reference frame: original, body, fluid;
    affects option **-S**
    (default: 'o')

  * **--ref-P** {s,o,b,f} :
    pressure calculation mode:
    's' - static pressure,
    'o','b','f' - dynamic pressure in original, body, fluid reference frame;
    affects option **-P**
    (default: 's')

  * **--gray, --no-gray** :
    plot image in grayscale (default: false)

  * **--colorbox, --no-colorbox** :
    draw colorbox on the bootom of the plot (default: false)

  * **--timelabel, --no-timelabel** :
    draw time label in top left corner (default: true)

  * **--holder, --no-holder** :
    draw body holder (default: false)

  * **--spring, --no-spring** :
    draw body spring (default: false)

  * **--res-hi** _RES\_HI_ :
    horizontal resolution of vorticity and pressure fields (default: 640)

  * **--res-lo** _RES\_LO_ :
    horizontal resolution of streamfunction field (default: 256)

  * **--load-field** _FILE_ :
    load field (either **-S** or **-U**) from file. Necessary for side
    processing of that field.


## SEE ALSO
  vvcompose(1), vvflow(1), vvxtract(1)
