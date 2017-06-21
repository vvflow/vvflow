#!/usr/bin/env vvcompose2

-- S.dt = 0.0300000000001
-- print(S.dt)
-- S.dt = {-99, 100}
-- print(S.dt)
function printf(fmt, ...)
	print(string.format(fmt, ...))
end

function vvxtract()
	-- printf("General:\n");
	-- attribute_print("Caption:", S->caption);
	-- attribute_print("Time:", S->Time);
	-- attribute_print("dt:", S->dt);
	-- attribute_print("Save dt:", S->dt_save);
	-- attribute_print("Streak dt:", S->dt_streak);
	-- attribute_print("Profile dt:", S->dt_profile);
	-- attribute_print("1/nyu:", S->Re);
	-- attribute_print("Pr:", S->Pr);
	-- attribute_print("Inf marker:", S->InfMarker);
	-- attribute_print("Inf speed_x:", S->InfSpeedX);
	-- attribute_print("Inf speed_y:", S->InfSpeedY);
	-- attribute_print("Inf gamma:", S->InfCirculation);
	-- attribute_print("Gravity:", S->gravitation);
	-- attribute_print("Finish:", S->Finish);
	-- attribute_print("VortexList:", long(S->VortexList.size()));
	-- attribute_print("HeatList:", long(S->HeatList.size()));
	-- attribute_print("InkList:", long(S->StreakList.size()));
	-- attribute_print("InkSourceList:", long(S->StreakSourceList.size()));
	-- attribute_print("SourceList:", long(S->SourceList.size()));
	print("General:")
	print("  caption    ", string.format("'%s'", S.caption) )
	print("  time       ", S.time)
	print("  dt         ", S.dt)
	print("  dt_save    ", S.dt_save)
	print("  dt_streak  ", S.dt_streak)
	print("  dt_profile ", S.dt_profile)
	print("  finish     ", S.finish)

	print("  re         ", S.re)
	print("  gravity    ", string.format("(%g, %g)", S.gravity.x, S.gravity.y) )
	print("  inf_g      ", S.inf_g)
	print("  inf_vx     ", string.format("'%s' = %g", S.inf_vx, S.inf_vx:eval(S.time)) )
	print("  inf_vy     ", string.format("'%s' = %g", S.inf_vy, S.inf_vy:eval(S.time)) )
end

S.caption = "Foo" .." ".. tostring(S.dt) .." ".. tostring(S.gravity)
S.inf_vx = "atan(3.2*pi)"
S.inf_vy = tostring(S.inf_vx) .. "+1"
-- S.inf_vx = "sin(t*0.1+k)"
S.dt = "1/100"
S.dt = {1.3, 10}

local cyl = gen_cylinder{N=500}
cyl.label = "cyl11"
-- print(cyl.label)
print(cyl.label, #cyl)
-- <TBody 'cyl'>

vvxtract()

function dump_table(tbl)
	for k, v in pairs(tbl) do
		print(k, "= ", v)
	end
end
-- dump_table(getmetatable(S))

cjson = require "cjson"
S.vort_list:clear()
-- S.sink_list:clear()
S.sink_list:append({0, 0, 1100})
S.sink_list:append({0, 0, 2100})
S.sink_list:append({0, 1, 3100}, {2,4,2})

function print_ipairs(val)
	print("ipairs:")
	for i, obj in ipairs(val) do
		print(i, obj)
	end
end

function print_pairs(val)
	print("pairs:")
	for i, obj in pairs(val) do
		print(i, obj)
	end
end

-- a1 = {"one", "two", "three"}
-- print_ipairs(a1)
-- print_pairs(a1)
-- a2 = {[0]=0, [1]=1, [2]=2, five= 5, four= 4, zero= 0}
-- print_ipairs(a2)
-- print_pairs(a2)


for i = 1, #S.sink_list+3 do
	print(i, "---", S.sink_list[i])
	-- printf("[%d] = %s", i, tostring(obj))
end

print("Test next")
for j, obj in next, S.sink_list:totable() do
	print(j, obj)
end
-- S.sink_list.foo = 5
-- dump_table(S.sink_list:totable())
-- dump_table(S.sink_list:totable())
S.sink_list:append({0, 7.77, 1200})
S.sink_list:append({0, 7.77, 2200})
S.sink_list:append({0, 7.77, 3200})
S.sink_list:load("/vvflow/tmp.txt")
dump_table(S.sink_list:totable())
print(cjson.encode(S.sink_list:totable()))

S:save("test_space.h5")
print("FIN")

