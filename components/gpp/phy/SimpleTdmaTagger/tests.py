# -*- coding: utf-8 -*-
"""
Created on Wed Feb  5 16:31:41 2014

@author: anpu
"""

import math

val = float(756132)

superslot = float(200000)


ceil = math.ceil(val / superslot) * superslot
print "ceil: %d" % ceil

diff = ceil - val
print "diff: %d" % diff


out = val + diff
print "out: %d" % out

