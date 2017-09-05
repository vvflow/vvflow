vvxtract(1) -- extract data from h5 files obtained with vvflow program
====

## SYNOPSIS

`vvxtract` [<OPTIONS>] <FILE> [<DATASET>]

## DESCRIPTION

vvxtract is a part of Vvflow CFD Suite. It serves as a postprocessor for extracting obtained CFD simulation results.

vvxtract writes data to the standard output in text or binary format.

Without <DATASET> specified vvxtract runs with `--info` option enabled.

## OPTIONS

  * -v, --version :
    print program version and exit

  * -h, --help :
    show manpage and exit

  * --list:
    list all datasets in file and exit

  * --info:
    show general information about the file and exit.
    The optut is:
    * internal caption (hdf attribute 'caption');
    * creation time (hdf attribute 'time_local');
    * creator version (hdf attributes 'git_info', 'git_rev').
    This is default when no dataset is specified.
    
  * -f, --format [text|float|double]:
    dataset output format.
    Default is `text`.

  * --xyvalue:
    rearrange output in form (x, y, value).
    Raise error if <DATASET> has no margins attributes.

  * --margins:
    prepend matrix with `xmin`, `xmax`, `ymin`, `ymax`, `spacing` attributes.
    Raise error if <DATASET> has no margins attributes.
    Ignored in `--xyvalue` mode.
