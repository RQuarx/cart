project('cart', 'cpp',
        meson_version: '>=1.2.0',
        version:       '0.1.0',
        license:       'GPL-3.0-or-later',
        default_options: { 'cpp_std': 'c++23',
#                           'cpp_importstd': true, shii doesn't work :<
                           'warning_level': '3' })

add_global_arguments([ '-DTOML_EXCEPTIONS=0' ], language: 'cpp')

dependencies = [ dependency('sdl3'),
                 dependency('tomlplusplus'),
                 dependency('boost.ut'),
                 dependency('spdlog'),
                 dependency('sdl3-ttf', fallback: [ 'sdl3_ttf', 'sdl3_ttf_dep' ]),
                 dependency('lyra',     fallback: [ 'Lyra', 'lyra_dep' ]) ]

configure_file(output: 'metadata.hh',
               input:  'meson/metadata.hh.in',
               configuration: { 'PROJECT_NAME':    meson.project_name(),
                                'PROJECT_VERSION': meson.project_version(),
                                'PROJECT_LICENSE': meson.project_license()[0] })
subdir('src')
executable(meson.project_name(), src,
           dependencies:        dependencies,
           include_directories: include_directories('include', '.'),
           cpp_args:            [ '-DSPDLOG_USE_STD_FORMAT' ])