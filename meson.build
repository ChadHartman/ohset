project('set', 
    'c', 
    default_options: ['c_std=c99'],
    version: '1.0.0')

# meson setup -Db_coverage=true builddir

c_args = []

if build_machine.system() == 'windows'
    c_args = ['/W4', '/WX']
else
    c_args = ['-Wall', '-Wextra', '-Werror']
endif

libset = library(
    'set', 
    files('./src/set.c'),
    include_directories: include_directories('./include/'),
    version: meson.project_version(),
    soversion: '1',
    c_args: c_args)

set_dep = declare_dependency(
  include_directories: include_directories('./include/'),
  link_with: libset)

tests = executable('set-tests',
    sources: [
        'src/set.c',
        'test/main.c'
    ],
    include_directories: include_directories(['include', 'src', 'test']),
    c_args: c_args)

# `ninja coverage` to generate coverage reports

test('set-tests', tests)
