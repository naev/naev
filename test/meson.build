test('Reaches main menu',
    find_program('watch-for-msg.py'),
    args: [
        naev_bin,
        meson.source_root() / 'dat',
        'Reached main menu'],
    workdir: meson.source_root(),
    protocol: 'exitcode')