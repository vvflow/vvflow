vvflow(1) -- solve the CFD problem
====

## SYNOPSIS

**vvflow** [ _options_ ] _file_

## DESCRIPTION

**vvflow** is the main tool of the Vvflow CFD Suite.
It performs the simulation and solves the CFD problem.

Problem statement is loaded from _file_ - a Lua script specifying the
CFD problem. For the backward compatibility the _file_ can also be an
hdf5 file copied from another simulation results.

## OPTIONS

  * **-v, --version** :
    print program version and exit

  * **-h, --help** :
    print help and exit

  * **--progress** :
    print progress during the simulation

  * **--profile** :
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
    generate a cylinder with the center at point (0, 0)
    with *R* - radius of the cylinder,
    *N* - number of segments,
    *dl* - average segment length.
    Either *N* or *dl* must be specified but not both.

```
    #        .-'""'-.
    #      .'        '.
    #     /            \
    #    ;              ;
    #    ;       \      ;
    #     \     R \    /
    #      '.      \ .'
    #        '-....-'

    Example:

    local b1 = gen_cylinder({R=0.5, N=500})
    local b2 = gen_cylinder({R=0.5, dl=0.01})
```

  * **gen_semicyl**({*R*, [*N*|*dl*]}) :
    generate bottom half of a cylinder with the center at point (0, 0)
    with *R* - radius of the cylinder,
    *N* - number of segments,
    *dl* - average segment length.
    Either *N* or *dl* must be specified but not both.

```
    #      ____________________
    #     |          /         |
    #     ;         /          ;
    #      ;       / R        ;
    #       \     /          /
    #        '.  /         .`
    #          `-.______.-'
    #

    Example:

    local b1 = gen_semicyl({R=0.5, N=500})
    local b2 = gen_semicyl({R=0.5, dl=0.01})
```

  * **gen_ellipse**({*Rx*, *Ry*, [*N*|*dl*]}) :
    generate an ellipse with the center at point (0, 0)
    with *Rx*, *Ry* - semi-axis,
    *N* - number of segments,
    *dl* - average segment length.
    Either *N* or *dl* must be specified but not both.

```
    #        .--"""""--.
    #      .'     | Ry  '.
    #     /       |       \
    #    ;        +---Rx---;
    #     \               /
    #      '.           .'
    #        '--.....--'

    Example:

    local b = gen_ellipse{Rx=4, Ry=0.5, N=600}
```

  * **gen_plate**({*R1*, *R2*, *L*, [*N*|*dl*], [*start*], [*stop*], [*gap*]}) :
    A plate is formed by two circles and two tangents.
    Circles radius are *R1* and *R2*, their centers are (0, 0) and (0, *L*).

    Parameters *start* and *stop* allow generating multi-segment fish-like body.

```
    #        .-'""""""""""""""""""""""""'-.
    #      .'                             /'.
    #     /                           R2 /   \
    #    ;       __________ L _________ /     ;
    #    ;      /                             ;
    #     \    / R1                          /
    #      '. /                            .'
    #        '-..........................-'
```

  * **gen_parallelogram**({*L*, *H*, *d*, [*N*|*dl*]}) :
    generate a parallelogram with its bottom left corner at point (0, 0)
    with *L* - basement length,
    *H* - height,
    *d* - angle in degrees (from 0 to 180),
    *N* - number of segments,
    *dl* - average segment length.
    Either *N* or *dl* must be specified but not both.

```
    #          _______________
    #         /|            d /
    #        / |             /
    #       /  |H           /
    #      /   |           /
    #     /    |          /
    #    /_____'_________/
    #            L
```

  * **gen_roundrect**({*L*, *H*, *R*, [*N*|*dl*]}) :
    generate a roundrect with its center at point (0, 0)
    with *L*, *H* - bounding rect size,
    *R* - corner radius (shoud be less than `0.5*min(L,H)`),
    *N* - number of segments,
    *dl* - average segment length.
    Either *N* or *dl* must be specified but not both.

```
    #       .-;""""""""""""""""""""""""""--.
    #     /`  | R                           `\
    #    ;    |                               ;
    #    |----+                               |
    #    |                                    |
    #    ;                                    ;
    #     \                                  /
    #      `'--..........................--'`
