# Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      'target_name': 'libcef2_static',
      'type': 'static_library',
      'msvs_guid': '0B319370-59A3-4B26-8D93-D2BB6121903F',
      'defines': [
        'BUILDING_CEF2_SHARED',
      ],
      'include_dirs': [
        '.',
        '..',
      ],
      'dependencies': [
        '../app/app.gyp:app_base',
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../ipc/ipc.gyp:ipc',
      ],
      'sources': [
        'common/cef_messages.h',
        'common/cef_messages_internal.h',
        'include/cef2.h',
        'include/cef2_base.h',
        'include/cef2_export.h',
        'include/cef2_string.h',
        'include/cef2_types.h',
        'include/cef2_types_win.h',
        'include/cef2_win.h',
        'libcef/cef_browser_impl.cc',
        'libcef/cef_browser_impl.h',
        'libcef/cef_context.cc',
        'libcef/cef_context.h',
        'libcef/cef_listener_manager.h',
        'libcef/cef_process.cc',
        'libcef/cef_process.h',
        'libcef/cef_process_sub_thread.cc',
        'libcef/cef_process_sub_thread.h',
        'libcef/cef_process_ui_thread.cc',
        'libcef/cef_process_ui_thread.h',
        'libcef/cef_profile_impl.cc',
        'libcef/cef_profile_impl.h',
        'libcef/cef_server.cc',
        'libcef/cef_server.h',
        'libcef/cef_string.c',
        'libcef/cef_tab_impl.cc',
        'libcef/cef_tab_impl.h',
        'libcef/cef_thread.cc',
        'libcef/cef_thread.h',
      ],
    },
    {
      'target_name': 'libcef2',
      'type': 'shared_library',
      'msvs_guid': '112AF04C-A453-4056-B818-3F6DD0A9D8B4',
      'dependencies': [
        '../app/app.gyp:app_base',
        '../base/base.gyp:base',
        '../base/base.gyp:base_i18n',
        '../ipc/ipc.gyp:ipc',
        'libcef2_static',
      ],
      'defines': [
        'BUILDING_CEF2_SHARED',
      ],
      'include_dirs': [
        '.',
        '..',
      ],
      # Avoid "RC1102: internal error : too many arguments to RCPP" error by
      # explicitly specifying a short list of resource include directories.
      'resource_include_dirs' :  [
        '.',
        '..',
      ],
      'sources': [
        'include/cef2.h',
        'include/cef2_base.h',
        'include/cef2_capi.h',
        'include/cef2_export.h',
        'include/cef2_string.h',
        'include/cef2_types.h',
        'include/cef2_types_win.h',
        'include/cef2_win.h',
        'libcef_dll/cef_logging.h',
        'libcef_dll/cpptoc/browser_cpptoc.cc',
        'libcef_dll/cpptoc/browser_cpptoc.h',
        'libcef_dll/cpptoc/cpptoc.h',
        'libcef_dll/cpptoc/profile_cpptoc.cc',
        'libcef_dll/cpptoc/profile_cpptoc.h',
        'libcef_dll/cpptoc/tab_cpptoc.cc',
        'libcef_dll/cpptoc/tab_cpptoc.h',
        'libcef_dll/ctocpp/browser_listener_ctocpp.cc',
        'libcef_dll/ctocpp/browser_listener_ctocpp.h',
        'libcef_dll/ctocpp/ctocpp.h',
        'libcef_dll/ctocpp/tab_listener_ctocpp.cc',
        'libcef_dll/ctocpp/tab_listener_ctocpp.h',
        'libcef_dll/libcef_dll.cc',
        'libcef_dll/libcef_dll.rc',
        'libcef_dll/resource.h',
        'libcef_dll/transfer_util.cpp',
        'libcef_dll/transfer_util.h',
      ],
    },
    {
      'target_name': 'libcef2_dll_wrapper',
      'type': 'static_library',
      'msvs_guid': '74ABF629-AD6E-4567-ACCA-36610EA017B0',
      'dependencies': [
        'libcef2',
      ],
      'defines': [
        'USING_CEF2_SHARED',
      ],
      'include_dirs': [
        '.',
        '..',
      ],
      'sources': [
        'libcef_dll/cef_logging.h',
        'libcef_dll/cpptoc/browser_listener_cpptoc.cc',
        'libcef_dll/cpptoc/browser_listener_cpptoc.h',
        'libcef_dll/cpptoc/cpptoc.h',
        'libcef_dll/cpptoc/tab_listener_cpptoc.cc',
        'libcef_dll/cpptoc/tab_listener_cpptoc.h',
        'libcef_dll/ctocpp/browser_ctocpp.cc',
        'libcef_dll/ctocpp/browser_ctocpp.h',
        'libcef_dll/ctocpp/ctocpp.h',
        'libcef_dll/ctocpp/profile_ctocpp.cc',
        'libcef_dll/ctocpp/profile_ctocpp.h',
        'libcef_dll/ctocpp/tab_ctocpp.cc',
        'libcef_dll/ctocpp/tab_ctocpp.h',
        'libcef_dll/transfer_util.cpp',
        'libcef_dll/transfer_util.h',
        'libcef_dll/wrapper/libcef_dll_wrapper.cc',
      ],
    },
    {
      'target_name': 'cefclient',
      'type': 'executable',
      'msvs_guid': 'A6168A04-EB90-4884-9FA7-8C686F4DF0CF',
      'dependencies': [
        'libcef2_dll_wrapper',
      ],
      'defines': [
        'USING_CEF2_SHARED',
      ],
      'include_dirs': [
        '.',
        '..',
      ],
      'sources': [
        'tests/cefclient/Resource.h',
        'tests/cefclient/cefclient.cpp',
        'tests/cefclient/cefclient.h',
        'tests/cefclient/cefclient.ico',
        'tests/cefclient/cefclient.rc',
        'tests/cefclient/res/cefclient.ico',
        'tests/cefclient/res/logo.jpg',
        'tests/cefclient/res/logoball.jpg',
        'tests/cefclient/res/small.ico',
        'tests/cefclient/res/uiplugin.html',
        'tests/cefclient/resource_util.cpp',
        'tests/cefclient/resource_util.h',
        'tests/cefclient/string_util.cpp',
        'tests/cefclient/string_util.h',
      ],
      'link_settings': {
        'libraries': [
          '-lcomctl32.lib',
          '-lshlwapi.lib',
          '-lrpcrt4.lib',
        ],
      },
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {
            'VCLinkerTool': {
              # Set /SUBSYSTEM:WINDOWS.
              'SubSystem': '2',
              'EntryPointSymbol' : 'wWinMainCRTStartup',
            },
          },
        }],
      ],
    },
  ]
}
