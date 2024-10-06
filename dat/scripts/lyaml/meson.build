# This is a horrible ugly hack, but it lets us drag it in from the subproject
fs = import('fs')
foreach file: subproject('lyaml').get_variable('lyaml_lua')
   fs.copyfile( file, install: true, install_dir: ndata_path / 'dat/scripts/lyaml' )
endforeach
