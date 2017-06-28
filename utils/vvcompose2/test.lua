function printf(fmt, ...)
    print(string.format(fmt, ...))
end

FAIL = 42

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
body = load_body("/dev/null")
check_val(function() return #body                         end, 0)
check_val(function() return body.label                    end, "")
check_val(function() return tostring(body.holder_pos)     end, "TVec3D(0,0,0)" )
check_val(function() return tostring(body.delta_pos)      end, "TVec3D(0,0,0)" )
check_val(function() return tostring(body.speed)          end, "TVec3D(0,0,0)" )
check_val(function() return tostring(body.collision_min)  end, "TVec3D(nan,nan,nan)")
check_val(function() return tostring(body.collision_max)  end, "TVec3D(nan,nan,nan)")
check_val(function() return tostring(body.spring_const)   end, "TVec3D(inf,inf,inf)")
check_val(function() return tostring(body.spring_damping) end, "TVec3D(0,0,0)")
check_val(function() return tostring(body.holder_vx)      end, "")
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
check_val(function() return body.holder_pos.r.x end, 16.1)
check_val(function() return body.holder_pos.r.y end, 16.2)
check_val(function() return body.holder_pos.o   end, 16.3)
check_val(function() return body.holder_pos.d   end, 16.3*180/math.pi)
check_val(function() return body.holder_pos.z   end, nil)

-- ObjList
check_val(function() return #S.vort_list          end, 0)
check_val(function() return #S.sink_list          end, 0)
check_val(function() return #S.streak_source_list end, 0)
check_val(function() return #S.streak_domain_list end, 0)
check_val(function() return S.vort_list.foo       end, nil)
check_val(function() return S.vort_list[0]        end, nil)
check_val(function() return S.vort_list[1]        end, nil)
check_err(function() S.vort_list = S.sink_list    end, "S can not assign 'vort_list'")
check_err(function() S.vort_list.foo = nil end, "TList can not assign anything")
check_err(function() S.vort_list[0]  = nil end, "TList can not assign anything")
check_err(function() S.vort_list[1]  = nil end, "TList can not assign anything")
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


os.exit(FAIL)
