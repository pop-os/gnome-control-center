project('gvc', 'c',
  meson_version: '>= 0.38.0',
  default_options: ['static=true']
)

if not meson.is_subproject()
  error('This project is only intended to be used as a subproject!')
endif

gnome = import('gnome')

pkglibdir = get_option('pkglibdir')
pkgdatadir = get_option('pkgdatadir')

gio = dependency('gio-2.0')
gobject = dependency('gobject-2.0')
libpulse = dependency('libpulse', version: '>= 2.0')
libpulse_glib = dependency('libpulse-mainloop-glib')

cdata = configuration_data()
cdata.set_quoted('GETTEXT_PACKAGE', get_option('package-name'))
cdata.set_quoted('PACKAGE_VERSION', get_option('package-version'))

libgvc_inc = include_directories('.')

libgvc_gir_sources = [
  'gvc-mixer-card.h',
  'gvc-mixer-card.c',
  'gvc-mixer-stream.h',
  'gvc-mixer-stream.c',
  'gvc-channel-map.h',
  'gvc-channel-map.c',
  'gvc-mixer-ui-device.h',
  'gvc-mixer-ui-device.c',
  'gvc-mixer-sink.h',
  'gvc-mixer-sink.c',
  'gvc-mixer-source.h',
  'gvc-mixer-source.c',
  'gvc-mixer-sink-input.h',
  'gvc-mixer-sink-input.c',
  'gvc-mixer-source-output.h',
  'gvc-mixer-source-output.c',
  'gvc-mixer-event-role.h',
  'gvc-mixer-event-role.c',
  'gvc-mixer-control.h',
  'gvc-mixer-control.c'
]

libgvc_no_gir_sources = [
  'gvc-mixer-card-private.h',
  'gvc-mixer-stream-private.h',
  'gvc-channel-map-private.h',
  'gvc-mixer-control-private.h',
  'gvc-pulseaudio-fake.h'
]

libgvc_deps = [
  gio,
  gobject,
  libpulse,
  libpulse_glib
]

enable_alsa = get_option('alsa')
if enable_alsa
  libgvc_deps += dependency('alsa')
endif
cdata.set('HAVE_ALSA', enable_alsa)

static = get_option('static')
with_introspection = get_option('with-introspection')

if static and with_introspection
  error('Currently meson requires a shared library for building girs.')
endif

c_args = ['-DG_LOG_DOMAIN="Gvc"']

if with_introspection
  c_args += '-DWITH_INTROSPECTION'
endif

if static
  libgvc_static = static_library('gvc',
    sources: libgvc_gir_sources + libgvc_no_gir_sources,
    include_directories: libgvc_inc,
    dependencies: libgvc_deps,
    c_args: c_args
  )

  libgvc = libgvc_static
else
  if pkglibdir == ''
    error('Installing shared library, but pkglibdir is unset!')
  endif

  libgvc_shared = shared_library('gvc',
    sources: libgvc_gir_sources + libgvc_no_gir_sources,
    include_directories: libgvc_inc,
    dependencies: libgvc_deps,
    c_args: c_args,
    install_rpath: pkgdatadir,
    install_dir: pkglibdir,
    install: true
  )

  libgvc = libgvc_shared
endif

if with_introspection
  if pkgdatadir == ''
    error('Installing introspection, but pkgdatadir is unset!')
  elif (pkglibdir == '')
    error('Installing introspection, but pkglibdir is unset!')
  endif

  libgvc_gir = gnome.generate_gir(libgvc,
    sources: libgvc_gir_sources,
    nsversion: '1.0',
    namespace: 'Gvc',
    includes: ['Gio-2.0', 'GObject-2.0'],
    extra_args: ['-DWITH_INTROSPECTION', '--quiet'],
    install_dir_gir: pkgdatadir,
    install_dir_typelib: pkglibdir,
    install: true
  )
endif

if enable_alsa
  executable('test-audio-device-selection',
    sources: 'test-audio-device-selection.c',
    link_with: libgvc,
    dependencies: libgvc_deps,
    c_args: c_args
  )
endif

libgvc_dep = declare_dependency(
  link_with: libgvc,
  include_directories: libgvc_inc,
  dependencies: libgvc_deps
)

config = configure_file(
  input: 'config.h.meson',
  output: 'config.h',
  configuration: cdata
)
