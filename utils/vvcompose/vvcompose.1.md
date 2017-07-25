vvcompose(1) -- compose the CFD problem to be solved with vvflow program
====

## SYNOPSIS

`vvcompose` [-v] [-h] [--] [<FILE>] [<args>]

## DESCRIPTION

vvcompose is a part of Vvflow CFD Suite. It serves as a preprocessor for defining the CFD problems.

vvcompose is an extended Lua interpreter. It supports everything that Lua supports. Plus it provides bindings for the VVD classes (structures) and functions.

The script is loaded from <FILE>, which consists of regular Lua commands. Optionally <FILE> may start with a shebang:

    #!/usr/bin/env vvcompose

then it can be launched as a usual shell script (if executable permissions are set, of course):

    $ ./FILE

When called without arguments,
vvcompose runs in the interactive mode
reading Lua commands from the standard input (stdin).

This manual will not discuss the Lua syntax in all details, although it will cover several topics which are essential for using vvcompose. The Lua basics can be found at various online resources. Here is a couple of them:

 * [Programming in Lua](https://www.lua.org/pil/1.html)
 * [Learn X in Y minutes](https://learnxinyminutes.com/docs/lua/)
 * [Lua math](http://lua-users.org/wiki/MathLibraryTutorial)
 * [Tutorials collection](http://lua-users.org/wiki/TutorialDirectory)

## OPTIONS

  * -v :
    print program version and exit

  * -h :
    show manpage and exit

## SYNTAX

### Space

The general concept in Vvflow CFD Suite is the _Space_ `S`.
It holds all the data about the simulation.
Here are listed all defined properties with their types and physical meaning.
Description of all types is given below.

  * S.*caption* (string) :
    simulation caption is used in results filenames:
    stepdata_*caption*.h5, results_*caption*/

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
    S.body_list.insert(cyl)
    S.caption = string.format("example_re%f", S.re)
    S:save(S.caption..".h5")

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

_Example_:

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

_Example_:

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

_Example_:

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

_Example_:

    S.inf_vx = "sin(2*pi*t/8)"
    S.inf_vy = "1"
    S.inf_vx:eval(2.0) -- 1.0

### TBody
  * body = `load_body`(*FILE*) :
    load body from text file with columns "*x* *y* *slip*".
    *slip* can be absent or `0` (no-slip, default) or `1` (slip).

  * body.*label* (string) :
    human-readable body name

  * body.*density* (number) :
    ratio of body and fluid densities

  * body.*holder_pos* (TVec3D) :
    the body holder coordinates

  * body.*holder_vx*, body.*holder_vy*, body.*holder_vo* (TEval):
    expressions of the holder movement speed

  * body.*speed* (TVec3D) :
    the speed obtained by solving the System of Linear Equations.
    The rotation *axis* `=` *holder_pos* `+` *delta_pos*, see body`:get_axis`().

  * body.*delta_pos* (TVec3D) :
    current deformations of elastic connections between the holder and the body

  * body.*spring_const* (TVec3D) :
    rate of elastic connections.
    Negative and `inf` values correspond to the rigid connection.

  * body.*spring_damping* (TVec3D) :
    viscous damping coefficient

    Force acting on a body can be expressed as
    `F = -`*spring_const*`*`*delta_pos* `+` *spring_damping*`*`*speed*.
    Due to the mistake in code the damping should be negative (the author will fix it later).

  * body.*collision_min*, body.*collision_max* (TVec3D) :
    boundaries for body movement (concerning the *axis*)

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

  * body`:get_com`() :
    return Lua table {*com.x*, *com.y*}, where *com* is the body center of mass

  * body`:get_arclen`() :
    return the body surface length

  * body`:get_area`() :
    return the body area

  * body`:get_moi_c`() :
    return the moment of inertia about the center of mass.
    Moment of inertia is calculated for density = 1,
    do not forget to multiply it.

  * body`:totable`() :
    return Lua table with body corners (TVec)

_Example_:

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

_Example_:

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

_Example_:

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

## BODY GENERATORS

### gen_cylinder{R, [N|dl]}

Generate a cylinder with center at {0, 0}

    #        .-'""'-.
    #      .'        '.
    #     /            \
    #    ;              ;
    #    ;       \      ;
    #     \     R \    /
    #      '.      \ .'
    #        '-....-'

  * `R` :
    radius of the cylinder

  * `N`, `dl` :
    specify either number of segments or average segment length

_Example_:
cyl = gen_cylinder{R=0.5, N=500}

### gen_semicyl{R, [N|dl]}

The bottom half of a cylinder with center at {0, 0}

    #      ____________________
    #     |          /         |
    #     ;         /          ;
    #      ;       / R        ;
    #       \     /          /
    #        '.  /         .`
    #          `-.______.-'
    #

### gen_ellipse{Rx, Ry, [N|dl]}

Generate an ellipse with center at {0, 0}

    #        .--"""""--.
    #      .'     | Ry  '.
    #     /       |       \
    #    ;        +---Rx---;
    #     \               /
    #      '.           .'
    #        '--.....--'

  * `Rx`, `Ry` :
    Ellipse semi-axis

  * `N`, `dl` :
    specify either number of segments or average segment length

### gen_plate{R1, R2, L, [N|dl]}

Shape of a plate is formed by two circles and two tangents.
Circles radius are `R1` and `R2`, their centers are {0, 0} and {0, `L`}.

    #        .-'""""""""""""""""""""""""'-.
    #      .'                             /'.
    #     /                           R2 /   \
    #    ;       __________ L _________ /     ;
    #    ;      /                             ;
    #     \    / R1                          /
    #      '. /                            .'
    #        '-..........................-'


  * `R1`, `R2` :
    radius of the left and right edge respectively

  * `L` :
    distance between circles centers

  * `N`, `dl` :
    specify either number of segments or average segment length

### gen_parallelogram{L, H, d, [N|dl]}

Generate parallelogram with origin is in bottom left corner.

    #          _______________
    #         /|            d /
    #        / |             /
    #       /  |H           /
    #      /   |           /
    #     /    |          /
    #    /_____'_________/
    #            L

  * `L` :
    basement length

  * `H` :
    height

  * `d` :
    angle in degrees

  * `N`, `dl` :
    specify either number of segments or average segment length

### gen_roundrect{L, H, R, [N|dl]}

Generate roundrect with origin in center.

    #       .-;""""""""""""""""""""""""""--.
    #     /`  | R                           `\
    #    ;    |                               ;
    #    |----+                               |
    #    |                                    |
    #    ;                                    ;
    #     \                                  /
    #      `'--..........................--'`

  * `L` :
    bounding rect length

  * `H` :
    bounding rect height

  * `R` :
    corner radius (shoud be less than `0.5*min(L,H)` )

  * `N`, `dl` :
    specify either number of segments or average segment length

### gen_gis{R0, X0, X1, X2, X3, d, L, H}

This is a chamber for pulsating jets generator.
It consists of prechamber, straight channel, expanding channel, and a bounding box.

Prechamber is a half of a circle with radius `R0` and center at {`X0`, 0}.
Boundary condition in prechamber is always slip.

Straight channel is formed by two symmetric horizontal lines:
with slip from `X0` to `X1` and no-slip from `X1` to `X2`.

Expanding channel is defined by angle `d` in degrees. It spans from `X2` to `X3`.

The bounding box is a rectangle with constraints X:{`X3`-`L`, `X3`}, Y:{-`H`/2, `H`/2}.

    #    ,----------------,  - H
    #    |              .-'  - y3
    #    |     .------""     - r0
    #    |    /
    #    |    \
    #    |     '------..
    #    |              '-.
    #    '----------------'

    #                     ..--"
    #      .-"""""""|""""'
    #     /    
    #    ;    +
    #     \    
    #      '-.......|....,
    #                     ""--,
    #         ^x0   ^x1  ^x2  ^x3

### gen_savonius{R, h, [N|dl]}

Savonius is formed by 6 semi-circles.
This is how it looks like:

    #        .-'""'-.
    #      .'        '.
    #     /   .-""-.   \          __
    #    ;   /      \   ;        /  \ 
    #    \__/        ;   \      /   ;
    #                 \   '-..-'   /
    #                  '.        .'
    #                    '-....-'

  * `R` :
    radius of the midline

  * `h` :
    thickness of savonius

  * `N`, `dl` :
    specify either number of segments or average segment length
             
            
          
          
          