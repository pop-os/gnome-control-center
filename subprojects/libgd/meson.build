project('libgd', 'c',
  meson_version: '>= 0.38.0',
  default_options: ['static=true'],
)

if not meson.is_subproject()
  error('This project is only intended to be used as a subproject!')
endif

pkglibdir = get_option('pkglibdir')
pkgdatadir = get_option('pkgdatadir')

libgtk = dependency('gtk+-3.0', version: '>= 3.7.10')
cc = meson.get_compiler('c')
libm = cc.find_library('m', required: false)
libgd_include = include_directories('.')

subdir('libgd')
