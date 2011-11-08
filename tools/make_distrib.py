# Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

from date_util import *
from file_util import *
from gclient_util import *
from optparse import OptionParser
import os
import re
from svn_util import *
import sys
import zipfile

def create_archive(input_dir, zip_file):
  """ Creates a zip archive of the specified input directory. """
  zf = zipfile.ZipFile(zip_file, 'w', zipfile.ZIP_DEFLATED)
  def addDir(dir):
    for f in os.listdir(dir):
      full_path = os.path.join(dir, f)
      if os.path.isdir(full_path):
        addDir(full_path)
      else:
        zf.write(full_path, os.path.relpath(full_path, \
                 os.path.join(input_dir, os.pardir)))
  addDir(input_dir)
  zf.close()

def create_readme(src, output_dir, cef_rev, chromium_rev, date):
  """ Creates the README.TXT file. """
  data = read_file(src)
  data = data.replace('$CEF_REV$', cef_rev)
  data = data.replace('$CHROMIUM_REV$', chromium_rev)
  data = data.replace('$DATE$', date)
  write_file(os.path.join(output_dir, 'README.TXT'), data)
  if not options.quiet:
    sys.stdout.write('Creating README.TXT file.\n')

def eval_file(src):
  """ Loads and evaluates the contents of the specified file. """
  return eval(read_file(src), {'__builtins__': None}, None)
    
def transfer_gypi_files(src_dir, gypi_paths, gypi_path_prefix, dst_dir, quiet):
  """ Transfer files from one location to another. """
  for path in gypi_paths:
    src = os.path.join(src_dir, path)
    dst = os.path.join(dst_dir, path.replace(gypi_path_prefix, ''))
    dst_path = os.path.dirname(dst)
    make_dir(dst_path, quiet)
    copy_file(src, dst, quiet)

def normalize_headers(file, new_path = ''):
  """ Normalize headers post-processing. Remove the path component from any
      project include directives. """
  data = read_file(file)
  data = re.sub(r'''#include \"[a-zA-Z0-9_\/]+\/+([a-zA-Z0-9_\.]+)\"''', \
                "// Include path modified for CEF Binary Distribution.\n#include \""+new_path+"\\1\"", data)
  write_file(file, data)

def transfer_files(cef_dir, script_dir, transfer_cfg, output_dir, quiet):
  """ Transfer files based on the specified configuration. """
  if not path_exists(transfer_cfg):
    return
  
  configs = eval_file(transfer_cfg)
  for cfg in configs:
    dst = os.path.join(output_dir, cfg['target'])
    
    # perform a copy if source is specified
    if not cfg['source'] is None:
      src = os.path.join(cef_dir, cfg['source'])
      dst_path = os.path.dirname(dst)
      make_dir(dst_path, quiet)
      copy_file(src, dst, quiet)
      
      # place a readme file in the destination directory
      readme = os.path.join(dst_path, 'README-TRANSFER.txt')
      if not path_exists(readme):
        copy_file(os.path.join(script_dir, 'distrib/README-TRANSFER.txt'), readme)
      open(readme, 'a').write(cfg['source']+"\n")
    
    # perform any required post-processing
    if 'post-process' in cfg:
      post = cfg['post-process']
      if post == 'normalize_headers':
        new_path = ''
        if cfg.has_key('new_header_path'):
          new_path = cfg['new_header_path']
        normalize_headers(dst, new_path)

def generate_msvs_projects(version):
  """ Generate MSVS projects for the specified version. """
  sys.stdout.write('Generating '+version+' project files...')
  os.environ['GYP_MSVS_VERSION'] = version
  gyper = [ 'python', 'tools/gyp_cef', os.path.relpath(os.path.join(output_dir, 'cefclient.gyp'), cef_dir) ]
  RunAction(cef_dir, gyper);
  move_file(os.path.relpath(os.path.join(output_dir, 'cefclient.sln')), \
            os.path.relpath(os.path.join(output_dir, 'cefclient'+version+'.sln')))

def fix_msvs_projects():
  """ Fix the output directory path in all .vcproj and .vcxproj files. """
  files = []
  for file in get_files(os.path.join(output_dir, '*.vcproj')):
    files.append(file)
  for file in get_files(os.path.join(output_dir, '*.vcxproj')):
    files.append(file)
  for file in files:
    data = read_file(file)
    data = data.replace('../../..\\build\\', '')
    write_file(file, data)

