vvflow(1) -- solve the CFD problem
====

## SYNOPSIS

`vvflow` [_options_] _file_

## DESCRIPTION

**vvflow** is the main tool of the Vvflow CFD Suite.
It performs the simulation and solves the CFD problem.

Problem statement is loaded from _file_ - a Lua script specifying the
CFD problem. For the backward compatibility the _file_ can also be an
hdf5 file copied from another simulation results.

## OPTIONS

  * -v, --version :
    print program version and exit

  * -h, --help :
    print help and exit

  * --progress :
    print progress during the simulation

  * --profile :
    save pressure and friction profiles along bodies surfaces to the stepdata file

## LUA SCRIPT

The CFD problem is defined with a script in Lua programming language.

The script is loaded from _file_, which consists of regular Lua commands.

Example:

    #!/usr/bin/env vvflow

    S.inf_vx = "1"
    S.re = 600
    S.dt = 0.05
    S.dt_save = 0.5
    S.finish = 20

    local cyl = gen_cylinder{R=0.5, N=350}
    S.body_list:insert(cyl)

    S.caption = "re600_n350"
    S:save(S.caption..".h5")

This manual will not discuss the Lua syntax in all details, although it
will cover several topics which are essential for using vvflow. The Lua
basics can be found at various online resources. Here is a couple of
them:

[Programming in Lua](https://www.lua.org/pil/1.html)

[Learn X in Y minutes where X is Lua](https://learnxinyminutes.com/docs/lua/)

[Lua math](http://lua-users.org/wiki/MathLibraryTutorial)

[Tutorials collection](http://lua-users.org/wiki/TutorialDirectory)

## LUA BINDINGS REFERENCE

Technically speaking, **vvflow** is an extended Lua interpreter. It
supports everything that Lua supports. Plus it provides bindings for the
VVD classes (structures) and functions.

Vvflow defines the following global variables:

  * **S** :
    the global object **Space**. See the detailed description below.

  * **load_body**(*filename*) :
    load body from a text file with columns "*x* *y* *slip*". Loaded
    points are the vertices of the body. Value of *slip* can be either
    absent or `0` (no-slip, default) or `1` (slip). The order of points
    in the file matters, clockwise order corresponds to the internal
    flow and counter-clockwise order corresponds to the flow around the
    body.

  * **simulate**() :
    run the simulation until what?

  * **gen_cylinder**({*R*, [*N*|*dl*]}) :
  * **gen_semicyl**({*R*, [*N*|*dl*]}) :
  * **gen_ellipse**({*Rx*, *Ry*, [*N*|*dl*]}) :
  * **gen_plate**({*R1*, *R2*, *L*, [*N*|*dl*]}) :
  * **gen_parallelogram**({*L*, *H*, *d*, [*N*|*dl*]}) :
  * **gen_roundrect**({*L*, *H*, *R*, [*N*|*dl*]}) :
  * **gen_savonius**({*R*, *h*, [*N*|*dl*]}) :
  * **gen_chamber_gpj**({*R0*, *X0*, *X1*, *X2*, *X3*, *d*, *L*, *H*, *dln*, *dls*}) :
  * **gen_chamber_box**({*L*, *H*, *D*, *h*, *d*, *dln*, *dls*}) :

### Space

The general concept in Vvflow CFD Suite is the _Space_ `S`. It holds all
the data about the simulation. Here are listed all defined properties
with their types and physical meaning. Description of all types is given
below.

  * S.*caption* (string) :
    simulation caption is used in results filenames:
    stepdata_*caption*.h5, results_*caption*

  * S.*re* (number) :
    *re* = 1/*nyu*, where *nyu* is the kinematic viscosity of fluid

  * S.*inf_g* (number) :
    circulation over an infinite contour

  * S.*inf_vx*, S.*inf_vy* (TEval) :
    math expressions of undisturbed flow speed

  * S.*gravity* (TVec) :
    acceleration of gravity

  * S.*time* (TTime) :
    current simulation time

  * S.*dt* (TTime) :
    simulation timestep

  * S.*dt_save* (TTime) :
    period to save results

  * S.*dt_streak* (TTime) :
    period of streak domains shedding

  * S.*dt_profile* (TTime) :
    period to save profiles of pressure and friction

  * S.*finish* (number) :
    time to finish the simulation

  * S.*body_list* (TBodyList) :
    list of bodies

  * S.*vort_list* (TObjList) :
    list of vortex domains

  * S.*sink_list* (TObjList) :
    list of sources and sinks

  * S.*streak_source_list* (TObjList) :
    list of streak sources

  * S.*streak_domain_list* (TObjList) :
    list of streak particles

  * S`:save`(*FILE*), S`:load`(*FILE*) :
    save or load the Space in HDF5 format

  * S`:XStreamfunction`(*TVec*) :
    calculate streamfunction in point

_Examples_:

    S.re = 140
    S.inf_vx = "1+0.5*sin(2*pi*0.1*t)" -- gusty wind
    S.dt = "1/1000"
    S.dt_save = "50/1000"
    S.finish = 10
    cyl = gen_cylinder{R=0.5, N=600}
    cyl.label = "cyl"
    cyl.density = 8 -- steel
    cyl.spring_const.r.y = 10
    S.body_list:insert(cyl)
    S.caption = string.format("example_re%f", S.re)
    S:save(S.caption..".h5")

    -- calculate fluid flow rate between two points
    print( S:XStreamfunction{2, -1} - S:XStreamfunction{2, 1} )

## SEE ALSO
  vvxtract(1), vvplot(1)
