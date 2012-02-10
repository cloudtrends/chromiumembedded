# Copyright (c) 2011 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

{
  'variables': {
    'pkg-config': 'pkg-config',
    'chromium_code': 1,
    'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/cef',
  },
  'conditions': [
    [ 'os_posix==1 and OS!="mac" and OS!="android" and gcc_version==46', {
      'target_defaults': {
        # Disable warnings about c++0x compatibility, as some names (such
        # as nullptr) conflict with upcoming c++0x types.
        'cflags_cc': ['-Wno-c++0x-compat'],
      },
    }],
  ],
  'includes': [
    # Bring in the source file lists.
    'cef_paths2.gypi',
   ],
  'targets': [
    {
      'target_name': 'cefclient',
      'type': 'executable',
      'mac_bundle': 1,
      'msvs_guid': '6617FED9-C5D4-4907-BF55-A90062A6683F',
      'dependencies': [
        'cef_pak',
        'libcef',
        'libcef_dll_wrapper',
      ],
      'defines': [
        'USING_CEF_SHARED',
      ],
      'include_dirs': [
        '.',
        # cefclient includes are relative to the tests directory to make
        # creation of binary releases easier.
        'tests'
      ],
      'sources': [
        '<@(includes_common)',
        '<@(includes_wrapper)',
        '<@(cefclient_sources_common)',
      ],
      'mac_bundle_resources': [
        '<@(cefclient_bundle_resources_mac)',
      ],
      'mac_bundle_resources!': [
        # TODO(mark): Come up with a fancier way to do this (mac_info_plist?)
        # that automatically sets the correct INFOPLIST_FILE setting and adds
        # the file to a source group.
        'tests/cefclient/mac/Info.plist',
      ],
      'xcode_settings': {
        'INFOPLIST_FILE': 'tests/cefclient/mac/Info.plist',
      },
      'conditions': [
        ['OS=="win" and win_use_allocator_shim==1', {
          'dependencies': [
            '<(DEPTH)/base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['OS=="win"', {
          'configurations': {
            'Debug_Base': {
              'msvs_settings': {
                'VCLinkerTool': {
                  'LinkIncremental': '<(msvs_large_module_debug_link_mode)',
                },
              },
            },
          },
          'msvs_settings': {
            'VCLinkerTool': {
              # Set /SUBSYSTEM:WINDOWS.
              'SubSystem': '2',
              'EntryPointSymbol' : 'wWinMainCRTStartup',
            },
          },
          'link_settings': {
            'libraries': [
              '-lcomctl32.lib',
              '-lshlwapi.lib',
              '-lrpcrt4.lib',
            ],
          },
          'sources': [
            '<@(includes_win)',
            '<@(cefclient_sources_win)',
          ],
        }],
        ['OS == "win" or (toolkit_uses_gtk == 1 and selinux == 0)', {
          'dependencies': [
            '<(DEPTH)/sandbox/sandbox.gyp:sandbox',
          ],
        }],
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            '<(DEPTH)/build/linux/system.gyp:gtk',
          ],
        }],
        [ 'OS=="mac"', {
          'product_name': 'cefclient',
          'copies': [
            {
              # Add library dependencies to the bundle.
              'destination': '<(PRODUCT_DIR)/cefclient.app/Contents/MacOS/',
              'files': [
                '<(PRODUCT_DIR)/libcef.dylib',
                '<(PRODUCT_DIR)/ffmpegsumo.so',
              ],
            },
            {
              # Add the WebCore resources to the bundle.
              'destination': '<(PRODUCT_DIR)/cefclient.app/Contents/',
              'files': [
                '<(DEPTH)/third_party/WebKit/Source/WebCore/Resources/',
              ],
            },
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
            ],
          },
          'sources': [
            '<@(includes_mac)',
            '<@(cefclient_sources_mac)',
          ],
        }],
        [ 'OS=="linux" or OS=="freebsd" or OS=="openbsd"', {
          'sources': [
            '<@(includes_linux)',
            '<@(cefclient_sources_linux)',
          ],
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)/files',
              'files': [
                '<@(cefclient_bundle_resources_linux)',
              ],
            },
          ],
        }],
      ],
    },
    {
      'target_name': 'cef_unittests',
      'type': 'executable',
      'mac_bundle': 1,
      'msvs_guid': '8500027C-B11A-11DE-A16E-B80256D89593',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/base.gyp:base_i18n',
        '<(DEPTH)/base/base.gyp:test_support_base',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/third_party/icu/icu.gyp:icui18n',
        '<(DEPTH)/third_party/icu/icu.gyp:icuuc',
        'cef_pak',
        'libcef',
        'libcef_dll_wrapper',
      ],
      'sources': [
        'tests/cefclient/cefclient_switches.cpp',
        'tests/cefclient/cefclient_switches.h',
        'tests/unittests/command_line_unittest.cc',
        'tests/unittests/navigation_unittest.cc',
        'tests/unittests/request_unittest.cc',
        'tests/unittests/run_all_unittests.cc',
        'tests/unittests/scheme_handler_unittest.cc',
        'tests/unittests/stream_unittest.cc',
        'tests/unittests/string_unittest.cc',
        'tests/unittests/test_handler.cc',
        'tests/unittests/test_handler.h',
        'tests/unittests/test_suite.cc',
        'tests/unittests/test_suite.h',
        'tests/unittests/url_unittest.cc',
        'tests/unittests/xml_reader_unittest.cc',
        'tests/unittests/zip_reader_unittest.cc',
      ],
      'mac_bundle_resources': [
        'tests/unittests/mac/unittests.icns',
        'tests/unittests/mac/English.lproj/InfoPlist.strings',
        'tests/unittests/mac/English.lproj/MainMenu.xib',
        'tests/unittests/mac/Info.plist',
      ],
      'mac_bundle_resources!': [
        # TODO(mark): Come up with a fancier way to do this (mac_info_plist?)
        # that automatically sets the correct INFOPLIST_FILE setting and adds
        # the file to a source group.
        'tests/unittests/mac/Info.plist',
      ],
      'xcode_settings': {
        'INFOPLIST_FILE': 'tests/unittests/mac/Info.plist',
      },
      'include_dirs': [
        '.',
      ],
      'conditions': [
        [ 'OS=="mac"', {
          'product_name': 'cef_unittests',
          'run_as': {
            'action': ['${BUILT_PRODUCTS_DIR}/${PRODUCT_NAME}.app/Contents/MacOS/${PRODUCT_NAME}'],
          },
          'copies': [
            {
              # Add library dependencies to the bundle.
              'destination': '<(PRODUCT_DIR)/cef_unittests.app/Contents/MacOS/',
              'files': [
                '<(PRODUCT_DIR)/libcef.dylib',
                '<(PRODUCT_DIR)/ffmpegsumo.so',
              ],
            },
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
            ],
          },
          'sources': [
            'tests/unittests/run_all_unittests_mac.mm',
          ],
        }],
      ],
    },
    {
      'target_name': 'libcef',
      'type': 'shared_library',
      'msvs_guid': 'C13650D5-CF1A-4259-BE45-B1EBA6280E47',
      'dependencies': [
        '<(DEPTH)/content/content.gyp:content_app',
        '<(DEPTH)/content/content.gyp:content_browser',
        '<(DEPTH)/content/content.gyp:content_common',
        '<(DEPTH)/content/content.gyp:content_gpu',
        '<(DEPTH)/content/content.gyp:content_plugin',
        '<(DEPTH)/content/content.gyp:content_ppapi_plugin',
        '<(DEPTH)/content/content.gyp:content_renderer',
        '<(DEPTH)/content/content.gyp:content_utility',
        '<(DEPTH)/content/content.gyp:content_worker',
        '<(DEPTH)/content/content_resources.gyp:content_resources',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '<(DEPTH)/build/temp_gyp/googleurl.gyp:googleurl',
        '<(DEPTH)/ipc/ipc.gyp:ipc',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
        '<(DEPTH)/ui/ui.gyp:ui',
        '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
        '<(DEPTH)/webkit/support/webkit_support.gyp:appcache',
        '<(DEPTH)/webkit/support/webkit_support.gyp:database',
        '<(DEPTH)/webkit/support/webkit_support.gyp:fileapi',
        '<(DEPTH)/webkit/support/webkit_support.gyp:glue',
        '<(DEPTH)/webkit/support/webkit_support.gyp:quota',
        'libcef_static',
      ],
      'defines': [
        'BUILDING_CEF_SHARED',
      ],
      'include_dirs': [
        '.',
      ],
      # Avoid "RC1102: internal error : too many arguments to RCPP" error by
      # explicitly specifying a short list of resource include directories.
      'resource_include_dirs' :  [
        '.',
        '..',
      ],
      'sources': [
        '<@(includes_common)',
        '<@(includes_capi)',
        '<@(libcef_sources_common)',
      ],
      'xcode_settings': {
        'INSTALL_PATH': '@executable_path',
        # The libcef_static target contains ObjC categories. Passing the -ObjC flag
        # is necessary to properly load them and avoid a "selector not recognized"
        # runtime error. See http://developer.apple.com/library/mac/#qa/qa1490/_index.html
        # for more information.
        'OTHER_LDFLAGS': ['-Wl,-ObjC'],
      },
      'conditions': [
        ['OS=="win" and win_use_allocator_shim==1', {
          'dependencies': [
            '<(DEPTH)/base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['OS=="win"', {
          'configurations': {
            'Debug_Base': {
              'msvs_settings': {
                'VCLinkerTool': {
                  'LinkIncremental': '<(msvs_large_module_debug_link_mode)',
                },
              },
            },
          },
          'sources': [
            '<@(includes_win)',
            'libcef_dll/libcef_dll.rc',
          ],
          'link_settings': {
            'libraries': [
              '-lcomctl32.lib',
            ],
          },
          'msvs_settings': {
            'VCLinkerTool': {
              # Generate a PDB symbol file for both Debug and Release builds.
              'GenerateDebugInformation': 'true',
            },
          },
        }],
        [ 'OS=="linux" or OS=="freebsd" or OS=="openbsd"', {
          'dependencies':[
            '<(DEPTH)/base/allocator/allocator.gyp:allocator',
          ],
          'direct_dependent_settings': {
            'cflags': [
              '<!@(<(pkg-config) --cflags gtk+-2.0 gthread-2.0)',
            ],
          },
          'link_settings': {
            'ldflags': [
              '<!@(<(pkg-config) --libs-only-L --libs-only-other gtk+-2.0 gthread-2.0)',
            ],
            'libraries': [
              '<!@(<(pkg-config) --libs-only-l gtk+-2.0 gthread-2.0)',
            ],
          },
      	}],
      ],
    },
    {
      'target_name': 'libcef_dll_wrapper',
      'type': 'static_library',
      'msvs_guid': 'A9D6DC71-C0DC-4549-AEA0-3B15B44E86A9',
      'dependencies': [
        'libcef',
      ],
      'defines': [
        'USING_CEF_SHARED',
      ],
      'include_dirs': [
        '.',
      ],
      'sources': [
        '<@(includes_common)',
        '<@(includes_capi)',
        '<@(includes_wrapper)',
        '<@(libcef_dll_wrapper_sources_common)',
      ],
    },
    {
      # Create the locale-specific pack files.
      'target_name': 'cef_locales',
      'type': 'none',
      'dependencies': [
        '<(DEPTH)/ui/base/strings/ui_strings.gyp:ui_strings',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_strings',
      ],
      'variables': {
        'repack_path': '<(DEPTH)/tools/grit/grit/format/repack.py',
        'repack_locales_cmd': ['python', 'tools/repack_locales.py'],
      },
      'conditions': [
        ['OS=="win"', {
          'actions': [
            {
              'action_name': 'repack_locales',
              'inputs': [
                'tools/repack_locales.py',
                # NOTE: Ideally the common command args would be shared
                # amongst inputs/outputs/action, but the args include shell
                # variables which need to be passed intact, and command
                # expansion wants to expand the shell variables. Adding the
                # explicit quoting here was the only way it seemed to work.
                '>!@(<(repack_locales_cmd) -i -g \"<(grit_out_dir)\" -s \"<(SHARED_INTERMEDIATE_DIR)\" -x \"<(INTERMEDIATE_DIR)\" <(locales))',
              ],
              'outputs': [
                '>!@(<(repack_locales_cmd) -o -g \"<(grit_out_dir)\" -s \"<(SHARED_INTERMEDIATE_DIR)\" -x \"<(INTERMEDIATE_DIR)\" <(locales))',
              ],
              'action': [
                '<@(repack_locales_cmd)',
                '-g', '<(grit_out_dir)',
                '-s', '<(SHARED_INTERMEDIATE_DIR)',
                '-x', '<(INTERMEDIATE_DIR)',
                '<@(locales)',
              ],
            },
          ],
        }, { # OS!="win"
          'actions': [
            {
              'action_name': 'repack_locales',
              'process_outputs_as_mac_bundle_resources': 1,
              'inputs': [
                'tools/repack_locales.py',
                # NOTE: Ideally the common command args would be shared
                # amongst inputs/outputs/action, but the args include shell
                # variables which need to be passed intact, and command
                # expansion wants to expand the shell variables. Adding the
                # explicit quoting here was the only way it seemed to work.
                '>!@(<(repack_locales_cmd) -i -g \'<(grit_out_dir)\' -s \'<(SHARED_INTERMEDIATE_DIR)\' -x \'<(INTERMEDIATE_DIR)\' <(locales))',
              ],
              'outputs': [
                '>!@(<(repack_locales_cmd) -o -g \'<(grit_out_dir)\' -s \'<(SHARED_INTERMEDIATE_DIR)\' -x \'<(INTERMEDIATE_DIR)\' <(locales))',
              ],
              'action': [
                '<@(repack_locales_cmd)',
                '-g', '<(grit_out_dir)',
                '-s', '<(SHARED_INTERMEDIATE_DIR)',
                '-x', '<(INTERMEDIATE_DIR)',
                '<@(locales)',
              ],
            },
          ],
        }],
        ['OS != "mac"', {
          'copies': [
            {
              'destination': '<(PRODUCT_DIR)/locales',
              'files': [
                '<!@pymod_do_main(repack_locales -o -g <(grit_out_dir) -s <(SHARED_INTERMEDIATE_DIR) -x <(INTERMEDIATE_DIR) <(locales))'
              ],
            },
          ],
        }],
      ],
    },
    {
      # Create the pack file for CEF resources.
      'target_name': 'cef_resources',
      'type': 'none',
      'actions': [
        {
          'action_name': 'cef_resources',
          'variables': {
            'grit_grd_file': 'libcef/browser/resources/cef_resources.grd',
          },
          'includes': [ '../build/grit_action.gypi' ],
        },
      ],
      'includes': [ '../build/grit_target.gypi' ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)',
          'files': [
            '<(grit_out_dir)/cef_resources.pak'
          ],
        },
      ],
    },
    {
      # Combine all non-localized pack file resources into a single CEF pack file.
      'target_name': 'cef_pak',
      'type': 'none',
      'dependencies': [
        '<(DEPTH)/content/browser/debugger/devtools_resources.gyp:devtools_resources',
        '<(DEPTH)/content/content_resources.gyp:content_resources',
        '<(DEPTH)/net/net.gyp:net_resources',
        '<(DEPTH)/ui/ui.gyp:ui_resources',
        '<(DEPTH)/ui/ui.gyp:ui_resources_standard',
        '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_resources',
        'cef_locales',
        'cef_resources',
      ],
      'variables': {
        'repack_path': '<(DEPTH)/tools/grit/grit/format/repack.py',
      },
      'actions': [
        {
          'action_name': 'repack_cef_pack',
          'process_outputs_as_mac_bundle_resources': 1,
          'variables': {
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/content/content_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/net/net_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources/ui_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/ui_resources_standard/ui_resources_standard.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/devtools_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_chromium_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_resources.pak',
              '<(grit_out_dir)/cef_resources.pak',
            ],
            'conditions': [
              ['OS != "mac"', {
                'pak_inputs': [
                  '<(SHARED_INTERMEDIATE_DIR)/ui/gfx/gfx_resources.pak',
                ]
              }],
            ],
          },
          'inputs': [
            '<(repack_path)',
            '<@(pak_inputs)',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/cef.pak',
          ],
          'action': ['python', '<(repack_path)', '<@(_outputs)',
                     '<@(pak_inputs)'],
        },
      ],
      'conditions': [
        ['OS != "mac"', {
          'dependencies': [
            '<(DEPTH)/ui/ui.gyp:gfx_resources',
          ],
        }],
      ],
    },
    {
      'target_name': 'libcef_static',
      'type': 'static_library',
      'msvs_guid': 'FA39524D-3067-4141-888D-28A86C66F2B9',
      'defines': [
        'BUILDING_CEF_SHARED',
      ],
      'include_dirs': [
        '.',
        # CEF grit resource includes
        '<(grit_out_dir)',
      ],
      'dependencies': [
        '<(DEPTH)/content/content.gyp:content_app',
        '<(DEPTH)/content/content.gyp:content_browser',
        '<(DEPTH)/content/content.gyp:content_common',
        '<(DEPTH)/content/content.gyp:content_gpu',
        '<(DEPTH)/content/content.gyp:content_plugin',
        '<(DEPTH)/content/content.gyp:content_ppapi_plugin',
        '<(DEPTH)/content/content.gyp:content_renderer',
        '<(DEPTH)/content/content.gyp:content_utility',
        '<(DEPTH)/content/content.gyp:content_worker',
        '<(DEPTH)/content/content_resources.gyp:content_resources',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '<(DEPTH)/build/temp_gyp/googleurl.gyp:googleurl',
        '<(DEPTH)/ipc/ipc.gyp:ipc',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/third_party/libxml/libxml.gyp:libxml',
        '<(DEPTH)/third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
        '<(DEPTH)/ui/ui.gyp:ui',
        '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
        '<(DEPTH)/webkit/support/webkit_support.gyp:appcache',
        '<(DEPTH)/webkit/support/webkit_support.gyp:database',
        '<(DEPTH)/webkit/support/webkit_support.gyp:fileapi',
        '<(DEPTH)/webkit/support/webkit_support.gyp:glue',
        '<(DEPTH)/webkit/support/webkit_support.gyp:quota',
        # Necessary to generate the grit include files.
        'cef_pak',
      ],
      'sources': [
        '<@(includes_common)',
        'libcef/browser/command_line_impl.cc',
        'libcef/browser/browser_context.cc',
        'libcef/browser/browser_context.h',
        'libcef/browser/browser_impl.cc',
        'libcef/browser/browser_impl.h',
        'libcef/browser/browser_main.cc',
        'libcef/browser/browser_main.h',
        'libcef/browser/browser_message_filter.cc',
        'libcef/browser/browser_message_filter.h',
        'libcef/browser/browser_message_loop.cc',
        'libcef/browser/browser_message_loop.h',
        'libcef/browser/browser_settings.cc',
        'libcef/browser/browser_settings.h',
        'libcef/browser/content_browser_client.cc',
        'libcef/browser/content_browser_client.h',
        'libcef/browser/context.cc',
        'libcef/browser/context.h',
        'libcef/browser/devtools_delegate.cc',
        'libcef/browser/devtools_delegate.h',
        'libcef/browser/download_manager_delegate.cc',
        'libcef/browser/download_manager_delegate.h',
        'libcef/browser/frame_impl.cc',
        'libcef/browser/frame_impl.h',
        'libcef/browser/navigate_params.cc',
        'libcef/browser/navigate_params.h',
        'libcef/browser/origin_whitelist_impl.cc',
        'libcef/browser/origin_whitelist_impl.h',
        'libcef/browser/resource_context.cc',
        'libcef/browser/resource_context.h',
        'libcef/browser/resource_dispatcher_host_delegate.cc',
        'libcef/browser/resource_dispatcher_host_delegate.h',
        'libcef/browser/resource_request_job.cc',
        'libcef/browser/resource_request_job.h',
        'libcef/browser/request_impl.cc',
        'libcef/browser/request_impl.h',
        'libcef/browser/response_impl.cc',
        'libcef/browser/response_impl.h',
        'libcef/browser/scheme_impl.cc',
        'libcef/browser/scheme_impl.h',
        'libcef/browser/stream_impl.cc',
        'libcef/browser/stream_impl.h',
        'libcef/browser/string_list_impl.cc',
        'libcef/browser/string_map_impl.cc',
        'libcef/browser/string_multimap_impl.cc',
        'libcef/browser/string_types_impl.cc',
        'libcef/browser/task_impl.cc',
        'libcef/browser/thread_util.h',
        'libcef/browser/time_impl.cc',
        'libcef/browser/time_util.h',
        'libcef/browser/url_impl.cc',
        'libcef/browser/url_network_delegate.cc',
        'libcef/browser/url_network_delegate.h',
        'libcef/browser/url_request_context_getter.cc',
        'libcef/browser/url_request_context_getter.h',
        'libcef/browser/url_request_interceptor.cc',
        'libcef/browser/url_request_interceptor.h',
        'libcef/browser/xml_reader_impl.cc',
        'libcef/browser/xml_reader_impl.h',
        'libcef/browser/zip_reader_impl.cc',
        'libcef/browser/zip_reader_impl.h',
        'libcef/common/cef_message_generator.cc',
        'libcef/common/cef_message_generator.h',
        'libcef/common/cef_messages.h',
        'libcef/common/content_client.cc',
        'libcef/common/content_client.h',
        'libcef/common/http_header_utils.cc',
        'libcef/common/http_header_utils.h',
        'libcef/common/main_delegate.cc',
        'libcef/common/main_delegate.h',
        'libcef/plugin/content_plugin_client.cc',
        'libcef/plugin/content_plugin_client.h',
        'libcef/renderer/content_renderer_client.cc',
        'libcef/renderer/content_renderer_client.h',
        'libcef/renderer/render_message_filter.cc',
        'libcef/renderer/render_message_filter.h',
        'libcef/renderer/render_view_observer.cc',
        'libcef/renderer/render_view_observer.h',
        'libcef/utility/content_utility_client.cc',
        'libcef/utility/content_utility_client.h',
        # TabContentsView implementations
        '<(DEPTH)/content/browser/tab_contents/tab_contents_view_win.cc',
        '<(DEPTH)/content/browser/tab_contents/tab_contents_view_win.h',
      ],
      'conditions': [
        ['OS=="win"', {
          'sources': [
            '<@(includes_win)',
            'libcef/browser/browser_impl_win.cc',
            'libcef/browser/browser_main_win.cc',
          ],
        }],
        [ 'OS=="mac"', {
          'sources': [
            '<@(includes_mac)',
            'libcef/browser/browser_impl_mac.mm',
            'libcef/browser/browser_main_mac.mm',
          ],
        }],
        [ 'OS=="linux" or OS=="freebsd" or OS=="openbsd"', {
          'sources': [
            '<@(includes_linux)',
            'libcef/browser/browser_impl_gtk.cc',
            'libcef/browser/browser_main_gtk.cc',
          ],
        }],
      ],
    },
  ]
}