# cannot be loaded as a module
if __name__ != "__main__":
  sys.stderr.write('This file cannot be loaded as a module!')
  sys.exit()

# parse command-line options
disc = """
This utility builds the CEF Binary Distribution.
"""

parser = OptionParser(description=disc)
parser.add_option('--output-dir', dest='outputdir', metavar='DIR',
                  help='output directory [required]')
parser.add_option('--allow-partial',
                  action='store_true', dest='allowpartial', default=False,
                  help='allow creation of partial distributions')
parser.add_option('-q', '--quiet',
                  action='store_true', dest='quiet', default=False,
                  help='do not output detailed status information')
(options, args) = parser.parse_args()

# the outputdir option is required
if options.outputdir is None:
  parser.print_help(sys.stdout)
  sys.exit()

# retrieve revision and date information
cef_rev = get_revision()
chromium_rev = get_revision('../../')
date = get_date()

# script directory
script_dir = os.path.dirname(__file__)

# CEF root directory
cef_dir = os.path.abspath(os.path.join(script_dir, os.pardir))

# Test the operating system.
platform = '';
if sys.platform == 'win32':
  platform = 'windows'
elif sys.platform == 'darwin':
  platform = 'macosx'
elif sys.platform.startswith('linux'):
  platform = 'linux'

# output directory
output_dir = os.path.abspath(os.path.join(options.outputdir, \
                                          'cef_binary_r'+cef_rev+'_'+platform))
remove_dir(output_dir, options.quiet)
make_dir(output_dir, options.quiet)

# transfer the LICENSE.txt file
copy_file(os.path.join(cef_dir, 'LICENSE.txt'), output_dir, options.quiet)

# read the variables list from cef_paths.gypi
cef_paths = eval_file(os.path.join(cef_dir, 'cef_paths.gypi'))
cef_paths = cef_paths['variables']

# create the include directory
include_dir = os.path.join(output_dir, 'include')
make_dir(include_dir, options.quiet)

# create the cefclient directory
cefclient_dir = os.path.join(output_dir, 'cefclient')
make_dir(cefclient_dir, options.quiet)

# create the libcef_dll_wrapper directory
wrapper_dir = os.path.join(output_dir, 'libcef_dll')
make_dir(wrapper_dir, options.quiet)

# transfer common include files
transfer_gypi_files(cef_dir, cef_paths['includes_common'], \
                    'include/', include_dir, options.quiet)

# transfer common cefclient files
transfer_gypi_files(cef_dir, cef_paths['cefclient_sources_common'], \
                    'tests/cefclient/', cefclient_dir, options.quiet)

# transfer common libcef_dll_wrapper files
transfer_gypi_files(cef_dir, cef_paths['libcef_dll_wrapper_sources_common'], \
                    'libcef_dll/', wrapper_dir, options.quiet)

# transfer gyp files
copy_file(os.path.join(script_dir, 'distrib/cefclient.gyp'), output_dir, options.quiet)
paths_gypi = os.path.join(cef_dir, 'cef_paths.gypi')
data = read_file(paths_gypi)
data = data.replace('tests/cefclient/', 'cefclient/')
write_file(os.path.join(output_dir, 'cef_paths.gypi'), data)

# transfer additional files
transfer_files(cef_dir, script_dir, os.path.join(script_dir, 'distrib/transfer.cfg'), \
               output_dir, options.quiet)

