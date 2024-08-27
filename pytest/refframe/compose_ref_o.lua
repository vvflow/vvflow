#!/usr/bin/env vvflow

S.inf_vx = "1"
S.re = 600
S.dt = 0.005
S.dt_save = 0.005
S.finish = 0.005

cyl = gen_cylinder{R=0.5, N=350}
cyl.label = "cyl"
-- cyl.holder_vx = "0"
S.body_list:insert(cyl)

S.caption = "ref_o"
S:save(arg[1])
