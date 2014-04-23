# Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

from optparse import OptionParser
import os
import re
import sys
from exec_util import exec_cmd
import svn_util as svn
import git_util as git

def msg(message):
  """ Output a message. """
  sys.stdout.write('--> ' + message + "\n")

def warn(message):
  """ Output a warning. """
  sys.stdout.write('-' * 80 + "\n")
  sys.stdout.write('!!!! WARNING: ' + message + "\n")
  sys.stdout.write('-' * 80 + "\n")

def extract_paths(file):
  """ Extract the list of modified paths from the patch file. """
  paths = []
  fp = open(file)
  for line in fp:
    if line[:4] != '+++ ':
      continue
    match = re.match('^([^\t]+)', line[4:])
    if not match:
      continue
    paths.append(match.group(1).strip())
  return paths

# Cannot be loaded as a module.
if __name__ != "__main__":
  sys.stderr.write('This file cannot be loaded as a module!')
  sys.exit()

# Parse command-line options.
disc = """
This utility updates existing patch files.
"""

parser = OptionParser(description=disc)
parser.add_option('--resave',
                  action='store_true', dest='resave', default=False,
                  help='re-save existing patch files to pick up manual changes')
(options, args) = parser.parse_args()

# The CEF root directory is the parent directory of _this_ script.
cef_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir))
src_dir = os.path.join(cef_dir, os.pardir)

# Determine the type of Chromium checkout.
if svn.is_checkout(src_dir):
  src_is_git = False
elif git.is_checkout(src_dir):
  src_is_git = True
else:
  raise Exception('Not a valid checkout: %s' % src_dir)

patch_dir = os.path.join(cef_dir, 'patch')
patch_cfg = os.path.join(patch_dir, 'patch.cfg')
if not os.path.isfile(patch_cfg):
  raise Exception('File does not exist: %s' % patch_cfg)

# Read the patch configuration file.
msg('Reading patch config %s' % patch_cfg)
scope = {}
execfile(patch_cfg, scope)
patches = scope["patches"]

# Read each individual patch file.
patches_dir = os.path.join(patch_dir, 'patches')
for patch in patches:
  sys.stdout.write('\n')
  patch_file = os.path.join(patches_dir, patch['name']+'.patch')

  if os.path.isfile(patch_file):
    msg('Reading patch file %s' % patch_file)
    patch_root = patch['path']
    patch_root_abs = os.path.abspath(os.path.join(cef_dir, patch_root))

    # Retrieve the list of paths modified by the patch file.
    patch_paths = extract_paths(patch_file)

    if not options.resave:
      # Revert any changes to existing files in the patch.
      for patch_path in patch_paths:
        patch_path_abs = os.path.abspath(os.path.join(patch_root_abs, \
                                                      patch_path))
        msg('Reverting changes to %s' % patch_path_abs)
        if src_is_git:
          cmd = 'git checkout -- %s' % (patch_path_abs)
        else:
          cmd = 'svn revert %s' % (patch_path_abs)
        result = exec_cmd(cmd, patch_root_abs)
        if result['err'] != '':
          raise Exception('Failed to revert file: %s' % result['err'])
        if result['out'] != '':
          sys.stdout.write(result['out'])

      # Apply the patch file.
      msg('Applying patch to %s' % patch_root_abs)
      result = exec_cmd('patch -p0 < %s' % patch_file, patch_root_abs)
      if result['err'] != '':
        raise Exception('Failed to apply patch file: %s' % result['err'])
      sys.stdout.write(result['out'])
      if result['out'].find('FAILED') != -1:
        warn('Failed to apply %s, fix manually and run with --resave' % \
             patch['name'])
        continue

    # Re-create the patch file.
    msg('Saving changes to %s' % patch_file)
    patch_paths_str = ' '.join(patch_paths)
    if src_is_git:
      cmd = 'git diff --no-prefix --relative %s > %s' % \
            (patch_paths_str, patch_file)
    else:
      cmd = 'svn diff %s > %s' % (patch_paths_str, patch_file)
    result = exec_cmd(cmd, patch_root_abs)
    if result['err'] != '' and result['err'].find('warning:') != 0:
      raise Exception('Failed to create patch file: %s' % result['err'])
  else:
    raise Exception('Patch file does not exist: %s' % patch_file)
