project('physfs', 'c', version: '3.0.2')

# Contrary to Meson 0.56's documentation, specifying default_options: ['warning_level=1'] has no effect. DIY.
cc = meson.get_compiler('c')
foreach arg: ['-Wno-int-in-bool-context', '-Wno-sign-compare', '-Wno-unused-parameter']
   if cc.has_argument(arg)
      add_project_arguments(arg, language : 'c')
   endif
endforeach

# Increment this if/when we break backwards compatibility.
physfs_soversion = 1

physfs_deps = [dependency('threads')]
physfs_sources = []

if host_machine.system() == 'darwin'
   add_languages('objc', native: false)
   physfs_deps += dependency('Foundation', required: true )
   physfs_deps += dependency('IOKit', required: true )
   physfs_sources += 'src/physfs_platform_apple.m'
elif host_machine.system() == 'haiku'
   add_languages('cpp', native: false)
   cpp = meson.get_compiler('cpp')
   physfs_deps += cpp.find_library('be')
   physfs_deps += cpp.find_library('root')
   physfs_sources += 'src/physfs_platform_haiku.cpp'
endif

# TODO: Not nobly supporting SunPro/WinRT at this point.

physfs_sources += [
   'src/physfs.c',
   'src/physfs_byteorder.c',
   'src/physfs_unicode.c',
   'src/physfs_platform_posix.c',
   'src/physfs_platform_unix.c',
   'src/physfs_platform_windows.c',
   'src/physfs_platform_os2.c',
   'src/physfs_platform_qnx.c',
   'src/physfs_archiver_dir.c',
   'src/physfs_archiver_unpacked.c',
   'src/physfs_archiver_grp.c',
   'src/physfs_archiver_hog.c',
   'src/physfs_archiver_7z.c',
   'src/physfs_archiver_mvl.c',
   'src/physfs_archiver_qpak.c',
   'src/physfs_archiver_wad.c',
   'src/physfs_archiver_zip.c',
   'src/physfs_archiver_slb.c',
   'src/physfs_archiver_iso9660.c',
   'src/physfs_archiver_vdf.c',
]

libphysfs = static_library('physfs', physfs_sources, dependencies: physfs_deps)

physfs_dep = declare_dependency(
    link_with: libphysfs,
    include_directories: 'src',
    dependencies: physfs_deps,
    version: meson.project_version())
