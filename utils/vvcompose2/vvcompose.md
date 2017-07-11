vvcompose(1) -- compose the CFD problem to be solved with vvflow program
====

## SYNOPSIS

`vvcompose` <FILE>

## DESCRIPTION

vvcompose is a part of Vvflow CFD Suite. It serves as a preprocessor for defining the CFD problems.

vvcompose is an extended Lua interpreter. It supports everything that Lua supports. Plus it provides bindings for the VVD classes (structures) and functions.

The script is loaded from <FILE>, which consists of regular Lua commands. Optionally <FILE> may start with a shebang:

    #!/usr/bin/env vvcompose

then it can be launched as a usual shell script (if executable permissions are set, of course):

    $ ./FILE

This manual will not discuss the Lua syntax in all details, although it will cover several topics which are essential for using vvcompose. The Lua basics can be found at various online resources. Here is a couple of them:

 * [Programming in Lua](https://www.lua.org/pil/1.html)
 * [Learn X in Y minutes](https://learnxinyminutes.com/docs/lua/)
 * [Lua math](http://lua-users.org/wiki/MathLibraryTutorial)
 * [Tutorials collection](http://lua-users.org/wiki/TutorialDirectory)

## SYNTAX

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
    local my_vector = S.gravity
    print(my_vector.x)      -- '0'
    print(my_vector.y)      -- '-9.8'
    print(my_vector:abs2()) -- '96.04', which is 9.8^2
    print(my_vector:abs())  -- '9.8'
    my_vector.x = 0 -- vectors are referenced by pointers
    my_vector.y = 0 -- now gravity.y is 0

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

  * body.*special_segment* (integer) :
    the number of segment where slip/no-slip boundary condition
    is replaced with equation of the flow steadines at infinity  

  * `#`body :
    return number of body segments

  * body`:move_r`({*x*, *y*}) :
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


### ObjList

### BodyList

    

Several built-in functions may be used to generate common shapes, which are described in section [BODY GENERATORS]. Alternatively, _TBody_ can be loaded from file:



### Space


## EXAMPLES

### Space

The most basic thing is the _Space_ `S`. Just as a usual class in programming, Space has member variables, accessed with dot (`.`). The complete list of members will be given later. Here is a short example: 

    S.caption = "example"
    S.re = 1/100 -- one can already use math here
    -- the string after tho dashes is a comment, which is ignored

_Space_ also has member functions:

    S:save("example.h5")
    S:load("example.h5")
    -- the colon (`:`) is a part of Lua syntax
    -- in fact it is just a short form of
    -- S.save(S, "example.h5")

### Body


### ObjList

_ObjList_ is a general class for It is used for _vortex domains_, _sources_ and _sinks_, _streak sources_ and _streak domains_.

    S.vort_list:load("list.txt")
    -- in three columns: x y g

    S.sink_list:append({-1, 0, +1})
    S.sink_list:append({+1, 0, -1})
    -- positive is for source
    -- negative is for sink

    S.streak_source_list:clear()
    S.streak_domain_list:clear()

## EXTENDED TOPICS

## BODY GENERATORS
