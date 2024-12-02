vvcompose(1) -- compose the CFD problem to be solved with vvflow program
====

## SYNOPSIS

`vvcompose` [<OPTIONS>] [--] [<FILE>] [<args>]

## DESCRIPTION

Moved to vvflow.1.md

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

### gen_cylinder

    Moved to vvflow.1.md

### gen_semicyl

    Moved to vvflow.1.md

### gen_ellipse

    Moved to vvflow.1.md

### gen_plate

    Moved to vvflow.1.md

### gen_parallelogram

    gen_parallelogram{L, H, d, [N|dl]}

Generate parallelogram with origin in bottom left corner.

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
    angle in degrees (from 0 to 180)

  * `N`, `dl` :
    specify either number of segments or average segment length

### gen_roundrect

    gen_roundrect{L, H, R, [N|dl]}

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

### gen_chamber_gpj

    gen_chamber_gpj{R0, X0, X1, X2, X3, d, L, H, dln, dls}

This is a chamber for the generator of pulsating jets (GPJ).
It consists of prechamber, straight channel, expanding channel, and a bounding box.

Prechamber is a half of a circle with radius `R0` and center at {`X0`, 0}.
Boundary condition in prechamber is always slip.

Straight channel is formed by two symmetric horizontal lines:
with slip condition from `X0` to `X1` and no-slip from `X1` to `X2`.

Expanding channel is defined by angle `d` in degrees. It spans from `X2` to `X3`.
The boundary condition in expanding channel is no-slip.

The bounding box is a rectangle with constraints X:{`X3`-`L`, `X3`}, Y:{-`H`/2, `H`/2}.
The bounding box implies slip boundary condition.

Regions with no-slip boundary condition have segment length `dln`, regions with slip - `dls`.

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

### gen_chamber_box

    gen_chamber_box{L, H, D, h, d, dln, dls}

This is a box chamber.
It consists of 4 thick walls with a hole on the bottom.

Inner rect has dimensions `L`x`H`.
with the centet of its bottom having coordinates {0, 0}.
Size of the hole is `D`.
Thickness of the left, right and top walls is `h`.
Thickness of the bottom wall is `d`.

Regions with no-slip boundary condition (inner bottom surface) have segment length `dln`,
regions with slip - `dls`.

    #    ,-------------------,  - H+h
    #    |   _____________   |  _ H
    #    |  |             |  |
    #    |  |             |  |
    #    |  |___   .   ___|  |  _ 0
    #    '------'     '------'  - 0-d
    #              ^  ^   ^  ^
    #              0 D/2 L/2 L/2+h

### gen_savonius

    gen_savonius{R, h, [N|dl]}

Savonius is formed by 2 thick semi-circles,
which boundary is 6 semi-circle arcs.
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

## SEE ALSO
  vvflow(1), vvxtract(1), vvplot(1)
