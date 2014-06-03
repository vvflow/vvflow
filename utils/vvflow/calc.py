#!/usr/bin/python

from sys import argv
from math import *

allargs = "";
if len(argv) > 1:
	for i in argv[1:]:
		allargs+= i+" "

def line(line, x):
	line.sort()
	#find approximation segment
	try:
		p1 = next(p for p in reversed(line) if p[0] < x)
		p2 = next(p for p in line if p[0] >= x)
		return p1[1]+float(x-p1[0])*float(p2[1]-p1[1])/float(p2[0]-p1[0])
	except:
		return 0

try: print eval(allargs);
except SyntaxError: print 0; exit();
except NameError: print 0; exit();
