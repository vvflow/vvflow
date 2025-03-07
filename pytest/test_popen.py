#!/usr/bin/env python3

import subprocess

tempdir = "/tmp/vvflow_test_results/test_popen"


def test(env):
    try:
        env.run("false")
    except subprocess.CalledProcessError as e:
        assert e.returncode == 1
    else:
        raise RuntimeError("process did not raise an error")

    try:
        env.run("exit 2", shell=True)
    except subprocess.CalledProcessError as e:
        assert e.returncode == 2
    else:
        raise RuntimeError("process did not raise an error")

    stdout = env.run(["echo", "foobar"]).stdout
    assert stdout == b"foobar\n"

    stderr = env.run("echo barbaz >&2", shell=True).stderr
    assert stderr == b"barbaz\n"

    try:
        env.run(["sleep", "1"], timeout=0.1)
    except subprocess.TimeoutExpired:
        pass
    else:
        raise RuntimeError("process did not raise timeout error")
