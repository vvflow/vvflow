function printf(fmt, ...)
    print(string.format(fmt, ...))
end

FAIL = 0
pi = math.pi

function check_err(fn, err_need)
    ret, err_have = pcall(fn)
    if err_need ~= nil then
        info = debug.getinfo(2)
        err_need = string.format("%s:%s: %s", info.short_src, info.currentline, err_need)
    end

    if err_have ~= err_need then
        printf("Unexpected error")
        printf("Have: %s", err_have)
        printf("Need: %s", err_need)
        FAIL = 1
    end
end

function check_val(fn, val_need)
    ret, val_have = pcall(fn)
    info = debug.getinfo(2)
    if val_have ~= val_need then
        printf("%s:%s: Unexpected value", info.short_src, info.currentline)
        printf("Have: %s", val_have)
        printf("Need: %s", val_need)
        FAIL = 1
    end
end

function check_dev(fn, val_need, precision)
    local ok, val_have = pcall(fn)
    info = debug.getinfo(2)
    if ok then
        ok = math.abs(val_have-val_need) < precision
    end
    if not ok then
        printf("%s:%s: Unexpected value", info.short_src, info.currentline)
        printf("Have: %s", val_have)
        printf("Need: %s", val_need)
        FAIL = 1
    end
end

-- inf, nan
check_val(function() return  inf end,  1/0)
check_val(function() return -inf end, -1/0)
check_val(function() return tostring( inf) end,  "inf")
check_val(function() return tostring(-inf) end, "-inf")
check_val(function() return tostring( nan) end,  "nan")

-- Space
check_err(function() S.foo = nil end, "S can not assign 'foo'")
check_err(function() return S.load() end, "bad argument #1 to 'load' (S expected, got no value)")
check_err(function() return S.save() end, "bad argument #1 to 'save' (S expected, got no value)")
check_err(function() return S:load() end, "bad argument #1 to 'load' (string expected, got no value)")
check_err(function() return S:save() end, "bad argument #1 to 'save' (string expected, got no value)")
check_val(function() return S.foo        end, nil)
check_val(function() return S.caption    end, "")
check_val(function() return S.time       end, 0)
check_val(function() return S.dt         end, 1)
check_val(function() return S.dt_save    end, 0)
check_val(function() return S.dt_streak  end, 0)
check_val(function() return S.dt_profile end, 0)
check_val(function() return S.re         end, inf)
check_val(function() return string.format("%.0e", S.finish) end, "2e+308")
check_val(function() return S.inf_vx:tostring() end, "")
check_val(function() return S.inf_vy:tostring() end, "")
check_val(function() return S.inf_g             end, 0)
check_val(function() return S.gravity:tostring() end, "TVec(0,0)")
S:load("/dev/null")
S:save("/dev/null")

-- std::string
S.caption = "good_caption"
check_val(function() return S.caption end, "good_caption")
check_err(function() S.caption = 100 end, "bad value for S.caption (string expected, got number)")
check_err(function() S.caption = {1} end, "bad value for S.caption (string expected, got table)")
check_err(function() S.caption = S.gravity end, "bad value for S.caption (string expected, got userdata)")
check_err(function() S.caption = nil end, "bad value for S.caption (string expected, got nil)")
check_val(function() return S.caption end, "good_caption")

-- double
S.re = inf
check_val(function() return S.re end, inf)
S.re = "10"
check_val(function() return S.re end, 10)
S.re = "+11e0"
check_val(function() return S.re end, 11)
S.re = 3*4
check_val(function() return S.re end, 12)
check_err(function() S.re = nil  end, "bad value for S.re (number expected, got nil)")
check_err(function() S.re = true end, "bad value for S.re (number expected, got boolean)")
check_val(function() return S.re end, 12)

