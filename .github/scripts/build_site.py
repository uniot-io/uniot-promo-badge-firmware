#!/usr/bin/env python3
"""
Assembles the firmware site from every release: each release's images under <version>/, and
versions.json listing them, newest first. The input holds one folder per release with the
release.json and images package_release.py produced; the workflow downloads them from
GitHub Releases.

    python .github/scripts/build_site.py releases site https://github.com/uniot-io/uniot-promo-badge-firmware

versions.json is what uniot-web-installer reads, so its shape is a contract:

    {
      "device": "badge",
      "latest": "1.0.0",
      "releases": [
        {
          "version": "1.0.0", "date": "2026-09-26", "core": "0.9.0", "lisp": "0.4.0",
          "chipFamily": "ESP32-C3",
          "notes": "https://github.com/.../releases/tag/1.0.0",
          "images": {
            "compatible": { "path": "1.0.0/compatible.bin", "size": 1266624, "sha256": "…" },
            "full-range": { … }, "hardware-test": { … }
          }
        }
      ]
    }

Paths are relative to versions.json. A consumer should ignore keys it does not know, so
fields can be added without breaking it.
"""
import hashlib
import json
import os
import shutil
import sys

INSTALLER_URL = 'https://install.uniot.io/'


def semver(version):
    return tuple(int(part) for part in version.split('.'))


def sha256(path):
    digest = hashlib.sha256()
    with open(path, 'rb') as data:
        for chunk in iter(lambda: data.read(1 << 16), b''):
            digest.update(chunk)
    return digest.hexdigest()


def main(releases_dir, site_dir, repo_url):
    if os.path.exists(site_dir):
        shutil.rmtree(site_dir)
    os.makedirs(site_dir)

    releases = []
    for name in sorted(os.listdir(releases_dir)):
        manifest = os.path.join(releases_dir, name, 'release.json')
        if not os.path.isfile(manifest):
            print(f'skipping {name}: no release.json (released before packaging existed?)')
            continue
        with open(manifest) as data:
            release = json.load(data)

        version = release['version']
        os.makedirs(os.path.join(site_dir, version))
        for image in release['images'].values():
            source = os.path.join(releases_dir, name, image.pop('file'))
            # A release whose upload was interrupted would otherwise go live with a truncated
            # image, and a badge would be flashed with it.
            if sha256(source) != image['sha256']:
                sys.exit(f'{source} does not match the sha256 in its release.json')
            image['path'] = f'{version}/{os.path.basename(source)}'
            shutil.copyfile(source, os.path.join(site_dir, image['path']))

        release['notes'] = f'{repo_url}/releases/tag/{version}'
        releases.append(release)

    if not releases:
        sys.exit('no releases to publish')

    devices = {release['device'] for release in releases}
    if len(devices) != 1:
        sys.exit(f'releases name more than one device: {sorted(devices)}')

    releases.sort(key=lambda release: semver(release['version']), reverse=True)
    versions = {
        'device': devices.pop(),
        'latest': releases[0]['version'],
        'releases': [{key: release[key] for key in release if key != 'device'} for release in releases],
    }
    with open(os.path.join(site_dir, 'versions.json'), 'w') as out:
        json.dump(versions, out, indent=2)
        out.write('\n')

    # This site is files for the installer, not a page. Anyone who opens it lands there instead.
    with open(os.path.join(site_dir, 'index.html'), 'w') as out:
        out.write(f'<!doctype html><meta charset="utf-8"><title>Uniot badge firmware</title>'
                  f'<meta http-equiv="refresh" content="0; url={INSTALLER_URL}">'
                  f'<a href="{INSTALLER_URL}">{INSTALLER_URL}</a>\n')

    print(f'{len(releases)} release(s), latest {versions["latest"]}')


if __name__ == '__main__':
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    main(*sys.argv[1:])
