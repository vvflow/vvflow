#!/usr/bin/env python3

import os
import py
import pytest
import logging
import tempfile
import shutil
import subprocess

logging.basicConfig(level=logging.INFO, format=" %(levelname)7s %(name)s > %(message)s")


class Env:
    def __init__(self, cwd, tempdir):
        self.cwd = cwd
        self.tempdir = tempdir

        self.env = os.environ.copy()
        # path = [
        #     'utils/vvcompose',
        #     'utils/vvxtract',
        #     'utils/vvplot',
        #     self.env['PATH']
        # ]
        # self.env['PATH'] = ':'.join(path)
        logging.warning('export PATH="%s"' % self.env["PATH"])
        self.which = {}
        for prog in ["vvflow", "vvplot"]:
            bin = shutil.which(prog)
            assert bin, f"Command '{prog}' not fount in PATH"
            self.which[prog] = os.path.abspath(bin)

    def run(self, cmd, **kwargs):
        timeout = 10
        if "timeout" in kwargs.keys():
            timeout = kwargs["timeout"]
            del kwargs["timeout"]

        logging.info("Running: '{}'".format(cmd))
        proc = subprocess.Popen(
            # fmt: skip
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=self.env,
            **kwargs,
        )

        try:
            stdout, stderr = proc.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            proc.kill()
            raise

        if stdout:
            logger = logging.getLogger("stdout:")
            try:
                for line in iter(stdout.splitlines()):
                    logger.info(line.decode("utf-8"))
            except UnicodeError:
                logger.info("not UTF-8 ({} B)".format(len(stdout)))
            del logger

        if stderr:
            logger = logging.getLogger("stderr:")
            for line in iter(stderr.splitlines()):
                logger.error(line.decode("utf-8"))
            del logger

        if proc.returncode != 0:
            raise subprocess.CalledProcessError(
                # fmt: skip
                returncode=proc.returncode,
                output=stdout,
                stderr=stderr,
                cmd=cmd,
            )

        class ProcOutput:
            pass

        output = ProcOutput()
        output.stdout = stdout
        output.stderr = stderr
        return output

    def vvcompose(self, lua_script, hdf):
        if not os.path.isabs(lua_script):
            lua_script = os.path.join(self.cwd, lua_script)
        if not os.path.isabs(hdf):
            hdf = self.tmp(hdf)
        return self.run(
            # fmt: skip
            [self.which["vvflow"], lua_script, hdf],
            cwd=self.tempdir,
        )

    def vvflow(self, hdf, *args, **kwargs):
        if not os.path.isabs(hdf):
            hdf = self.tmp(hdf)
        return self.run(
            # fmt: skip
            [self.which["vvflow"], hdf, *args],
            cwd=self.tempdir,
            **kwargs,
        )

    def vvplot(self, ifile, ofile, args=[]):
        if not os.path.isabs(ifile):
            ifile = self.tmp(ifile)
        if not os.path.isabs(ofile):
            ofile = self.tmp(ofile)
        if type(args) is str:
            args = args.split(" ")

        output = self.run(
            # fmt: skip
            [self.which["vvplot"], ifile, ofile] + args,
            cwd=self.tempdir,
            timeout=30,
        )
        output.ifile = ifile
        output.ofile = ofile
        return output

    def tmp(self, fname):
        return os.path.join(self.tempdir, fname)


@pytest.fixture(scope="module")
def tempdir(request):
    print("")
    tempdir = getattr(request.module, "tempdir")
    if not tempdir:
        dir = py.path.local(tempfile.mkdtemp())
        logging.warning("Create tempdir: {}".format(str(dir)))
        request.addfinalizer(lambda: dir.remove(rec=1))
        return str(dir)
    else:
        logging.warning("Use clean tempdir: {}".format(tempdir))
        if os.path.exists(tempdir):
            shutil.rmtree(tempdir)
        os.makedirs(tempdir)
        return tempdir


@pytest.fixture(scope="module")
def env(request, tempdir):
    env = Env(
        cwd=str(request.fspath.dirname),
        tempdir=tempdir,
    )
    return env
    # dir = os.path.join(request.fspath.dirname, 'output')
    # return str(dir)
