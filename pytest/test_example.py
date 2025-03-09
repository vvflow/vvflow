#!/usr/bin/env python3
import os
import shutil

tempdir = "/tmp/vvflow_test_results/test_example"


def test(env):
    lua = env.tmp("cyl_re600.lua")
    shutil.copy(os.path.join(env.cwd, "../example/cyl_re600.lua"), lua)
    with open(lua, "a") as f:
        f.write("S.finish = 1\n")
        f.write("simulate()\n")

    env.vvflow(lua)
    stepdata = env.tmp("stepdata_cyl_re600.h5")

    ret = env.vvxtract(stepdata, "--list")
    assert ret.stdout.decode().strip().split("\n") == [
        "body00/delta_position",
        "body00/force_friction",
        "body00/force_holder",
        "body00/force_hydro",
        "body00/holder_position",
        "body00/speed_slae",
        "time",
    ]

    ret = env.vvxtract(stepdata, ["time", "body00/force_hydro"])
    ret_lines = ret.stdout.decode().strip().split("\n")
    assert ret_lines[0].strip().split() == [
        "#time",
        "body00/force_hydro[1]",
        "body00/force_hydro[2]",
        "body00/force_hydro[3]",
    ]
    assert float(ret_lines[1].strip().split()[0]) == 0
    assert float(ret_lines[-1].strip().split()[0]) == 1
    assert len(ret_lines) == 22

    env.vvplot(
        # fmt: skip
        "results_cyl_re600/000020.h5",
        "000020.png",
        args="-BV -x -1,4 --size 480x360",
    )

    cmd = f"""
        set -e
        mkdir -p images_cyl_re600
        for f in $(ls results_cyl_re600/*.h5); do
            {env.which["vvplot"]} $f images_cyl_re600 -BV -x -2,20
            echo $f
        done
    """
    env.run(cmd, shell=True, cwd=env.tempdir)
    assert sorted(os.listdir(env.tmp("images_cyl_re600"))) == [
        "000000.png",
        "000010.png",
        "000020.png",
    ]

    cmd = f"""
        set -e
        {env.which["vvxtract"]} stepdata_cyl_re600.h5 time body00/force_holder \\
        | {env.which["gpquick"]} --points -u 1:3 -y -0.5 0.5 \\
            --size 480 360 -o fy_raw.png --dump-script;
    """
    ret = env.run(cmd, shell=True, cwd=env.tempdir)
    ret_lines = ret.stdout.decode().strip().split("\n")
    assert "set terminal pngcairo enhanced size 480, 360;" in ret_lines
    assert "set output 'fy_raw.png';" in ret_lines
    assert "plot '-' using 1:3 with linespoints lt 0 pt 7 lc rgb 'black';" in ret_lines
