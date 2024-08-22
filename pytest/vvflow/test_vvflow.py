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
