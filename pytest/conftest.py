#!/usr/bin/env python3

import os
import py
import pytest
import logging
import tempfile
import subprocess
logging.basicConfig(level=logging.INFO, format=' %(levelname)7s %(name)s > %(message)s')


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
        logging.warning('export PATH="%s"' % self.env['PATH'])

    def run(self, cmd, **kwargs):
        timeout = 10
        if "timeout" in kwargs.keys():
            timeout = kwargs["timeout"]
            del kwargs["timeout"]

        logging.info("Running: '{}'".format(cmd))
        proc = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=self.env,
            **kwargs
        )

        try:
            stdout, stderr = proc.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            proc.kill()
            raise

        if stdout:
            logger = logging.getLogger('stdout:')
            try:
                for line in iter(stdout.splitlines()):
                    logger.info(line.decode('utf-8'))
            except UnicodeError:
                logger.info("not UTF-8 ({} B)".format(len(stdout)))
            del logger

        if stderr:
            logger = logging.getLogger('stderr:')
            for line in iter(stderr.splitlines()):
                logger.error(line.decode('utf-8'))
            del logger

        if proc.returncode != 0:
            raise subprocess.CalledProcessError(
                returncode=proc.returncode,
                output=stdout,
                stderr=stderr,
                cmd=cmd
            )

        class ProcOutput():
            pass
        output = ProcOutput()
        output.stdout = stdout
        output.stderr = stderr
        return output

    def vvcompose(self, lua_script, hdf):
        return self.run([
                'vvcompose',
                os.path.join(self.cwd, lua_script),
                self.tmp(hdf)
            ],
            cwd=self.tempdir
        )

    def vvflow(self, hdf, **kwargs):
        return self.run([
                'vvflow',
                self.tmp(hdf)
            ],
            cwd=self.tempdir,
            **kwargs
        )

    def vvplot(self, ifile, ofile, args=[]):
        ifile = self.tmp(ifile)
        ofile = self.tmp(ofile)
        if type(args) is str:
            args = args.split(' ')

        output = self.run(
            ['vvplot', ifile, ofile] + args,
            cwd=self.tempdir,
            timeout=30
        )
        output.ifile = ifile
        output.ofile = ofile
        return output

    def tmp(self, fname):
        return os.path.join(self.tempdir, fname)


@pytest.fixture(scope='module')
def tempdir(request):
    print('')
    tempdir = getattr(request.module, "tempdir")
    if not tempdir:
        dir = py.path.local(tempfile.mkdtemp())
        logging.warning("Create tempdir: {}".format(str(dir)))
        request.addfinalizer(lambda: dir.remove(rec=1))
        return str(dir)
    else:
        logging.warning("Use tempdir: {}".format(tempdir))
        if not os.path.exists(tempdir):
            os.makedirs(tempdir)
        return tempdir


@pytest.fixture(scope='module')
def env(request, tempdir):
    env = Env(
        cwd=str(request.fspath.dirname),
        tempdir=tempdir,
    )
    return env
    # dir = os.path.join(request.fspath.dirname, 'output')
    # return str(dir)
