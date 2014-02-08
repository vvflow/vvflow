#!/usr/bin/python

import argparse

my_epilog="""Additional environmental variables:
VV_PREC_HI - horiz resolution of temperature and pressure fields (default 500; affects -opPt)
VV_PREC_LO - horiz resolution of streamlines (default 200; affects -s)
VV_ISOPSI - streamlines levels (default $(seq -s \' \' -10 0.1 10); affects -s)
VV_ISOTHERMS - temperature isolines (default $(seq -s \' \' 0.05 0.05 0.95); affects -o)
VV_VORT_RANGE - change contrast of vorticity field (default 50; affects -g)
VV_EPS_MULT - smooth temperature field (default 2; affects -gt)
VV_BODY_TEMP - body surface temperature (default 1; affects -t)"""

parser = argparse.ArgumentParser(description='Plot binary vvhd file',
	epilog=my_epilog, formatter_class=argparse.RawDescriptionHelpFormatter);

parser.add_argument(
	'input_file',
	type=argparse.FileType('r'),
	#dest='input',
	#action='store_true',
	help='input file'
)
parser.add_argument(
	'output_file',
	#dest='output',
	#action='store_true',
	help='output file or directory'
)

################################################################################

parser.add_argument(
	'-b',
	dest='plot_body',
	action='store_true',
	help='plot body'
)
parser.add_argument(
	'-g',
	dest='plot_vorticity',
	action='store_true',
	help='plot vorticity'
)
parser.add_argument(
	'-H',
	dest='plot_heat',
	action='store_true',
	help='plot heat particles'
)
parser.add_argument(
	'-i',
	dest='plot_ink',
	action='store_true',
	help='plot ink (streaklines)'
)
parser.add_argument(
	'-O',
	dest='plot_isotherms',
	action='store_true',
	help='plot isotherms'
)
parser.add_argument(
	'-p',
	dest='plot_pressure',
	action='store_true',
	help='plot pressure field'
)
parser.add_argument(
	'-P',
	dest='plot_isopressure',
	action='store_true',
	help='plot pressure isolines'
)
parser.add_argument(
	'-s',
	dest='plot_streamlines',
	action='store_true',
	help='plot streamlines'
)
parser.add_argument(
	'-t',
	dest='plot_temperature',
	action='store_true',
	help='plot temperature field'
)
parser.add_argument(
	'-v',
	dest='plot_vortexes',
	action='store_true',
	help='plot vortex domains with dots'
)
parser.add_argument(
	'-V',
	dest='plot_vortexes_bold',
	action='store_true',
	help='plot vortex domains in bold'
)

################################################################################

parser.add_argument(
	'-w', '--grayscale',
	dest='mode_bw',
	action='store_true',
	help='plot in grayscale'
)
parser.add_argument(
	'-f', '--force',
	dest='mode_force',
	action='store_true',
	help='update fields files even if they exist'
)
parser.add_argument(
	'-n', '--dry',
	dest='mode_dry',
	action='store_true',
	help='plot fields only, dont produce output picture'
)

parser.add_argument(
	'--nooverride',
	dest='mode_nooverride',
	action='store_true',
	help='do not override output if it already exists'
)

parser.add_argument(
	'--colorbox',
	dest='mode_colorbox',
	action='store_true',
	help='draw colorbox on bottom of plot'
)

parser.add_argument(
	'--timelabel',
	dest='mode_timelabel',
	action='store_true',
	help='draw time label in top left corner'
)

parser.add_argument(
	'--blankbody',
	metavar="N",
	type = int,
	default = 0,
	help='do not fill body (numeration starts with 1)'
)

################################################################################
def axis_range(string):
	if len(string) == 0:
		return float('NaN')
	try:
		result = float(string)
	except ValueError:
		msg = "invalid float value: %r" % string
		raise argparse.ArgumentTypeError(msg)
	return result

parser.add_argument(
	'-x',
	dest='size_x',
	nargs=2,
	type=float,
	metavar=('XMIN', 'XMAX'),
	required=True,
	help='X axis constraints'
)
parser.add_argument(
	'-y',
	dest='size_y',
	nargs=2,
	type=axis_range,
	default=(float('NaN'),float('NaN')),
	metavar=('YMIN', 'YMAX'),
	help='Y axis constraints, either YMIN or YMAX may be an empty string'
)

################################################################################
def picture_size(string):
	try:
		w_str,h_str = string.split('x')
		if (w_str == ''): w_str='0'
		elif (h_str == ''): h_str='0'
		result = (int(w_str), int(h_str))
		if result[0] < 0 or result[1] <0:
			raise ValueError;
	except ValueError:
		msg = "invalid size: %r" % string
		raise argparse.ArgumentTypeError(msg)
	return result

parser.add_argument(
	'--size',
	dest='size_pic',
	metavar='WxH',
	default='1280x720',
	type=picture_size,
	help='output figure size, either W or H can be ommited'
)

################################################################################
## REF FRAMES OPTIONS
parser.add_argument(
#	'-F',
	'--referenceframe',
#	metavar='REFFRAME',
#	dest='referenceframe'
	choices='obf',
	default='o',
	help='reference frame (original/body/fluid), default:  \'%(default)s\''
)

parser.add_argument(
	'--streamlines',
#	metavar='REFFRAME',
#	dest='streamlines'
	choices='obf',
	default='o',
	help="""choose streamlines reference frame (original/body/fluid),
	      default:  \'%(default)s\'"""
)

parser.add_argument(
	'--pressure',
#	metavar='REFFRAME',
#	dest='pressure'
	choices='sobf',
	default='s',
	help="""choose pressure mode (static pressure, original/body/fluid refframe),
	        default:  \'%(default)s\'"""
)

################################################################################
parser.add_argument(
	'--tree',
	metavar='FILE',
	dest='tree',
	help='draw tree nodes from file'
)

args = parser.parse_args()
