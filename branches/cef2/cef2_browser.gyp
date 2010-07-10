# Copyright (c) 2010 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      'target_name': 'patcher',
      'type': 'none',
      'msvs_guid': '5953189C-46EF-4171-8FF7-34334B83BAAF',
      'actions': [
        {
          'action_name': 'patch_source',
          'msvs_cygwin_shell': 0,
          'inputs': [
            'tools/patch_source.bat',
          ],
          'outputs': [
            'tools/patch_source.bat.output',
          ],
          'action': ['', '<@(_inputs)'],
        },
      ],
    },
    {
      'target_name': 'cef2_browser',
      'type': 'static_library',
      'msvs_guid': '16DC39B1-BA78-43B4-9D56-AF991D6B37CD',
      'defines': [
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
        '../skia/skia.gyp:skia',
      ],
      'sources': [
        'common/cef_messages.h',
        'common/cef_messages_internal.h',
        'browser/cef_channel.cc',
        'browser/cef_channel.h',
        'browser/cef_connector.cc',
        'browser/cef_connector.h',
        'browser/cef_service.cc',
        'browser/cef_service.h',
      ],
    },
  ]
}