if platform == 'windows':
  # create the README.TXT file
  create_readme(os.path.join(script_dir, 'distrib/win/README.TXT'), output_dir, cef_rev, \
                chromium_rev, date)

  # transfer include files
  transfer_gypi_files(cef_dir, cef_paths['includes_win'], \
                      'include/', include_dir, options.quiet)

  # transfer cefclient files
  transfer_gypi_files(cef_dir, cef_paths['cefclient_sources_win'], \
                      'tests/cefclient/', cefclient_dir, options.quiet)

  # transfer build/Debug files
  if not options.allowpartial or path_exists(os.path.join(cef_dir, 'Debug')):
    dst_dir = os.path.join(output_dir, 'Debug')
    make_dir(dst_dir, options.quiet)
    copy_files(os.path.join(script_dir, 'distrib/win/*.dll'), dst_dir, options.quiet)
    copy_files(os.path.join(cef_dir, 'Debug/*.dll'), dst_dir, options.quiet)
    copy_file(os.path.join(cef_dir, 'Debug/cefclient.exe'), dst_dir, options.quiet)
    copy_file(os.path.join(cef_dir, 'Debug/chrome.pak'), dst_dir, options.quiet)
    copy_dir(os.path.join(cef_dir, 'Debug/locales'), os.path.join(dst_dir, 'locales'), \
             options.quiet)
  
    # transfer lib/Debug files
    dst_dir = os.path.join(output_dir, 'lib/Debug')
    make_dir(dst_dir, options.quiet)
    copy_file(os.path.join(cef_dir, 'Debug/lib/libcef.lib'), dst_dir, options.quiet)
  else:
    sys.stderr.write("No Debug build files.\n")

  # transfer build/Release files
  if not options.allowpartial or path_exists(os.path.join(cef_dir, 'Release')):
    dst_dir = os.path.join(output_dir, 'Release')
    make_dir(dst_dir, options.quiet)
    copy_files(os.path.join(script_dir, 'distrib/win/*.dll'), dst_dir, options.quiet)
    copy_files(os.path.join(cef_dir, 'Release/*.dll'), dst_dir, options.quiet)
    copy_file(os.path.join(cef_dir, 'Release/cefclient.exe'), dst_dir, options.quiet)
    copy_file(os.path.join(cef_dir, 'Release/chrome.pak'), dst_dir, options.quiet)
    copy_dir(os.path.join(cef_dir, 'Release/locales'), os.path.join(dst_dir, 'locales'), \
             options.quiet)

    # transfer lib/Release files
    dst_dir = os.path.join(output_dir, 'lib/Release')
    make_dir(dst_dir, options.quiet)
    copy_file(os.path.join(cef_dir, 'Release/lib/libcef.lib'), dst_dir, options.quiet)
  else:
    sys.stderr.write("No Release build files.\n")

  # generate doc files
  os.popen('make_cppdocs.bat '+cef_rev)

  # transfer docs files
  dst_dir = os.path.join(output_dir, 'docs')
  src_dir = os.path.join(cef_dir, 'docs')
  if path_exists(src_dir):
    copy_dir(src_dir, dst_dir, options.quiet)

  # transfer additional files, if any
  transfer_files(cef_dir, script_dir, os.path.join(script_dir, 'distrib/win/transfer.cfg'), \
                 output_dir, options.quiet)

  # generate the project files
  generate_msvs_projects('2005');
  generate_msvs_projects('2008');
  generate_msvs_projects('2010');
  fix_msvs_projects();

