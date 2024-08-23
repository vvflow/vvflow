#!/usr/bin/env python3

import os
from conftest import Env

tempdir = '/tmp/test_vvflow'


def test_args(env: Env):
    ret = env.run([
        "vvflow",
        os.path.join(env.cwd, "echo_args.lua"),
        "bar",
        "baz",
        "--progress",
        "boo"])
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

    ret = env.run(["vvflow", env.tmp("run2.lua")])
    assert ret.stderr.decode("utf-8") == "Boo\n"
