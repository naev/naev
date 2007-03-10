#!/usr/bin/env python

import math;

trig_table = []

cte = 150.0

func = math.sin
afunc = math.asin

for i in range(0, int(round(math.pi*cte/2))):
	trig_table.append( func(i/cte) )


print "ACCURACY OF "
print afunc(trig_table[1])/math.pi*180.0
print "-------------------------------------------"
print trig_table
print "-------------------------------------------"
print "ACCURACY OF "
print afunc(trig_table[1])/math.pi*180.0
