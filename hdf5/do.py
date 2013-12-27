#!/usr/bin/python

import h5py
import numpy as np
from fractions import Fraction
from decimal import Decimal
import argparse

my_epilog="""SCRIPT
	SCRIPT is a bash command that will be executed at each time step. It can use $t
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
	#type=h5py.File,
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

group_header = parser.add_argument_group('general parameters')
group_header.add_argument('-t', '--time', metavar="TIME", type=Fraction, help='set current time')
group_header.add_argument('-f', '--finish', metavar="TIME", type=Fraction, help='set time to stop calculation')
group_header.add_argument('--caption', help='set file caption')
group_header.add_argument('--dt-save', metavar="DT", type=Fraction, help='set binary saving delta')
group_header.add_argument('--dt-streak', metavar="DT", type=Fraction, help='set streak shed delta')
group_header.add_argument('--dt-profile', metavar="DT", type=Fraction, help='set velocity sensors measurement delta')
group_header.add_argument('--re', metavar="RE", type=float, help='set 1/viscosity value')
group_header.add_argument('--pr', metavar="PR", type=float, help='set Prandtl number')
group_header.add_argument('--ix', metavar="SCRIPT", help='set script for infinity X speed')
group_header.add_argument('--iy', metavar="SCRIPT", help='set script for infinity Y speed')
group_header.add_argument('--ig', metavar='GAMMA', type=float, help='set constant circulation at infinity')
group_header.add_argument(
	'--gravity',
	nargs=2,
	metavar=("GX", "GY"),
	type=float,
	help='set gravity vector (gx, gy)'
)

group_lists = parser.add_argument_group('loading lists from plain text')
group_lists.add_argument('--vort', metavar='FILE', action='append', help='load vortex domains')
group_lists.add_argument('--heat', metavar='FILE', action='append', help='load heat domains')
group_lists.add_argument('--body', '-b', metavar='FILE', action='append', help='load body')
group_lists.add_argument('--ink', '--streak', metavar='FILE', action='append', help='load streak particles')
group_lists.add_argument('--ink-source', '--streak-source', metavar='FILE', action='append', help='load streak sources')


# group1 = parser.add_argument_group('working with bodies')

# for arg in ['--bvx', '--bvy', '--bvo']:
# 	group1.add_argument(
# 		arg,
# 		metavar="SCRIPT",
# 		required=False,
# 		#dest='output',
# 		help='set body velocity'
# 	)

# for arg in ['-mx', '-my']:
# 	group1.add_argument(
# 		arg,
# 		#nargs=1,
# 		#metavar="a", #("N", "dx", "dy"),
# 		dest='move',
# 		#type=float,
# 		action='append_const', const=arg,
# 		help='Move Nth body by (dx, dy)'
# 	)

args = parser.parse_args()

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