```

  * **gen_savonius**({*R*, *h*, [*N*|*dl*]}) :
    generate a savonius formed by 2 thick semi-circles
    with *R* - the radius of the midline,
    *h* - thickness of savonius,
    *N* - number of segments,
    *dl* - average segment length.
    Either *N* or *dl* must be specified but not both.

```
    #        .-'""'-.
    #      .'        '.
    #     /   .-""-.   \          __
    #    ;   /      \   ;        /  \
    #    \__/        ;   \      /   ;
    #                 \   '-..-'   /
    #                  '.        .'
    #                    '-....-'
```

  * **gen_chamber_gpj**({*R0*, *X0*, *X1*, *X2*, *X3*, *d*, *L*, *H*, *dln*, *dls*}) :
    This is a chamber for the generator of pulsating jets (GPJ).
    It consists of prechamber, straight channel, expanding channel, and a bounding box.

    Prechamber is a half of a circle with radius *R0* and the center at point (*X0*, 0).
    Boundary condition in prechamber is always slip.

    Straight channel is formed by two symmetric horizontal lines:
    with slip condition from *X0* to *X1* and no-slip from *X1* to *X2*.

    Expanding channel is defined by angle *d* in degrees. It spans from *X2* to *X3*.
    The boundary condition in expanding channel is no-slip.

    The bounding box is a rectangle with constraints X:{*X3*-*L*, *X3*}, Y:{-*H*/2, *H*/2}.
    The bounding box implies slip boundary condition.

    Regions with no-slip boundary condition have segment length *dln*, regions with slip - *dls*.

```
    #    ,----------------,  - H
    #    |              .-'
    #    |     .------""     - r0
    #    |    /
    #    |    \
    #    |     '------..
    #    |              '-.
    #    '----------------'

    #          slip | noslip ..--"
    #      .-"""""""|"""""""'
    #     /
    #    ;    +
    #     \
    #      '-.......|.......,
    #                        ""--,
    #         ^x0   ^x1     ^x2  ^x3
```

  * **gen_chamber_box**({*L*, *H*, *D*, *h*, *d*, *dln*, *dls*}) :
    This is a box chamber.
    It consists of 4 thick walls with a hole on the bottom.

    Inner rect has dimensions *L*x*H*
    with the centet of its bottom side at point (0, 0).
    Size of the hole is *D*.
    Thickness of the left, right and top walls is *h*.
    Thickness of the bottom wall is *d*.

    Regions with no-slip boundary condition (inner bottom surface) have segment length *dln*,
    regions with slip - *dls*.

```
    #    ,-------------------,  - H+h
    #    |   _____________   |  _ H
    #    |  |             |  |
    #    |  |             |  |
    #    |  |___   .   ___|  |  _ 0
    #    '------'     '------'  - 0-d
    #              ^  ^   ^  ^
    #              0 D/2 L/2 L/2+h
