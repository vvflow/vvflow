#!/usr/bin/env vvflow

S.inf_vx = "0"
S.re = 600
S.dt = 0.005
S.dt_save = 0.005
S.finish = 0.005

plate = gen_plate{R1=0.04, R2=0.04, L=1, N=400}
plate.label = "plate"
plate:move_r({-0.5, 0})
plate.holder_pos = {0, 0, 0}
S.body_list:insert(plate)

N = 500
for i = 1, N do
	rad = 0.2 * i/N * math.sin(i)
	phi = i/N * 10*math.pi
	x = 0.25 + rad*math.cos(phi)
	y = 0.25 + rad*math.sin(phi)
	S.vort_list:append({ x,  y, 1/N})
	S.vort_list:append({-x, -y, 1/N})
end

S.caption = "plate"
S:save(arg[1])
