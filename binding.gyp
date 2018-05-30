{
  "targets": [
    {
      "target_name": "binding",
      "sources": [
        "src/main.cpp",
        "src/PosixSharedBuffer.cpp"
      ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
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