```

### Space

The general concept in Vvflow CFD Suite is the _Space_ `S`. It holds all
the data about the simulation. Here are listed all defined properties
with their types and physical meaning. Description of all types is given
below.

  * **S.caption** = *caption* (string) :
    simulation caption is used in results filenames:
    stepdata\_*caption*.h5, results\_*caption*/

  * **S.re** = *re* (number) :
    *re* = 1/*nyu*, where *nyu* is the kinematic viscosity of fluid

  * **S.inf_g** = ... (number) :
    circulation over an infinite contour

  * **S.inf_vx, S.inf_vy** = ... (TEval) :
    math expressions of undisturbed flow speed

  * **S.gravity** = ... (TVec) :
    acceleration of gravity

  * **S.time** = ... (TTime) :
    current simulation time

  * **S.dt** = ... (TTime) :
    simulation timestep

  * **S.dt_save** = ... (TTime) :
    period to save results

  * **S.dt_streak** = ... (TTime) :
    period of streak domains shedding

  * **S.dt_profile** = ... (TTime) :
    period to save profiles of pressure and friction

  * **S.finish** = ... (number) :
    time to finish the simulation

  * **S.body_list** (TBodyList) :
    list of bodies

  * **S.vort_list** (TObjList) :
    list of vortex domains

  * **S.sink_list** (TObjList) :
    list of sources and sinks

  * **S.streak_source_list** (TObjList) :
    list of streak sources

  * **S.streak_domain_list** (TObjList) :
    list of streak particles

  * **S:save**(_filename_), **S:load**(_filename_) :
    save or load the Space in HDF5 format

  * **S:XStreamfunction**({_x_, _y_}) :
    calculate streamfunction in point

Example:

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

### TVec

  * vec.*x* (number) :

  * vec.*y* (number) :

  * vec = {*x*, *y*} :

  * vec`:abs2`() :
    return *x*\^2 + *y*\^2

  * vec`:abs`() :
    return sqrt( *x*\^2 + *y*\^2 )

  * vec`:tostring`(), `tostring`(vec) :
    return string "TVec(*x*,*y*)"

  * vec`:totable`() :
    return Lua table {*x*, *y*}

Example:

    S.gravity = {0, -9.8}
    local vec = S.gravity
    print(vec.x)      -- '0'
    print(vec.y)      -- '-9.8'
    print(vec:abs2()) -- '96.04', which is 9.8^2
    print(vec:abs())  -- '9.8'
    vec.x = 0 -- vectors are referenced by pointers
    vec.y = 0 -- now gravity.y is 0

### TVec3D

Extended version of TVec with the third rotational component (angle)

  * vec3d.*r* (TVec) :

  * vec3d.*o* (number, radians) :

  * vec3d.*d* (number, degrees) :

  * vec3d = {*x*, *y*, *o*} :
    initialization with *r* = {*x*, *y*} and angle *o* in radians

  * vec3d`:tostring`(), `tostring`(vec3d) :
    return string "TVec3D(*x*,*y*,*o*)"

  * vec3d`:totable`() :
    return Lua table {*x*, *y*, *o*}

Example:

    body.holder_pos = {0, 0, 0}
    body.delta_pos.r = {1, 0} -- x, y components
    body.delta_pos.o = math.pi -- in radians
    body.delta_pos.d = 180 -- in degrees

### TTime

TTime class represets time as a common fraction.
There are three forms of assignment:

  * time =  {*num*, *den*} :

  * time = "*num*/*den*" :
    initialize with fraction *num*/*den*

  * time = *t* :
    initialize with decimal *t*

Example:

    S.time = 0.33 -- parsed as decimal 33/100
    S.dt_save = {1, 500} -- 0.002
    S.dt = "1/77" -- do not forget about quotes here
    -- fraction without quotes is calculated by Lua,
    -- thus obtained number will be considered decimal,
    -- which may lead to the accuracy loss

### TEval

TEval class represents a math function of time *t*, expressed as string.
TEval is implemented using [GNU libmatheval](https://www.gnu.org/software/libmatheval/manual/)

  * e = "*expr*" :
    initialize with math expression *expr*

  * e`:eval`(*t*) :
    evaluate the math expression

  * e`:tostring`(), `tostring`(e) :
    return string "*expr*"

Example:

    S.inf_vx = "sin(2*pi*t/8)"
    S.inf_vy = "1"
    S.inf_vx:eval(2.0) -- 1.0

### TBody
  * body.*label* (string) :
    human-readable body name

  * body.*density* (number) :
    ratio of body and fluid densities

  * body.*holder_pos* (TVec3D) :
    the body holder coordinates

  * body.*holder_vx*, body.*holder_vy*, body.*holder_vo* (TEval):
    expressions of the holder movement speed

  * body.*holder_mo* (TEval):
    expression of the external holder torque

  * body.*speed* (TVec3D) :
    the speed obtained by solving the System of Linear Equations.
    The rotation *axis* `=` *holder_pos* `+` *delta_pos*, see body`:get_axis`().

  * body.*delta_pos* (TVec3D) :
    current deformations of elastic connections between the holder and the body

  * body.*spring_const* (TVec3D) :
    rate of elastic connections.
    Value `inf` corresponds to the rigid connection.

  * body.*spring_damping* (TVec3D) :
    viscous damping coefficient

    Force acting on a body can be expressed as
    `F = -`*spring_const*`*`*delta_pos* `-` *spring_damping*`*`*speed*.

  * body.*collision_min*, body.*collision_max* (TVec3D) :
    boundaries of body movement (concerning the *axis*).
    Value `nan` correspond to the absence of boubdaries.

  * body.*bounce* (number) :
    coefficient of restitution (considering collisions)

  * body.*slip* (boolean) :
    whether to use slip or no-slip condition
    for all segments of the body.
    Valid values are `false` (no-slip, default) and `true` (slip).
    Getting the value may return `nil`, which indicates
    that the boundary condition is not uniform along the surface.

  * body.*special_segment* (integer) :
    the number of segment where slip/no-slip boundary condition
    is replaced with equation of the flow steadines at infinity

  * body.*root* (TBody) :
    the root of the body. When `nil`, the holder is ultimate.

  * `#`body :
    return number of body segments

  * body`:move_r`(TVec) :
    displace the body together with *holder_pos.r*, keeping *delta_pos* unchanged

  * body`:move_o`(*o*), body`:move_d`(*d*):
    rotate the body around the *axis* by *o* radiand or *d* degrees.

    Here *axis* `=` *holder_pos.r* `+` *delta_pos.r*.
    After rotaion *holder_pos.r* and *delta_pos* remains unchanged,
    and *holder_pos.o* increases by *o*.

  * body`:get_axis`() :
    return Lua table {*axis.x*, *axis.y*}, where *axis* `=` *holder_pos.r* `+` *delta_pos.r*

  * body`:get_cofm`() :
    return Lua table {*cofm.x*, *cofm.y*}, where *cofm* is the body center of mass

  * body`:get_slen`() :
    return the body surface length

  * body`:get_area`() :
    return the body area

  * body`:get_moi_cofm`() :
    return the moment of inertia about the center of mass.
    Moment of inertia is calculated for density = 1,
    do not forget to multiply it.

  * body`:get_moi_axis`() :
    return the moment of inertia about the *axis*.
    *moi_axis* = *moi_cofm* + (*axis*-*cofm*).abs2() * *area*.

  * body`:totable`() :
    return Lua table with body corners (TVec)

