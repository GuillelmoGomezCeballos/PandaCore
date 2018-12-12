import sys 
from os import environ, getenv 
from argparse import ArgumentParser
from array import array 
import subprocess as sp 

parser = ArgumentParser()

# just a dict, but with + implemented
class ArgOpt(dict):
    def __init__(self, *args, **kwargs):
        super(ArgOpt, self).__init__()
        for a in args:
            if isinstance(a, dict):
                self.update(a)
        self.update(kwargs)
    def __add__(self, other):
        return ArgOpt(self, other)
    def __iadd__(self, other):
        self.update(other)
        return self 

STORE_TRUE = ArgOpt({'action':'store_true'})
STORE_FALSE = ArgOpt({'action':'store_false'})
MANY = ArgOpt({'nargs':'+'})

def parse(*args):
    for a in args:
        if type(a) == tuple:
            parser.add_argument(a[0], **a[1])
        else:
            parser.add_argument(a)
    return parser.parse_args()

def do(cmd, shell=False):
    return sp.call(cmd.split(), shell=shell)


from PandaCore.Utils.load import * 
from PandaCore.Tools.Misc import * 
from PandaCore.Utils.logging import * 

_env_vars = ['PANDA_FLATDIR', 'SUBMIT_OUTDIR', 'SUBMIT_LOGDIR',
             'SUBMIT_WORKDIR', 'SUBMIT_NAME', 'SUBMIT_USER',
             'CMSSW_BASE', 'SUBMIT_REPORT']
def validate_env(v=_env_vars):
    missing = [vv for vv in v if vv not in environ]
    if len(missing) > 0:
        logger.error('validate_env', 'missing: %s'%(repr(missing)))
        raise RuntimeError('')

flatdir = getenv('PANDA_FLATDIR')
cmssw_base = getenv('CMSSW_BASE')
