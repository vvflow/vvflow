#!/usr/bin/env vvflow

S.inf_vx = "0"
S.re = 600
S.dt = 0.005
S.dt_save = 0.005
S.finish = 0.005

cyl = gen_cylinder{R=0.5, N=350}
cyl.label = "cyl"
cyl.holder_pos.r = {0.2, 0}
cyl.delta_pos.r = {-0.2, 0}
S.body_list:insert(cyl)

N = 500
for i = 1, N do
	rad = 0.5 * i/N * math.sin(i)
	phi = i/N * 10*math.pi
	x = 0.6 + rad*math.cos(phi)
	y = 0.6 + rad*math.sin(phi)
	S.vort_list:append({x, y, 1/N})
	S.vort_list:append({x, -y, -1/N})
	S.vort_list:append({-x, y, -1/N})
	S.vort_list:append({-x, -y, 1/N})
end

S.caption = "cyl"
S:save(arg[1])