elif platform == 'macosx':
  # create the README.TXT file
  create_readme(os.path.join(script_dir, 'distrib/mac/README.TXT'), output_dir, cef_rev, \
                chromium_rev, date)
  
  # transfer include files
  transfer_gypi_files(cef_dir, cef_paths['includes_mac'], \
                      'include/', include_dir, options.quiet)

  # transfer cefclient files
  transfer_gypi_files(cef_dir, cef_paths['cefclient_sources_mac'], \
                      'tests/cefclient/', cefclient_dir, options.quiet)

  # transfer cefclient/mac files
  copy_dir(os.path.join(cef_dir, 'tests/cefclient/mac/'), os.path.join(output_dir, 'cefclient/mac/'), \
           options.quiet)
  
  # transfer xcodebuild/Debug files
  if not options.allowpartial or path_exists(os.path.join(cef_dir, '../xcodebuild/Debug')):
    dst_dir = os.path.join(output_dir, 'Debug')
    make_dir(dst_dir, options.quiet)
    copy_file(os.path.join(cef_dir, '../xcodebuild/Debug/ffmpegsumo.so'), dst_dir, options.quiet)
    copy_file(os.path.join(cef_dir, '../xcodebuild/Debug/libcef.dylib'), dst_dir, options.quiet)
  
  # transfer xcodebuild/Release files
  if not options.allowpartial or path_exists(os.path.join(cef_dir, '../xcodebuild/Release')):
    dst_dir = os.path.join(output_dir, 'Release')
    make_dir(dst_dir, options.quiet)
    copy_file(os.path.join(cef_dir, '../xcodebuild/Release/ffmpegsumo.so'), dst_dir, options.quiet)
    copy_file(os.path.join(cef_dir, '../xcodebuild/Release/libcef.dylib'), dst_dir, options.quiet)
  
  # transfer resource files
  dst_dir = os.path.join(output_dir, 'Resources')
  make_dir(dst_dir, options.quiet)
  copy_files(os.path.join(cef_dir, '../third_party/WebKit/Source/WebCore/Resources/*.*'), dst_dir, options.quiet)
  copy_file(os.path.join(cef_dir, '../xcodebuild/Release/cefclient.app/Contents/Resources/chrome.pak'), dst_dir, options.quiet)
  copy_files(os.path.join(cef_dir, '../xcodebuild/Release/cefclient.app/Contents/Resources/*.lproj'), dst_dir, options.quiet)
  remove_dir(os.path.join(dst_dir, 'English.lproj'))
  
  # transfer additional files, if any
  transfer_files(cef_dir, script_dir, os.path.join(script_dir, 'distrib/mac/transfer.cfg'), \
                output_dir, options.quiet)

  # Generate Xcode project files
  sys.stdout.write('Generating Xcode project files...')
  gyper = [ 'python', 'tools/gyp_cef', os.path.relpath(os.path.join(output_dir, 'cefclient.gyp'), cef_dir) ]
  RunAction(cef_dir, gyper);

  # Post-process the Xcode project to fix file paths
  src_file = os.path.join(output_dir, 'cefclient.xcodeproj/project.pbxproj')
  data = read_file(src_file)
  data = data.replace('../../../build/mac/', 'tools/')
  data = data.replace('../../../', '')
  write_file(src_file, data)

elif platform == 'linux':
  linux_build_dir = os.path.join(cef_dir, os.pardir, 'out')
  # transfer build/Debug files
  if not options.allowpartial or path_exists(os.path.join(linux_build_dir, 'Debug')):
    dst_dir = os.path.join(output_dir, 'Debug')
    make_dir(dst_dir, options.quiet)
    copy_files(os.path.join(linux_build_dir, 'Debug/lib.target/*'), dst_dir, options.quiet)
    copy_file(os.path.join(linux_build_dir, 'Debug/cefclient'), dst_dir, options.quiet)
    copy_file(os.path.join(linux_build_dir, 'Debug/chrome.pak'), dst_dir, options.quiet)
    copy_dir(os.path.join(linux_build_dir, 'Debug/locales'), os.path.join(dst_dir, 'locales'), options.quiet)

  else:
    sys.stderr.write("No Debug build files.\n")

  # transfer build/Release files
  if not options.allowpartial or path_exists(os.path.join(linux_build_dir, 'Release')):
    dst_dir = os.path.join(output_dir, 'Release')
    make_dir(dst_dir, options.quiet)
    copy_files(os.path.join(linux_build_dir, 'Release/lib.target/*'), dst_dir, options.quiet)
    copy_file(os.path.join(linux_build_dir, 'Release/cefclient'), dst_dir, options.quiet)
    copy_file(os.path.join(linux_build_dir, 'Release/chrome.pak'), dst_dir, options.quiet)
    copy_dir(os.path.join(linux_build_dir, 'Release/locales'), os.path.join(dst_dir, 'locales'), options.quiet)

  else:
    sys.stderr.write("No Release build files.\n")

   # transfer include files
  transfer_gypi_files(cef_dir, cef_paths['includes_linux'], \
                      'include/', include_dir, options.quiet)

  # transfer cefclient files
  transfer_gypi_files(cef_dir, cef_paths['cefclient_sources_linux'], \
                      'tests/cefclient/', cefclient_dir, options.quiet)

  # transfer additional files, if any
  transfer_files(cef_dir, script_dir, os.path.join(script_dir, 'distrib/linux/transfer.cfg'), \
                output_dir, options.quiet)

# Create an archive of the output directory
zip_file = os.path.split(output_dir)[1] + '.zip'
if not options.quiet:
  sys.stdout.write('Creating '+zip_file+"...\n")
create_archive(output_dir, os.path.join(output_dir, os.pardir, zip_file))
