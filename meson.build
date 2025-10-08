project('ohset', 
    'c', 
    default_options: ['c_std=c99'],
    version: '1.0.0')

# meson ohsetup -Db_coverage=true builddir

c_args = []

if build_machine.system() == 'windows'
    c_args = ['/W4', '/WX']
else
    c_args = ['-Wall', '-Wextra', '-Werror']
endif

libohset = library(
    'ohset', 
    files('./src/ohset.c'),
    include_directories: include_directories('./include/'),
    version: meson.project_version(),
    soversion: '1',
    c_args: c_args)

ohset_dep = declare_dependency(
  include_directories: include_directories('./include/'),
  link_with: libohset)

tests = executable('ohset-tests',
    sources: [
        'src/ohset.c',
        'test/main.c',
        'test/suite/addtests.c',
        'test/suite/alloctests.c',
        'test/suite/gettests.c',
        'test/suite/hashtests.c',
        'test/suite/itertests.c',
        'test/suite/newtests.c',
        'test/suite/removetests.c',
        'test/test/allocator.c',
        'test/test/test.c'
    ],
    include_directories: include_directories(['include', 'src', 'test']),
    c_args: c_args 
        + '-DOHSET_NO_ABORT'
        + '-DOHSET_DEBUG')

# `ninja coverage` to generate coverage reports

test('ohset-tests', tests)
