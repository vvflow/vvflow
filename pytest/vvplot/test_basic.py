#!/usr/bin/env python3
import os
import pytest

tempdir = "/tmp/test_vvplot"


@pytest.fixture(scope="module")
def env_cyl(env):
    hdf = "cyl.h5"
    env.vvcompose("compose_cyl.lua", hdf)
    env.vvflow(hdf, timeout=30)
    return env


def test_cyl(env_cyl):
    h5 = "results_cyl/000001.h5"

    def check(ofile, args):
        ret = env_cyl.vvplot(h5, ofile, args)
        assert not ret.stdout
        assert not ret.stderr

    check("opts.gray.png", args="--size 720x720 -x -1.5,1.5 -y -1.5,1.5 -BV --gray")
    check("opts.spring.png", args="--size 720x720 -x -1.5,1.5 -y -1.5,1.5 -BV --spring")
    check("opts.holder.png", args="--size 720x720 -x -1.5,1.5 -y -1.5,1.5 -BV --holder")
    check("opts.default.png", args="--size 720x720 -x -1.5,1.5 -y -1.5,1.5 -BV")
    check(
        "opts.colorbox.png",
        args="--size 720x720 -x -1.5,1.5 -y -1.5,1.5 -BV --G=5 --colorbox --res-hi 256",
    )
    check(
        "opts.no-timelabel.png",
        args="--size 720x720 -x -1.5,1.5 -y -1.5,1.5 -BV --no-timelabel",
    )

    check("cyl.bga.png", args="-x -3,3 -B -G --res-hi 256")
    check("cyl.bg5.png", args="-x -3,3 -B --G=5 --res-hi 256")
    check("cyl.bv4.png", args="-x -3,3 -B --V=4")
    check("cyl.bv0.png", args="-x -3,3 -B --V=0")
    check("cyl.bsa.png", args="-x -3,3 -B -S")
    check("cyl.bp.png", args="-x -3,3 -B --P=-15,15 --res-hi 256 --colorbox")
    check("cyl.bs1.png", args="-x -3,3 -B --S=-10,10,0.1")


@pytest.fixture(scope="module")
def env_plate(env):
    hdf = "plate.h5"
    env.vvcompose("compose_plate.lua", hdf)
    return env


def test_plate(env_plate):
    h5 = "plate.h5"

    def check(ofile, args):
        ret = env_plate.vvplot(h5, ofile, args)
        assert not ret.stdout
        assert not ret.stderr

    check(
        "plate.bg.png",
        args="--size 720x720 -x -1.5,1.5 -y -1.5,1.5 -BG --colorbox --res-hi 256",
    )
    check(
        "plate.bp.png",
        args="--size 720x720 -x -1.5,1.5 -y -1.5,1.5 -BP --colorbox --res-hi 256",
    )
