#!/usr/bin/env python
'''@package docstring
Loads some numerical functions defined in a C++ file
'''

from PandaCore.Utils.load import Load 
from PandaCore.Utils.root import root 

Load('Functions')

# need to instantiate these things
root.bound
root.dsign
root.Mxx
root.MT
root.SignedDeltaPhi
root.DeltaR2
root.ExpErf
