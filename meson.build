project('ohset', 
    'c', 
    default_options: ['c_std=c99'],
    version: '1.0.1')

c_args = []

if build_machine.system() == 'windows'
    c_args = ['/W4', '/WX']
else
    c_args = ['-Wall', '-Wextra', '-Werror', '-Wpedantic']
endif

libohset = library(
    'ohset', 
    files('./src/ohset.c'),
    include_directories: include_directories('./include/'),
    version: meson.project_version(),
    soversion: '1',
    c_args: c_args 
        + '-DOHSET_EXPORTS')

ohset_dep = declare_dependency(
  include_directories: include_directories('./include/'),
  link_with: libohset)

ex_dictionary = executable('example-dictionary', 
    sources: files('./example/main-dictionary.c'),
    dependencies: [ohset_dep])

ex_int = executable('example-int', 
    sources: files('./example/main-int.c'),
    dependencies: [ohset_dep])

ex_alloc = executable('example-alloc', 
    sources: files('./example/main-alloc.c'),
    dependencies: [ohset_dep])


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
        'test/suite/puttests.c',
        'test/suite/removetests.c',
        'test/suite/shrinktests.c',
        'test/test/allocator.c'
    ],
    include_directories: include_directories(['include', 'src', 'test']),
    c_args: c_args 
        + '-DOHSET_NO_ABORT'
        + '-DOHSET_DEBUG'
        + '-DOHSET_TEST')

test('ohset-tests', tests)
test('example-alloc', ex_alloc)
test('example-dictionary', ex_dictionary)
test('example-int', ex_int)