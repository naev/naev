project('sdl2-image', 'c', version: '2.0.5')

cc = meson.get_compiler('c')
foreach arg: ['-Wno-misleading-indentation', '-Wno-sign-compare', '-Wno-unused-function', '-Wno-unused-parameter']
   if cc.has_argument(arg)
      add_project_arguments(arg, language : 'c')
   endif
endforeach


feature_args = ['-DLOAD_PNG', '-DLOAD_WEBP']

sdl2_dep = dependency('sdl2')
png_dep = dependency('libpng', required: true, static: get_option('steamruntime'))
webp_dep = dependency('libwebp', required: true, static: get_option('steamruntime'))


sdl2_image_lib = static_library('sdl2image',
  'IMG.c',
  'IMG_bmp.c',
  'IMG_gif.c',
  'IMG_jpg.c',
  'IMG_lbm.c',
  'IMG_pcx.c',
  'IMG_png.c',
  'IMG_pnm.c',
  'IMG_svg.c',
  'IMG_tga.c',
  'IMG_tif.c',
  'IMG_webp.c',
  'IMG_WIC.c',
  'IMG_xcf.c',
  'IMG_xpm.c',
  'IMG_xv.c',
  'IMG_xxx.c',
  c_args: feature_args,
  dependencies: [png_dep, webp_dep, sdl2_dep])

sdl2_image_dep = declare_dependency(include_directories: '.',
  link_with: sdl2_image_lib)
