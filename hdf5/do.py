#!/usr/bin/python

import h5py
import numpy as np
from fractions import Fraction
from decimal import Decimal
import argparse

my_epilog="""SCRIPT
	SCRIPT is a bash comand that will be executed at each time step. It can use $t
	variable to retrieve current time and must print a single resulting value.
	Examples:
	'echo 1' - returns constant value.
	'awk "BEGIN{print sin($t)+2}"' - perform calculations with awk
	'python -c "import math; print math.sin($t)+2"' - calculations with python"""

parser = argparse.ArgumentParser(description='Compose binary vvhd file',
	epilog=my_epilog, formatter_class=argparse.RawDescriptionHelpFormatter);


parser.add_argument(
	'-i',
	metavar="FILE",
	type=argparse.FileType('r'),
	#dest='input',
	#action='store_true',
	help='input file'
)
parser.add_argument(
	'-o',
	metavar="FILE",
	#dest='output',
	#action='store_true',
	help='output file'
)

parser.add_argument(
	'-b',
	metavar="FILE",
	#dest='output',
	action='append',
	help='load body'
)

parser.add_argument(
	'--bvx', '--bvy', '--bvo',
	metavar="SCRIPT",
	#dest='output',
	action='store_true',
	help='set body velocity'
)

parser.add_argument(
	'-m',
	nargs=3,
	metavar=("N", "dx", "dy"),
	#dest='output',
	type=float,
	action='append',
	help='Move Nth body by (dy, dy)'
)

args = parser.parse_args()
print args
exit(0)
###############################################################################################
###############################################################################################
###############################################################################################
###############################################################################################
###############################################################################################

def tuple(frac):
	return (frac.numerator, frac.denominator)

time_d = np.dtype([('value', 'i4'), ('timescale', 'i4')])

file = h5py.File("mytestfile.h5", "w")
file.attrs['caption'] = "mycap"
file.attrs['caption'] = "mycap123"
file.attrs['re'] = 888.8;
file.attrs.create('time', tuple(Fraction('0.005')), dtype=time_d);
file.close()

file = h5py.File("mytestfile.h5", "r")
print file.attrs.keys()
print file.attrs.items()
print file.keys()
print file.values()
print file.items()
file.close()
