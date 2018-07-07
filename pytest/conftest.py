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
        path = [
            'utils/vvcompose',
            'utils/vvxtract',
            'utils/vvplot3',
            self.env['PATH']
        ]
        # self.env['PATH'] = ':'.join(path)
        logging.warn('export PATH="%s"' % self.env['PATH'])

    def run(self, cmd, **kwargs):

        logging.info('Running: ' + ' '.join(cmd))
        proc = subprocess.run(cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=self.env,
            **kwargs
            )

        if proc.stdout:
            logger = logging.getLogger('stdout:')
            try:
                for l in iter(proc.stdout.splitlines()):
                    logger.info(l.decode('utf-8'))
            except UnicodeError:
                logger.info("not UTF-8 ({} B)".format(len(proc.stdout)))
            del logger

        if proc.stderr:
            logger = logging.getLogger('stderr:')
            for l in iter(proc.stderr.splitlines()):
                logger.error(l.decode('utf-8'))
            del logger

        proc.check_returncode()
        return proc

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


