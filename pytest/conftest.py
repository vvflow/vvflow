#!/usr/bin/env python3

import os
import py
import pytest
import logging
import tempfile
import subprocess
logging.basicConfig(level=logging.INFO, format=' %(levelname)7s %(name)s > %(message)s')

class ProcOutput:
    def __init__(self, stdout, stderr):
        self.stdout = stdout
        self.stderr = stderr

class Env:
    def __init__(self, cwd, tempdir):
        self.cwd = cwd
        self.tempdir = tempdir

        self.env = os.environ.copy()
        path = [
            'utils/vvcompose',
            'utils/vvxtract',
            'utils/vvplot3',
            self.env['PATH']
        ]
        # self.env['PATH'] = ':'.join(path)
        logging.warn('export PATH="%s"' % self.env['PATH'])

    def run(self, cmd, **kwargs):
        timeout = 10
        if "timeout" in kwargs.keys():
            timeout = kwargs["timeout"]
            del kwargs["timeout"]

        logging.info("Running: '{}'".format(cmd))
        proc = subprocess.Popen(cmd,
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
                for l in iter(stdout.splitlines()):
                    logger.info(l.decode('utf-8'))
            except UnicodeError:
                logger.info("not UTF-8 ({} B)".format(len(stdout)))
            del logger

        if stderr:
            logger = logging.getLogger('stderr:')
            for l in iter(stderr.splitlines()):
                logger.error(l.decode('utf-8'))
            del logger

        if proc.returncode != 0:
            raise subprocess.CalledProcessError(returncode=proc.returncode, cmd=cmd)

        return ProcOutput(stdout=stdout, stderr=stderr)

    def vvcompose(self, lua_script, hdf):
        return self.run([
                'vvcompose',
                os.path.join(self.cwd, lua_script),
                self.tmp(hdf)
            ],
            cwd=self.tempdir
        )

    def vvflow(self, hdf):
        return self.run([
                'vvflow',
                self.tmp(hdf)
            ],
            cwd=self.tempdir
        )

    def vvplot3(self, hdf, ear, png, args):
        self.run([
                'vvplot3', '-n',
                self.tmp(hdf),
                self.tmp(ear)
            ] + args.split(' '),
            cwd=self.tempdir
        )
        self.run([
                self.tmp(ear) + ' > ' + self.tmp(png)
            ],
            cwd=self.tempdir,
            shell=True
        )

    def tmp(self, fname):
        return os.path.join(self.tempdir, fname)


@pytest.fixture(scope='module')
def tempdir(request):
    print('')
    tempdir = getattr(request.module, "tempdir")
    if not tempdir:
        dir = py.path.local(tempfile.mkdtemp())
        logging.warn("Create tempdir: {}".format(str(dir)))
        request.addfinalizer(lambda: dir.remove(rec=1))
        return str(dir)
    else:
        logging.warn("Use tempdir: {}".format(tempdir))
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