Example:

    local cyl = load_body("cyl.txt")
    cyl.label = "cylinder"
    cyl:move_r({1, 0})
    cyl.holder_pos.r = {0, 0}
    cyl.delta_pos.r = {1, 0}
    cyl.spring_const.r.x = 1
    cyl.spring_const.r.y = inf

### TBodyList

  * S.body_list`:insert`(*body*) :
    add *body* to the list

  * S.body_list`:erase`(*body*) :
    remove *body* from the list

  * S.body_list`:clear`() :
    remove all bodies

  * `#`S.body_list :
    return number of bodies in list

  * S.body_list[*i*] :
    return body at position *i*.
    Counting is valid from `1` to `#list`, other indices return `nil`.

  * `ipairs`(S.body_list) :
    iterate over bodies

Example:

    -- change body density
    S:load("example_re140.h5")
    S.body_list[1].density = 4
    S:save("example_re140.h5")

### TObj

TObj is a general class for vortex domains, sources and sinks, streak domains and streak sources.

  * obj.*r* (TVec) :
    object position

  * obj.*v* (TVec) :
    object speed, for internal use only

  * obj.*g* (number) :
    In *S.vort_list* it denotes circulation of vortex domain;

    In *S.sink_list* it denotes intensity of sources (positive) and sinks (negative);

    In *S.streak_source_list* and *S.streak_domain_list* the value *g* is ignored (and copied as is);

  * obj = {*x*, *y*, *g*} :
    initialization with *r* = {*x*, *y*}, *v* = {0, 0}

  * obj:`tostring`(), `tostring`(obj) :
    return string "TObj(*x*,*y*,*g*)"

### TObjList

  * list`:append`(*obj*, ...) :
    append one or more objects to the list.
    *obj* can be either TObj instance or Lua table {*x*, *y*, *g*}

  * list`:clear`() :
    remove all objects from list

  * list`:load`(*FILE*) :
    load body from text file with columns "*x* *y* *g*".
    Loading does not remove any objects appended earlier.

  * list`:load_bin`(*FILE*) :
    load body from binary file, reading TObj values *x*, *y*, *g* from
    three 8-byte blocks in double-precision floating-point format

  * list`:totable`() :
    return Lua table with objects (TObj)

  * `#`list :
    return number of objects in list

  * list[*i*] :
    return object at position *i*.
    Counting is valid from `1` to `#list`, other indices return `nil`.

  * `ipairs`(list) :
    iterate over objects in list

Example:

    S.vort_list:clear()
    S.vort_list:load("vlist.txt")
    local gsum = 0
    for i, obj in ipairs(S.vort_list) do
        gsum = gsum + obj.g
    end
    print("Total circulation =", gsum)

    S.sink_list:clear()
    S.sink_list:append({0, 0, 10}) -- add source
    print(#S.sink_list)   -- '1'
    print(S.sink_list[1]) -- 'TObj(0,0,10)'
    print(S.sink_list[2]) -- 'nil'

## SEE ALSO
  vvxtract(1), vvplot(1)
