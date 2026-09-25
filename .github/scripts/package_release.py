#!/usr/bin/env python3
"""
Packages one release: a single flashable image per build, merged so it goes on at offset 0,
and release.json describing them.

    python .github/scripts/package_release.py 1.0.0 dist

Run it after building, with the same PLATFORMIO_BUILD_FLAGS the build had. PlatformIO counts
that variable as part of the project configuration and empties every build directory when it
changes -- and `pio project metadata`, which this script calls, is enough to trigger that.

The merged image covers the bootloader, partition table and application only. It ends long
before the filesystem partition, so flashing it without erasing keeps a badge's WiFi
settings, identity and script.
"""
import datetime
import hashlib
import json
import os
import shlex
import subprocess
import sys

# PlatformIO environment -> the name its image goes by, in release assets and in
# uniot-web-installer. Renaming one breaks the installer's lookup, so change both together.
IMAGES = {
    'uniot_app': 'compatible',
    'uniot_app_full_range': 'full-range',
    'factory_test': 'hardware-test',
}

DEVICE = 'badge'            # SerialIdentity's device= in src/uniot_app/main.cpp
CHIP = 'esp32c3'            # as esptool names it
CHIP_FAMILY = 'ESP32-C3'    # as esptool reports a connected chip, which the installer compares

# CI installs esptool as a module; locally, point this at PlatformIO's copy if you prefer.
ESPTOOL = shlex.split(os.environ.get('ESPTOOL', f'{shlex.quote(sys.executable)} -m esptool'))


def metadata(env):
    output = subprocess.check_output(['pio', 'project', 'metadata', '-e', env, '--json-output'])
    return json.loads(output)[env]


def library_version(env, name):
    with open(os.path.join('.pio', 'libdeps', env, name, 'library.json')) as manifest:
        return json.load(manifest)['version']


def sha256(path):
    digest = hashlib.sha256()
    with open(path, 'rb') as data:
        for chunk in iter(lambda: data.read(1 << 16), b''):
            digest.update(chunk)
    return digest.hexdigest()


def merge(env, output):
    meta = metadata(env)
    firmware = os.path.splitext(meta['prog_path'])[0] + '.bin'
    if not os.path.exists(firmware):
        sys.exit(f'{firmware} is missing. If the build ran, PlatformIO emptied the build '
                 'directory because PLATFORMIO_BUILD_FLAGS differs from the build\'s -- run '
                 'this with the same value.')

    parts = []
    for image in meta['extra']['flash_images']:
        parts += [image['offset'], image['path']]
    parts += [meta['extra']['application_offset'], firmware]

    # "keep" leaves flash mode, frequency and size as PlatformIO wrote them into the
    # bootloader for this board, rather than restating them here.
    subprocess.check_call(ESPTOOL + ['--chip', CHIP, 'merge_bin', '-o', output,
                                     '--flash_mode', 'keep', '--flash_freq', 'keep',
                                     '--flash_size', 'keep'] + parts,
                          stdout=subprocess.DEVNULL)


def main(version, out_dir):
    os.makedirs(out_dir, exist_ok=True)

    images = {}
    for env, name in IMAGES.items():
        file = f'{name}.bin'
        path = os.path.join(out_dir, file)
        merge(env, path)
        images[name] = {'file': file, 'size': os.path.getsize(path), 'sha256': sha256(path)}
        print(f'{name:14} {images[name]["size"]:>9} bytes  {images[name]["sha256"][:16]}…')

    release = {
        'device': DEVICE,
        'version': version,
        'date': datetime.datetime.now(datetime.timezone.utc).date().isoformat(),
        'core': library_version('uniot_app', 'uniot-core'),
        'lisp': library_version('uniot_app', 'uniot-lisp'),
        'chipFamily': CHIP_FAMILY,
        'images': images,
    }
    with open(os.path.join(out_dir, 'release.json'), 'w') as out:
        json.dump(release, out, indent=2)
        out.write('\n')


if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    main(*sys.argv[1:])
