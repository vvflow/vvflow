#!/usr/bin/env python3

import os
import pytest
from conftest import Env
from subprocess import CalledProcessError

tempdir = "/tmp/vvflow_test_results/test_vvflow"


def test_args(env: Env):
    ret = env.run(
        [
            "vvflow",
            os.path.join(env.cwd, "echo_args.lua"),
            "bar",
            "baz",
            "--progress",
            "boo",
        ]
    )
    assert ret.stdout == b"bar\nbaz\nboo\n"


def test_run1(env: Env):
    s = """
        print("hello vvflow")
    """
    with open(env.tmp("run1.lua"), "w") as f:
        f.write(s)

    ret = env.run(["vvflow", env.tmp("run1.lua")])
    assert ret.stdout == b"hello vvflow\n"
    assert ret.stderr == b""


def test_run2(env: Env):
    with open(env.tmp("run2.lua"), "w") as f:
        f.write("error('Boo', 0)")

    with pytest.raises(CalledProcessError) as e:
        env.run(["vvflow", env.tmp("run2.lua")])

    assert e.value.stderr.decode("utf-8") == "Boo\n"
    assert e.value.returncode == 8  # hardcoded in vvflow.cpp


def test_run3(env: Env):
    script = os.path.join(env.cwd, "nothing.lua")
    env.run([env.which["vvflow"], script], cwd=tempdir)
    assert os.path.exists(tempdir + "/nothing.h5")
    assert os.path.exists(tempdir + "/stepdata_nothing.h5")
    results = os.listdir(tempdir + "/results_nothing")
    results.sort()
    assert results == [
        "000000.h5",
        "000005.h5",
        "000010.h5",
        "000012.h5",
        "000016.h5",
        "000020.h5",
    ]
