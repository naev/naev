#!/usr/bin/env python3

import argparse
import os
import shutil
import glob
import subprocess

def parse_arguments():
    parser = argparse.ArgumentParser(description='Convert files and build a site.')
    parser.add_argument('--source-dir', dest='source_dir', required=True, help='Path to the source directory.')
    parser.add_argument('--build-dir', dest='build_dir', required=True, help='Path to the build directory.')
    return parser.parse_args()

def copy_file(input_path, output_path):
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    shutil.copy(input_path, output_path)

def clean_directories(build_dir, output_dir):
    for directory in [build_dir, output_dir]:
        if os.path.exists(directory):
            shutil.rmtree(directory)

def copy_directories_and_files(source_dir, build_dir):
    dirs_to_copy = ['content', 'layouts', 'lib']
    os.makedirs(build_dir, exist_ok=True)
    for directory in dirs_to_copy:
        shutil.copytree(os.path.join(source_dir, directory), os.path.join(build_dir, directory))

    files_to_copy = ['nanoc.yaml', 'Rules', 'Gemfile']
    for file_name in files_to_copy:
        shutil.copy(os.path.join(source_dir, file_name), os.path.join(build_dir, file_name))

def convert_xml_to_md(input_path, output_path):
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    try:
        with open(output_path, 'w') as outfile:
            subprocess.run(['yq', '-oy', '.', input_path], stdout=outfile, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error converting XML to MD: {e}")
        raise

def convert_lua_to_md(input_path, output_path):
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    try:
        with open(output_path, 'w') as outfile:
            subprocess.run(['sed', '-n', r'/--\[\[/{:a;n;/--\]\]/q;p;ba}', input_path], stdout=outfile, check=True)
            subprocess.run(['yq', '-oy', '.', '-p', 'xml'], stdout=outfile, check=True)
            with open(input_path, 'r') as lua_file:
                outfile.write(lua_file.read())
    except subprocess.CalledProcessError as e:
        print(f"Error converting Lua to MD: {e}")
        raise

def convert_gfx(input_path, output_path):
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    try:
        copy_file(input_path, output_path)
    except Exception as e:
        print(f"Error copying GFX file: {e}")
        raise

def convert_all_files(source_dir, build_dir):
    try:
        spob_files = glob.glob(os.path.join(source_dir, 'dat/spob/*.xml'))
        for spob_file in spob_files:
            output_path = spob_file.replace(os.path.join(source_dir, 'dat/spob/'), os.path.join(build_dir, 'content/spob/')).replace('.xml', '.md')
            convert_xml_to_md(spob_file, output_path)

        ssys_files = glob.glob(os.path.join(source_dir, 'dat/ssys/*.xml'))
        for ssys_file in ssys_files:
            output_path = ssys_file.replace(os.path.join(source_dir, 'dat/ssys/'), os.path.join(build_dir, 'content/ssys/')).replace('.xml', '.md')
            convert_xml_to_md(ssys_file, output_path)

        fcts_files = glob.glob(os.path.join(source_dir, 'dat/factions/*.xml'))
        for fcts_file in fcts_files:
            output_path = fcts_file.replace(os.path.join(source_dir, 'dat/factions/'), os.path.join(build_dir, 'content/fcts/')).replace('.xml', '.md')
            convert_xml_to_md(fcts_file, output_path)

        misn_files = glob.glob(os.path.join(source_dir, 'dat/missions/*.lua'))
        for misn_file in misn_files:
            output_path = misn_file.replace(os.path.join(source_dir, 'dat/missions/'), os.path.join(build_dir, 'content/misn/'))
            convert_lua_to_md(misn_file, output_path)

        evts_files = glob.glob(os.path.join(source_dir, 'dat/events/*.lua'))
        for evts_file in evts_files:
            output_path = evts_file.replace(os.path.join(source_dir, 'dat/events/'), os.path.join(build_dir, 'content/evts/'))
            convert_lua_to_md(evts_file, output_path)

        gfx_files = glob.glob(os.path.join(source_dir, 'artwork/gfx/spob/**/*.webp')) + \
                    glob.glob(os.path.join(source_dir, 'artwork/gfx/spob/space/**/*.webp')) + \
                    glob.glob(os.path.join(source_dir, 'artwork/gfx/logo/*.webp'))
        for gfx_file in gfx_files:
            output_path = gfx_file.replace(os.path.join(source_dir, 'artwork/'), os.path.join(build_dir, 'content/'))
            convert_gfx(gfx_file, output_path)

    except Exception as e:
        print(f"Error converting files: {e}")
        raise

def install_deps(build_dir):
    try:
        subprocess.run(['bundle', 'install'], cwd=build_dir, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error installing dependencies: {e}")
        raise

def build_site(build_dir):
    try:
        subprocess.run(['bundle', 'exec', 'nanoc'], cwd=build_dir, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error building site: {e}")
        raise

def copy_output(build_dir, output_dir):
    try:
        shutil.copytree(os.path.join(build_dir, "output"), output_dir)
    except Exception as e:
        print(f"Error copying output: {e}")
        raise

def main():
    args = parse_arguments()
    source_dir = os.path.join(args.source_dir, "docs", "lore")
    build_dir = os.path.join(args.build_dir, "docs", "lore.p")
    output_dir = os.path.join(args.build_dir, "docs", "lore")

    try:
        clean_directories(build_dir, output_dir)
        copy_directories_and_files(source_dir, build_dir)
        install_deps(build_dir)
        convert_all_files(source_dir, build_dir)
        build_site(build_dir)
        copy_output(build_dir, output_dir)
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    main()
