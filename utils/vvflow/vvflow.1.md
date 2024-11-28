vvflow(1) -- solve the CFD problem
====

## SYNOPSIS

`vvflow` [_options_] _file_

## DESCRIPTION

**vvflow** is the main tool of the Vvflow CFD Suite.
It performs the simulation and solves the CFD problem.

Problem statement is loaded from _file_ - a Lua script specifying the
CFD problem. For the backward compatibility the _file_ can also be an
hdf5 file copied from another simulation results.

## OPTIONS

  * -v, --version :
    print program version and exit

  * -h, --help :
    print help and exit

  * --progress :
    print progress during the simulation

  * --profile :
    save pressure and friction profiles along bodies surfaces to the stepdata file

## SEE ALSO
  vvxtract(1), vvplot(1)
