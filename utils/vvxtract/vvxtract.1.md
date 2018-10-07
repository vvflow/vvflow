vvxtract(1) -- extract data from h5 files obtained with vvflow program
====

## SYNOPSIS

`vvxtract` [<OPTIONS>] <FILE> [<DATASET> ...]

## DESCRIPTION

vvxtract is a part of Vvflow CFD Suite. It serves as a postprocessor for extracting obtained CFD simulation results.

<FILE> is a hdf5 file, containing either a _Space_ from `vvcompose` or _stepdata_ from `vvflow`.

Without <DATASET> specified vvxtract runs with `--info` option enabled.
It shows general information about the file: caption, creation time, creator version.
When <FILE> is a _Space_ it also shows attributes of the space and all bodies in vvcompose-like style. 

With one or more <DATASET> option specified
vvxtract merges lines of all datasets.

## OPTIONS

  * -v, --version :
    print program version and exit

  * -h, --help :
    show manpage and exit

  * -l, --list:
    list all datasets in file and exit

  * -i, --info:
    show general information about the file and exit

## SEE ALSO
  vvcompose(1)