-- TVec
S.gravity = {inf, nan}
S.gravity = {"0", "0"}
S.gravity = {15.1, 15.2}
S.gravity = S.gravity
check_err(function() S.gravity = {1} end, "bad value for S.gravity (TVec needs table with two numbers)")
check_err(function() S.gravity = 100 end, "bad value for S.gravity (TVec needs table with two numbers)")
check_err(function() S.gravity = nil end, "bad value for S.gravity (TVec needs table with two numbers)")
check_err(function() S.gravity = "1" end, "bad value for S.gravity (TVec needs table with two numbers)")
check_err(function() S.gravity = {"a", "b"} end, "bad value for S.gravity (TVec needs table with two numbers)")
check_val(function() return tostring(S.gravity)  end, "TVec(15.1,15.2)")
check_val(function() return S.gravity:tostring() end, "TVec(15.1,15.2)")
check_val(function() return #S.gravity:totable()    end, 2)
check_val(function() return S.gravity:totable()[1]  end, 15.1)
check_val(function() return S.gravity:totable()[2]  end, 15.2)
check_val(function() return S.gravity.x end, 15.1)
check_val(function() return S.gravity.y end, 15.2)
check_val(function() return S.gravity.z end, nil)
check_val(function() return S.gravity:abs2() end, 15.1^2 + 15.2^2)
check_val(function() return S.gravity:abs() end, math.sqrt(15.1^2 + 15.2^2))

-- TTime
S.dt = 0.1
check_val(function() return S.dt end, 0.1)
S.dt = {1, 10}
check_val(function() return S.dt end, 0.1)
S.dt = "1/10"
check_val(function() return S.dt end, 0.1)
check_err(function() S.dt = "inf" end, "bad value for S.dt (TTime can not parse 'inf')")
check_err(function() S.dt = "nan" end, "bad value for S.dt (TTime can not parse 'nan')")

check_err(function() S.dt = "1e500" end, "bad value for S.dt (TTime can not parse '1e500')")
check_err(function() S.dt = {}      end, "bad value for S.dt (TTime needs table with two elements, got 0)")
check_err(function() S.dt = {1,2,3} end, "bad value for S.dt (TTime needs table with two elements, got 3)")
check_err(function() S.dt = {1,"a"} end, "bad value for S.dt (TTime needs two integers, got number and string)")
check_err(function() S.dt = {-1,-3} end, "bad value for S.dt (TTime timescale must be positive, got -3)")
check_err(function() S.dt = nil     end, "bad value for S.dt (number or table expected, got nil)")

-- ShellScript
check_err(function() S.inf_vx = "z" end, "bad value for S.inf_vx (invalid expression 'z')")
check_err(function() S.inf_vx = {1} end, "bad value for S.inf_vx (string or ShellScript expected, got table)")
check_err(function() S.inf_vx = 1.0 end, "bad value for S.inf_vx (string or ShellScript expected, got number)")
check_err(function() S.inf_vx = nil end, "bad value for S.inf_vx (string or ShellScript expected, got nil)")
check_err(function() S.inf_vx.s = 0 end,    "ShellScript has no parameters")
S.inf_vx = "sin(2*pi*t/16)"
S.inf_vy = "sin(2*pi*t/32)"
check_val(function() return tostring(S.inf_vx) end, "sin(2*pi*t/16)")
check_val(function() return tostring(S.inf_vy) end, "sin(2*pi*t/32)")
check_val(function() return S.inf_vx:tostring() end, "sin(2*pi*t/16)")
check_val(function() return S.inf_vy:tostring() end, "sin(2*pi*t/32)")
check_val(function() return S.inf_vx:eval(4) end, 1)
check_val(function() return S.inf_vy:eval(8) end, 1)
check_val(function() return S.inf_vx.member end, nil)

-- TBody
local body = load_body("/dev/null")
check_val(function() return #body                         end, 0)
check_val(function() return body.foo                      end, nil)
check_err(function() body.foo = nil                       end, "TBody can not assign 'foo'")
check_val(function() return body.label                    end, "")
check_val(function() return tostring(body.holder_pos)     end, "TVec3D(0,0,0)" )
check_val(function() return tostring(body.delta_pos)      end, "TVec3D(0,0,0)" )
check_val(function() return tostring(body.speed)          end, "TVec3D(0,0,0)" )
check_val(function() return tostring(body.collision_min)  end, "TVec3D(nan,nan,nan)")
check_val(function() return tostring(body.collision_max)  end, "TVec3D(nan,nan,nan)")
check_val(function() return tostring(body.spring_const)   end, "TVec3D(inf,inf,inf)")
check_val(function() return tostring(body.spring_damping) end, "TVec3D(0,0,0)")
check_val(function() return tostring(body.holder_vx)      end, "")
check_err(function() body.holder_vx.foo = nil             end, "ShellScript has no parameters")
check_val(function() return tostring(body.holder_vy)      end, "")
check_val(function() return tostring(body.holder_vo)      end, "")
check_val(function() return body.density                  end, 1)
check_val(function() return body.bounce                   end, 0)
check_val(function() return body.special_segment          end, 0)

-- TVec3D
body.holder_pos = {inf, nan, -inf}
body.holder_pos = {"0", "0", "1"}
body.holder_pos = {16.1, 16.2, 16.3}
body.holder_pos = body.holder_pos
check_err(function() body.holder_pos = {1} end, "bad value for TBody.holder_pos (TVec3D needs table with three numbers)")
check_err(function() body.holder_pos = 100 end, "bad value for TBody.holder_pos (TVec3D needs table with three numbers)")
check_err(function() body.holder_pos = nil end, "bad value for TBody.holder_pos (TVec3D needs table with three numbers)")
check_err(function() body.holder_pos = "1" end, "bad value for TBody.holder_pos (TVec3D needs table with three numbers)")
check_err(function() body.holder_pos = {"a",0,0} end, "bad value for TBody.holder_pos (TVec3D needs table with three numbers)")
check_val(function() return tostring(body.holder_pos)  end, "TVec3D(16.1,16.2,16.3)")
check_val(function() return body.holder_pos:tostring() end, "TVec3D(16.1,16.2,16.3)")
check_val(function() return #body.holder_pos:totable()    end, 3)
check_val(function() return body.holder_pos:totable()[1]  end, 16.1)
check_val(function() return body.holder_pos:totable()[2]  end, 16.2)
check_val(function() return body.holder_pos:totable()[3]  end, 16.3)
body.holder_pos.r = {17.1, 17.2}
body.holder_pos.d = 17.3*180/math.pi
check_val(function() return body.holder_pos.r.x end, 17.1)
check_val(function() return body.holder_pos.r.y end, 17.2)
check_val(function() return body.holder_pos.o   end, 17.3)
check_val(function() return body.holder_pos.d   end, 17.3*180/math.pi)
check_val(function() return body.holder_pos.z   end, nil)
body = nil
collectgarbage()

-- ObjList
check_val(function() return #S.vort_list          end, 0)
check_val(function() return #S.sink_list          end, 0)
check_val(function() return #S.streak_source_list end, 0)
check_val(function() return #S.streak_domain_list end, 0)
check_val(function() return S.vort_list.foo       end, nil)
check_val(function() return S.vort_list[0]        end, nil)
check_val(function() return S.vort_list[1]        end, nil)
check_err(function() S.vort_list = S.sink_list    end, "S can not assign 'vort_list'")
check_err(function() S.vort_list.foo = nil end, "TObjList can not assign anything")
check_err(function() S.vort_list[0]  = nil end, "TObjList can not assign anything")
check_err(function() S.vort_list[1]  = nil end, "TObjList can not assign anything")
check_err(function() S.vort_list:append({1.1, 1.2, 1.3}) end, nil)
check_err(function() S.vort_list:append({2.1, 2.2, 2.3}) end, nil)
check_err(function() S.vort_list:append({3.1, 3.2, 3.3}) end, nil)
check_err(function() S.vort_list:append(S.vort_list[3] ) end, nil)
check_err(function() S.vort_list:append({1}) end, "bad argument #1 to 'append' (expected TObj)")
check_err(function() S.vort_list:append(1.0) end, "bad argument #1 to 'append' (expected TObj)")
check_err(function() S.vort_list:append("x") end, "bad argument #1 to 'append' (expected TObj)")
check_val(function() return #S.vort_list end, 4)
obj = S.vort_list[4]
check_val(function() return tostring(obj)    end, "TObj(3.1,3.2,3.3)")
check_val(function() return obj:tostring()   end, "TObj(3.1,3.2,3.3)")
obj.r = {4.1, 4.2}
obj.v = {5.3, 5.4}
obj.g =  6.5
check_val(function() return obj.r:tostring() end, "TVec(4.1,4.2)")
check_val(function() return obj.v:tostring() end, "TVec(5.3,5.4)")
check_val(function() return obj.g            end, 6.5)
check_val(function() return obj.z            end, nil)
check_err(function() obj.foo = nil end, "TObj can not assign 'foo'")
check_err(function() obj.r   = nil end, "bad value for TObj.r (TVec needs table with two numbers)")
check_err(function() obj.v   = nil end, "bad value for TObj.v (TVec needs table with two numbers)")
S.vort_list:clear()
check_val(function() return #S.vort_list end, 0)

fname = os.tmpname()
file = io.open(fname, "w")
file:write("1.1 1.2 1.3\n")
file:write("2e1 2e2 2e3\n")
file:write("inf nan 0.0\n")
io.close(file)
check_err(function() S.vort_list:load(fname)         end, nil)
check_val(function() return #S.vort_list             end, 3)
check_val(function() return tostring(S.vort_list[1]) end, "TObj(1.1,1.2,1.3)")
check_val(function() return tostring(S.vort_list[2]) end, "TObj(20,200,2000)")
check_val(function() return tostring(S.vort_list[3]) end, "TObj(inf,nan,0)")
check_val(function() return #S.vort_list:totable()   end, 3)
check_err(function() S.vort_list:load("/nowhere")    end, "can not load '/nowhere' (No such file or directory)")
check_err(function() S.vort_list:load("/proc/1/mem") end, "can not load '/proc/1/mem' (Permission denied)")
file = io.open(fname, "w")
file:write("txt txt txt\n")
file:write("3.1 3.2 3.3\n")
io.close(file)
check_err(function() S.vort_list:load(fname)         end, nil)
check_val(function() return #S.vort_list             end, 3)
check_val(function()
    local gsum = 0
    for _, obj in ipairs(S.vort_list) do
        gsum = gsum + obj.g
    end
    return gsum
end, 2001.3)

file = io.open(fname, "w")
file:write("\x66\x66\x66\x66\x66\x66\x10\x40") -- 4.1
file:write("\xCC\xCC\xCC\xCC\xCC\xCC\x10\x40") -- 4.2
file:write("\x33\x33\x33\x33\x33\x33\x11\x40") -- 4.3
file:write("\x33\x33\x33\x33\x33\x33\x16\x40") -- not read
file:write("\x33\x33\x33\x33\x33\x33\x16\x40") -- not read
file:write("\x33\x33\x33\x33\x33\x33\x16")     -- not read
io.close(file)
check_err(function() S.vort_list:load_bin(fname)     end, nil)
check_val(function() return #S.vort_list             end, 4)
check_val(function() return tostring(S.vort_list[4]) end, "TObj(4.1,4.2,4.3)")
check_val(function() return tostring(S.vort_list[5]) end, "nil")
os.remove(fname)

-- root_body, general_slip
fname = os.tmpname()
file = io.open(fname, "w")
file:write(" 1 0 1\n")
file:write(" 0 2 1\n")
file:write("-1 0 1\n")
io.close(file)
body1 = load_body(fname)
check_val(function() return body1.slip end, true)
check_err(function() body1.slip = true end)
check_err(function() body1.slip = false end)
check_err(function() body1.slip = 0 end, "bad value for TBody.slip (boolean expected, got number)")
check_err(function() body1.slip = nil end, "bad value for TBody.slip (boolean expected, got nil)")
check_val(function() return body1.slip end, false)
check_err(function() body1.root = body1 end)
check_val(function() return body1.root end, body1)
check_err(function() body1.root = nil end)
check_val(function() return body1.root end, nil)

file = io.open(fname, "w")
file:write(" 1 0 0\n")
file:write(" 0 2 1\n")
file:write("-1 0 0\n")
io.close(file)
body2 = load_body(fname)
check_val(function() return body2.slip end, nil)
check_err(function() body2.slip = true end)
check_val(function() return body2.slip end, true)
check_err(function() body2.root = body1 end)
body1 = nil
collectgarbage()
check_val(function() return body2.root end, nil)
body2 = nil
os.remove(fname)

-- gen_cylinder
check_err(function() gen_cylinder() end, "bad argument #1 to 'gen_cylinder' (table expected, got no value)")
check_err(function() gen_cylinder{ N={0} } end, "bad argument #1 to 'gen_cylinder' ('N' must be a number)")
check_err(function() gen_cylinder{ N=nan } end, "bad argument #1 to 'gen_cylinder' ('N' must be finite)")
check_err(function() gen_cylinder{ N=inf } end, "bad argument #1 to 'gen_cylinder' ('N' must be finite)")
check_err(function() gen_cylinder{ N=-11 } end, "bad argument #1 to 'gen_cylinder' ('N' must be positive)")
check_err(function() gen_cylinder{ dl={0} } end, "bad argument #1 to 'gen_cylinder' ('dl' must be a number)")
check_err(function() gen_cylinder{ dl=nan } end, "bad argument #1 to 'gen_cylinder' ('dl' must be finite)")
check_err(function() gen_cylinder{ dl=inf } end, "bad argument #1 to 'gen_cylinder' ('dl' must be finite)")
check_err(function() gen_cylinder{ dl=-11 } end, "bad argument #1 to 'gen_cylinder' ('dl' must be positive)")
check_err(function() gen_cylinder{ N=1, dl=1 } end, "bad argument #1 to 'gen_cylinder' ('N' and 'dl' are mutually exclusive)")
check_err(function() gen_cylinder{ R=1       } end, "bad argument #1 to 'gen_cylinder' (either 'N' or dl' must be specified)")
check_err(function() gen_cylinder{ R={0}, N=1 } end, "bad argument #1 to 'gen_cylinder' ('R' must be a number)")
check_err(function() gen_cylinder{ R=inf, N=1 } end, "bad argument #1 to 'gen_cylinder' ('R' must be finite)")
check_err(function() gen_cylinder{ R=nan, N=1 } end, "bad argument #1 to 'gen_cylinder' ('R' must be finite)")
check_err(function() gen_cylinder{ R=-11, N=1 } end, "bad argument #1 to 'gen_cylinder' ('R' must be positive)")
check_err(function() gen_cylinder{ R=1, N=1, foo=5 } end, "bad argument #1 to 'gen_cylinder' (excess parameter 'foo')")
check_val(function() return #gen_cylinder{ R=1, N=10 } end, 10)
collectgarbage()
check_val(function() return #gen_cylinder{ R=0.5/math.pi, dl=1/10 } end, 10)
collectgarbage()
local cyl = gen_cylinder{ R=1, N=500 }
check_dev(function() return cyl:get_arclen() end, 2*math.pi, 1e-4)
check_dev(function() return cyl:get_area() end, math.pi, 1e-4)
check_dev(function() return cyl:get_moi_c() end, math.pi/2, 1e-4)
check_err(function() cyl:move_r() end, "bad argument #1 for TBody.move_r (TVec needs table with two numbers)")
check_err(function() cyl:move_r(0) end, "bad argument #1 for TBody.move_r (TVec needs table with two numbers)")
check_err(function() cyl.move_r("", {1, 2}) end, "bad argument #1 to 'move_r' (TBody expected, got string)")
check_err(function() cyl:move_r({1, 2}) end, nil)
check_dev(function() return cyl.holder_pos.r.x end, 1, 1e-8)
check_dev(function() return cyl.holder_pos.r.y end, 2, 1e-8)
check_dev(function() return cyl.holder_pos.o end, 0, 1e-8)
check_dev(function() return cyl.delta_pos.r.x end, 0, 1e-8)
check_dev(function() return cyl.delta_pos.r.y end, 0, 1e-8)
check_dev(function() return cyl.delta_pos.o end, 0, 1e-8)
check_dev(function() return cyl:get_com()[1] end, 1, 1e-8)
check_dev(function() return cyl:get_com()[2] end, 2, 1e-8)
check_dev(function() return cyl:get_axis()[1] end, 1, 1e-8)
check_dev(function() return cyl:get_axis()[2] end, 2, 1e-8)
check_err(function() cyl.holder_pos.r = {0,0} end, nil)
check_err(function() cyl:move_o(math.pi/4) end, nil)
check_err(function() cyl:move_d(45) end, nil)
check_dev(function() return cyl:get_com()[1] end, -2, 1e-8)
check_dev(function() return cyl:get_com()[2] end, 1, 1e-8)
check_dev(function() return cyl:get_axis()[1] end, 0, 1e-8)
check_dev(function() return cyl:get_axis()[2] end, 0, 1e-8)
cyl = nil
collectgarbage()

-- gen_plate
check_err(function() gen_plate() end, "bad argument #1 to 'gen_plate' (table expected, got no value)")
check_err(function() gen_plate{ N={0} } end, "bad argument #1 to 'gen_plate' ('N' must be a number)")
check_err(function() gen_plate{ N=nan } end, "bad argument #1 to 'gen_plate' ('N' must be finite)")
check_err(function() gen_plate{ N=inf } end, "bad argument #1 to 'gen_plate' ('N' must be finite)")
check_err(function() gen_plate{ N=-11 } end, "bad argument #1 to 'gen_plate' ('N' must be positive)")
check_err(function() gen_plate{ dl={0} } end, "bad argument #1 to 'gen_plate' ('dl' must be a number)")
check_err(function() gen_plate{ dl=nan } end, "bad argument #1 to 'gen_plate' ('dl' must be finite)")
check_err(function() gen_plate{ dl=inf } end, "bad argument #1 to 'gen_plate' ('dl' must be finite)")
check_err(function() gen_plate{ dl=-11 } end, "bad argument #1 to 'gen_plate' ('dl' must be positive)")
check_err(function() gen_plate{ N=1, dl=1 } end, "bad argument #1 to 'gen_plate' ('N' and 'dl' are mutually exclusive)")
check_err(function() gen_plate{ R1=1       } end, "bad argument #1 to 'gen_plate' (either 'N' or dl' must be specified)")
check_err(function() gen_plate{ R1={0}, N=1 } end, "bad argument #1 to 'gen_plate' ('R1' must be a number)")
check_err(function() gen_plate{ R1=inf, N=1 } end, "bad argument #1 to 'gen_plate' ('R1' must be finite)")
check_err(function() gen_plate{ R1=nan, N=1 } end, "bad argument #1 to 'gen_plate' ('R1' must be finite)")
check_err(function() gen_plate{ R1=-11, N=1 } end, "bad argument #1 to 'gen_plate' ('R1' must be positive)")
check_err(function() gen_plate{ R1=1, N=1 } end, "bad argument #1 to 'gen_plate' ('R2' must be a number)")
check_err(function() gen_plate{ R1=1, R2=1, N=1 } end, "bad argument #1 to 'gen_plate' ('L' must be a number)")
check_err(function() gen_plate{ R1=1, R2=1, L=5, N=1, foo=5 } end, "bad argument #1 to 'gen_plate' (excess parameter 'foo')")
check_val(function() return #gen_plate{ R1=0.5,    R2=0.5,    L=1, N=100 } end, 100)
collectgarbage()
check_val(function() return #gen_plate{ R1=0.5/pi, R2=0.5/pi, L=1, dl=1/100 } end, 100+200)
collectgarbage()
local plate = gen_plate{ R1=1, R2=1, L=2, N=1000 }
check_dev(function() return plate:get_arclen() end, 2*math.pi+4, 1e-4)
check_dev(function() return plate:get_area() end, math.pi+4, 1e-4)
check_dev(function() return plate:get_com()[1] end, 1, 1e-8)
check_dev(function() return plate:get_com()[2] end, 0, 1e-8)
check_err(function() plate.delta_pos = {2, 0, math.pi} end, nil)
check_dev(function() return plate:get_axis()[1] end, 2, 1e-8)
check_dev(function() return plate:get_axis()[2] end, 0, 1e-8)
check_err(function() plate:move_d(90) end, nil)
check_dev(function() return plate.holder_pos.r.x end, 0, 1e-8)
check_dev(function() return plate.holder_pos.r.y end, 0, 1e-8)
check_dev(function() return plate.holder_pos.d end, 90, 1e-8)
check_dev(function() return plate.delta_pos.r.x end, 2, 1e-8)
check_dev(function() return plate.delta_pos.r.y end, 0, 1e-8)
check_dev(function() return plate.delta_pos.d end, 180, 1e-8)
check_dev(function() return plate:get_com()[1] end, 2, 1e-8)
check_dev(function() return plate:get_com()[2] end, -1, 1e-8)
check_dev(function() return plate:get_axis()[1] end, 2, 1e-8)
check_dev(function() return plate:get_axis()[2] end, 0, 1e-8)
plate = nil
collectgarbage()

-- TBodyList
local cyl = gen_cylinder{R=0.5, N=200}
cyl.label = "cyl"
check_err(function() S.body_list.foo = nil end, "TBodyList can not assign anything")
check_err(function() S.body_list[0] = nil end, "TBodyList can not assign anything")
check_err(function() S.body_list:erase(cyl) end, "bad argument #1 to 'erase' (the body is not in list)")
check_err(function() S.body_list:erase(cyl) end, "bad argument #1 to 'erase' (the body is not in list)")
check_err(function() S.body_list:erase(cyl) end, "bad argument #1 to 'erase' (the body is not in list)")
check_err(function() S.body_list:erase(cyl) end, "bad argument #1 to 'erase' (the body is not in list)")
check_err(function() S.body_list:erase(cyl) end, "bad argument #1 to 'erase' (the body is not in list)")
check_err(function() S.body_list:erase(cyl) end, "bad argument #1 to 'erase' (the body is not in list)")
check_err(function() S.body_list:erase(cyl) end, "bad argument #1 to 'erase' (the body is not in list)")
check_err(function() S.body_list:insert(cyl) end, nil)
check_err(function() S.body_list:insert(cyl) end, "bad argument #1 to 'insert' (can not insert the body twice)")
check_val(function() return #S.body_list end, 1)
check_err(function() S.body_list:clear() end, nil)
check_val(function() return #S.body_list end, 0)
cyl = nil
collectgarbage()

local cyl1 = gen_cylinder{R=0.5, N=200}
cyl1.label = "cyl1"
cyl1:move_r({1, 0})
check_err(function() S.body_list:insert(cyl1) end, nil)
local cyl2 = gen_cylinder{R=0.5, N=200}
cyl2.label = "cyl2"
cyl2:move_r({-1, 0})
check_err(function() S.body_list:insert(cyl2) end, nil)
local blist = {}
for i, b in ipairs(S.body_list) do
    blist[i] = b
    check_val(function() return S.body_list[i].label end, b.label)
    collectgarbage()
end
for i, b in ipairs({"cyl1", "cyl2"}) do
    check_val(function() return blist[i].label end, b)
end
collectgarbage()

if FAIL ~= 0 then
    error("There were errors")
end
