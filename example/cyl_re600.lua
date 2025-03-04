#!/usr/bin/env vvflow
-- See `man vvflow` for the full reference

S.inf_vx = "1"
S.re = 600
S.dt = 0.05
S.dt_save = 0.5
S.finish = 20

cyl = gen_cylinder{R=0.5, N=350}
cyl.label = "cyl"
S.body_list:insert(cyl)

S.caption = "cyl_re600"
local filename = S.caption..".h5"
S:save(filename)
print('Written ' .. filename)
