{
  "targets": [
    {
      "target_name": "binding",
      "sources": [
        "src/cpp/addon/addon.cpp",
        "src/cpp/addon/Buffer.cpp",
        "src/cpp/addon/IPC_Callee.cpp",
        "src/cpp/addon/IPC_Caller.cpp",
        "src/cpp/addon/Store.cpp",
        "src/cpp/addon/Utils.cpp",

        "src/cpp/ipc/IPC.cpp",
        "src/cpp/ipc/Message.cpp",
        "src/cpp/ipc/Worker.cpp",
        "src/cpp/SharedMemory.cpp",
        "src/cpp/platform/Unix.cpp",
        "src/cpp/allocator/Allocator.cpp",
        "src/cpp/allocator/PoolAllocator.cpp",
        "src/cpp/allocator/STL.cpp",
        "src/cpp/utils/Logger.cpp"
      ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")",
                                                "jemalloc/include"
      ],
      'conditions': [
              [ 'OS!="win"', {
                'cflags_cc': [
                  '-fexceptions'
                ]
              }],
              [ 'OS=="mac"', {
                'xcode_settings': {
                  'OTHER_CFLAGS': [
                    '-fexceptions'
                  ],
                }
            }]
        ]
    },
    {
      "target_name": "boot",
      "sources": [
        "src/cpp/boot.cpp",

        "src/cpp/ipc/IPC.cpp",
        "src/cpp/ipc/Message.cpp",
        "src/cpp/ipc/Worker.cpp",
        "src/cpp/SharedMemory.cpp",
        "src/cpp/platform/Unix.cpp",
        "src/cpp/allocator/Allocator.cpp",
        "src/cpp/allocator/PoolAllocator.cpp",
        "src/cpp/allocator/STL.cpp",
        "src/cpp/utils/Configuration.cpp",
        "src/cpp/utils/Logger.cpp"
      ],
      "libraries": [
        "<(module_root_dir)/deps/libnode.64.dylib",
        "-Wl,-rpath,<(module_root_dir)/deps"
      ],
      "type": "executable",
      "include_dirs" : [
        "<!(node -e \"require('nan')\")",
        "jemalloc/include"
      ],
      'conditions': [
              [ 'OS!="win"', {
                'cflags_cc': [
                  '-fexceptions'
                ]
              }],
              [ 'OS=="mac"', {
                'xcode_settings': {
                  'OTHER_CFLAGS': [
                    '-fexceptions',
                    '-frtti',
                  ],
                }
            }]
        ]
    }
  ]
}