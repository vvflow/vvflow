#!/usr/bin/env python3
import os
import pytest

tempdir = '/tmp/test_vvplot'

@pytest.fixture(scope='module')
def env_cyl(env):
    hdf = 'base.h5'
    env.vvcompose('compose_cyl.lua', hdf)
    env.vvflow(hdf, timeout=30)
    return env

def test(env_cyl):
    # ref_o
    h5 = 'results_base/000001.h5'
    def check(ofile, args):
        ret = env_cyl.vvplot3(h5, ofile, args)
        assert not ret.stdout
        assert not ret.stderr

    check('opts.gray.png',         args='--size 720x720 -x -1.5,1.5 -y -1.5,1.5 -BV --gray')
    check('opts.spring.png',       args='--size 720x720 -x -1.5,1.5 -y -1.5,1.5 -BV --spring')
    check('opts.holder.png',       args='--size 720x720 -x -1.5,1.5 -y -1.5,1.5 -BV --holder')
    check('opts.default.png',      args='--size 720x720 -x -1.5,1.5 -y -1.5,1.5 -BV')
    check('opts.colorbox.png',     args='--size 720x720 -x -1.5,1.5 -y -1.5,1.5 -BV --G=5 --colorbox --res-hi 256')
    check('opts.no-timelabel.png', args='--size 720x720 -x -1.5,1.5 -y -1.5,1.5 -BV --no-timelabel')

    check('base.bga.png', args='-x -3,3 -B -G --res-hi 256')
    check('base.bg5.png', args='-x -3,3 -B --G=5 --res-hi 256')
    check('base.bv4.png', args='-x -3,3 -B --V=4')
    check('base.bv0.png', args='-x -3,3 -B --V=0')
    check('base.bsa.png', args='-x -3,3 -B -S')
    check('base.bp.png',  args='-x -3,3 -B --P=-15,15 --res-hi 256 --colorbox')
    check('base.bs1.png', args='-x -3,3 -B --S=-10,10,0.1')
    # env.vvplot3(ret.ofile, '.')
