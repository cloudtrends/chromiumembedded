# Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

import pickle
from optparse import OptionParser
import os.path
import sys
from file_util import *
from patch_util import *


# cannot be loaded as a module
if __name__ != "__main__":
    sys.stderr.write('This file cannot be loaded as a module!')
    sys.exit()
    

# parse command-line options
disc = """
This utility applies patch files.
"""

parser = OptionParser(description=disc)
parser.add_option('--patch-config', dest='patchconfig', metavar='DIR',
                  help='patch configuration file')
parser.add_option('--patch-dir', dest='patchdir', metavar='DIR',
                  help='patch source directory')
(options, args) = parser.parse_args()

# the patchconfig and patchdir options are required
if options.patchconfig is None or options.patchdir is None:
    parser.print_help(sys.stdout)
    sys.exit()

# normalize the patch directory value
patchdir = os.path.dirname(os.path.abspath(options.patchconfig)).replace('\\', '/')
if patchdir[-1] != '/':
    patchdir += '/'
patchsrcdir = patchdir + options.patchdir;
if patchsrcdir[-1] != '/':
    patchsrcdir += '/'

# check if the patching should be skipped
if os.path.isfile(patchdir + 'NOPATCH'):
    nopatch = True
    sys.stdout.write('NOPATCH exists -- files have not been patched.\n')
else:
    nopatch = False
    # locate the patch configuration file
    if not os.path.isfile(options.patchconfig):
        sys.stderr.write('File '+options.patchconfig+' does not exist.\n')
        sys.exit()
    
    scope = {}
    execfile(options.patchconfig, scope)
    patches = scope["patches"]
    
    for name in patches.keys():
        file = patchsrcdir+name+'.patch'
        if not os.path.isfile(file):
            sys.stderr.write('Patch file '+file+' does not exist.\n')
        else:
            sys.stderr.write('Reading patch file '+file+'\n')
            dir = patches[name]
            patchObj = from_file(file)
            patchObj.apply(dir)
