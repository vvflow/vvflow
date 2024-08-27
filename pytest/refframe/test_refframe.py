#!/usr/bin/env python3

import numpy
import struct
import logging

tempdir = "/tmp/test_refframe"


def test(env):
    # ref_o
    hdf = "ref_o.h5"
    env.vvcompose("compose_ref_o.lua", hdf)
    env.vvflow(hdf)
    ret = env.vvplot(
        # fmt: skip
        "results_ref_o/000001.h5",
        "ref_o.tar",
        args="-x -1.5,1.5 -B -S --ref-S o --tar",
    )
    env.vvplot(ret.ofile, ".")
    sf_o = env.run(["tar", "-xOf", ret.ofile, "map_streamfunction"]).stdout
    sf_o = struct.unpack("f" * int(len(sf_o) / 4), sf_o)

    # ref_b
    hdf = "ref_b.h5"
    env.vvcompose("compose_ref_b.lua", hdf)
    env.vvflow(hdf)
    ret = env.vvplot(
        # fmt: skip
        "results_ref_b/000001.h5",
        "ref_b.tar",
        args="-x -1.5,1.5 -B -S --ref-S b --tar",
    )
    env.vvplot(ret.ofile, ".")
    sf_b = env.run(["tar", "-xOf", ret.ofile, "map_streamfunction"]).stdout
    sf_b = struct.unpack("f" * int(len(sf_b) / 4), sf_b)

    assert len(sf_o) == len(sf_b)

    spread_o = numpy.percentile(sf_o, 95) - numpy.percentile(sf_o, 5)
    spread_b = numpy.percentile(sf_o, 95) - numpy.percentile(sf_o, 5)
    logging.info("spread_o = {}".format(spread_o))
    logging.info("spread_b = {}".format(spread_b))

    abserror = numpy.absolute(numpy.subtract(sf_o, sf_b))
    logging.info("abserror(80%) = {}".format(numpy.percentile(abserror, 80)))

    e = 2 * numpy.percentile(abserror, 80) / (spread_o + spread_b)
    logging.info("e = {}".format(e))

    assert e < 0.001
