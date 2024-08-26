#!/usr/bin/env vvflow

S.dt = 0.1
S.dt_save = 0.5
S.finish = 1
S.caption = "nothing"
S:save("nothing.h5")

simulate()

S.dt_save = 0.4
S.finish = 2

simulate()
