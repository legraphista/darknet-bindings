{
  "targets": [
    {
      "target_name": "darknet-bindings",
      'include_dirs': [
        "<!@(node -p \"require('napi-thread-safe-callback').include\")",
        "<!@(node -p \"require('node-addon-api').include\")",
        "src",
        "darknet/src",
        "darknet/include"
       ],
      "cflags" : [
        "-std=c++11"
      ],
      'cflags!': [ '-fno-exceptions', '-D_THREAD_SAFE' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      "ldflags" : [
        "-Wl,-rpath,'$$ORIGIN'"
      ],
      "libraries": [
        "<!(node ./libs)"
      ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7',
        "OTHER_CFLAGS": [
          "-std=c++11",
          "-stdlib=libc++"
        ],
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      },
      'conditions': [
        ['OS=="win"', { 'defines': [ '_HAS_EXCEPTIONS=1' ] }]
      ],
      "sources": [
        "<!@(ls -1 src/*.cc)"
      ],
    }
  ]
}