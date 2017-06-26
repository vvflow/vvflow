function printf(fmt, ...)
	print(string.format(fmt, ...))
end

FAIL = 5

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
check_err(function() return S.foo end, "S has no member 'foo'")
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
check_val(function() return tostring(S.gravity) end, "TVec(15.1,15.2)")
check_val(function() return S.gravity:tostring() end, "TVec(15.1,15.2)")
check_val(function() return S.gravity.x end, 15.1)
check_val(function() return S.gravity.y end, 15.2)
check_val(function() return S.gravity:abs2() end, 15.1^2 + 15.2^2)
check_val(function() return S.gravity:abs() end, math.sqrt(15.1^2 + 15.2^2))

-- inf_vx
-- inf_vy

os.exit(FAIL)
