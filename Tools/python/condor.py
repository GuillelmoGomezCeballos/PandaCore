#!/usr/bin/env python

files = {}

try:
    import htcondor
    import classad
except ImportError:
    import imp
    local_path = ['/usr/lib64/python2.7/site-packages/']
    def _get_module(name):
        global files
        found = imp.find_module(name,local_path)
        files[name] = found[0]
        return imp.load_module(name,*found)
    classad = _get_module('classad') 
    htcondor = _get_module('htcondor') 

def _close_files():
    global files
    for k,v in files.iteritems():
        v.close()

import atexit
atexit.register(_close_files)
