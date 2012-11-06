#!/usr/bin/python

from sys import argv
from math import sin, cos, pi

allargs = "";
if len(argv) > 1:
	for i in argv[1:]:
		allargs+= i+" "

try: print eval(allargs);
except SyntaxError: print 0; exit();
except NameError: print 0; exit();
