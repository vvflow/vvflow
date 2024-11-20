#!/usr/bin/env vvcompose

S.inf_vx = "0"
S.re = 600
S.dt = 0.005
S.dt_save = 0.005
S.finish = 0.005

cyl = gen_cylinder{R=0.5, N=350}
cyl:move_r({0.005, 0})
cyl.label = "cyl"
cyl.holder_vx = "-1"
S.body_list:insert(cyl)

S.caption = "ref_b"
S:save(arg[1])
