cmake_minimum_required (VERSION 3.0)
project (vvscripts)

install(PROGRAMS
    gpquick
    vvawk.ampl
    vvawk.avg
    vvawk.drv
    vvawk.mavg
    vvawk.sd
    vvawk.zeros
    vvencode
    DESTINATION bin
)
