
{
  'variables': {
      'node_module_sources': [
          "tilemill.cc",
      ]
  },
  'targets': [
    {
      'target_name': 'TileMill',
      'product_name': 'TileMIll',
      'type': 'executable',
      'product_prefix': '',
      'product_extension':'exe',
      'sources': [
        '<@(node_module_sources)',
      ],
      'defines': [
        'PLATFORM="<(OS)"',
        '_LARGEFILE_SOURCE',
        '_FILE_OFFSET_BITS=64',
      ],
      'conditions': [
        [ 'OS=="mac"', {
          'libraries': [
            '-undefined dynamic_lookup'
          ],
          'include_dirs': [
          ],
        }],
        [ 'OS=="win"', {
          'defines': [
            'PLATFORM="win32"',
            '_LARGEFILE_SOURCE',
            '_FILE_OFFSET_BITS=64',
            '_WINDOWS',
            'BUILDING_NODE_EXTENSION'
          ],
          'libraries': [
          ],
          'include_dirs': [
          ],
          'msvs_settings': {
            'VCLinkerTool': {
			  'target_conditions': [
			     ['_type=="executable"', {
                     'SubSystem': 2,
				 }],
			  ],
              'AdditionalLibraryDirectories': [
              ],
            },
			'VCResourceCompilerTool' : {
			      'PreprocessorDefinitions': ["_WINDOWS"]
			},
			},
        },
      ], # windows
      ] # condition
    }, # targets
  ],
}