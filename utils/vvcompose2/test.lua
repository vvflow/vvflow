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


check_err(function() S.foo = nil end, "S can not assign 'foo'")
check_err(function() return S.foo end, "S has no member 'foo'")

S.caption = "good_caption"
check_val(function() return S.caption end, "good_caption")
check_err(function() S.caption = 100 end, "bad value for S.caption (string expected, got number)")
check_err(function() S.caption = {1} end, "bad value for S.caption (string expected, got table)")
check_err(function() S.caption = S.gravity end, "bad value for S.caption (string expected, got userdata)")
check_err(function() S.caption = nil end, "bad value for S.caption (string expected, got nil)")
check_val(function() return S.caption end, "good_caption")
-- caption
-- re
-- finish
-- inf_g
-- inf_vx
-- inf_vy
-- gravity

os.exit(FAIL)
