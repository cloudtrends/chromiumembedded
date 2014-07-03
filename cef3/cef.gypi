# Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

{
  'variables': {
    # Don't use the chrome style plugin with CEF.
    'clang_use_chrome_plugins': 0,
    'conditions': [
      # Directory for CEF source files.
      [ 'OS=="win"', {
        'cef_directory' : '<!(echo %CEF_DIRECTORY%)',
        # Disable tcmalloc's debugallocation to avoid crashing during startup.
        'disable_debugallocation': 1,
      }, { # OS!="win"
        'cef_directory' : '<!(echo $CEF_DIRECTORY)',
      }],
      [ 'OS=="mac"', {
        # Strip symbols and create dSYM files for the Release target.
        'mac_strip_release': 1,
      }],
    ]
  }, 'conditions': [
    ['os_posix==1 and OS!="mac" and OS!="android"', {
      'target_defaults': {
        'cflags_cc': ['-Wno-deprecated-declarations'],
      },
    }]
  ],
}
