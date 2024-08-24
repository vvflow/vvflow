#!/usr/bin/env vvflow

S.dt = 0.1
S.dt_save = 0.3
S.finish = 1
S.caption = "nothing"
S:save("nothing.h5")

simulate()
