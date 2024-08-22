#!/bin/cat

assert(_G.arg)
assert(type(_G.arg) == "table")
assert(type(_G.arg[0]) == "string")
for i = 1, #arg do print(arg[i]) end
