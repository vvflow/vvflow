function printf(fmt, ...)
	print(string.format(fmt, ...))
end

FAIL = 42

function check_err(fn, err_need)
	ret, err_have = pcall(fn)
	info = debug.getinfo(2)
	err_need = string.format("%s:%s: %s", info.short_src, info.currentline, err_need)
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

-- space_getindex, space_setindex
check_err(function() S.foo = nil end, "S can not assign 'foo'")
check_val(function() return S.foo end, nil)
check_err(function() return S.load() end, "bad argument #1 to 'load' (S expected, got no value)")
check_err(function() return S.save() end, "bad argument #1 to 'save' (S expected, got no value)")
check_err(function() return S:load() end, "bad argument #1 to 'load' (string expected, got no value)")
check_err(function() return S:save() end, "bad argument #1 to 'save' (string expected, got no value)")
S:load("/dev/null")
S:save("/dev/null")


-- luavvd_getstring, luavvd_setstring
S.caption = "good_caption"
check_val(function() return S.caption end, "good_caption")
check_err(function() S.caption = 100 end, "bad value for S.caption (string expected, got number)")
check_err(function() S.caption = {1} end, "bad value for S.caption (string expected, got table)")
check_err(function() S.caption = S.gravity end, "bad value for S.caption (string expected, got userdata)")
check_err(function() S.caption = nil end, "bad value for S.caption (string expected, got nil)")
check_val(function() return S.caption end, "good_caption")

-- inf, nan
check_val(function() return  inf end,  1/0)
check_val(function() return -inf end, -1/0)
check_val(function() return tostring( inf) end,  "inf")
check_val(function() return tostring(-inf) end, "-inf")
check_val(function() return tostring( nan) end,  "nan")

-- luavvd_getdouble, luavvd_setdouble
S.re = inf
check_val(function() return S.re end, inf)
S.re = "10"
check_val(function() return S.re end, 10)
S.re = "+11e0"
check_val(function() return S.re end, 11)
S.re = 3*4
check_val(function() return S.re end, 12)
check_err(function() S.re = nil end, "bad value for S.re (number expected, got nil)")
check_err(function() S.re = false end, "bad value for S.re (number expected, got boolean)")
check_val(function() return S.re end, 12)
S.finish = 13
check_val(function() return S.finish end, 13)
S.inf_g = 14
check_val(function() return S.inf_g end, 14)

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



os.exit(FAIL)
