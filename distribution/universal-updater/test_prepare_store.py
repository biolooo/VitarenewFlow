import copy
import hashlib
import tempfile
import unittest
from pathlib import Path
from prepare_store import prepare, NAMES, APP, CACHE, REPO


class StoreTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='vitarenewflow-unistore-')
        self.addCleanup(self.temp.cleanup)
        self.folder = Path(self.temp.name)
        payloads = [b'3DSX'+bytes(64), b'SMDH'+bytes(0x36c0-4), b'\x20\x20\0\0'+bytes(0x2020-4),b'3DSX'+bytes(64),b'# VitarenewFlow assistant\n']
        self.payloads = dict(zip(NAMES, payloads))
        self.release = dict(draft=False, prerelease=False, tag_name='v1.0.0',
                            html_url=f'https://github.com/{REPO}/releases/tag/v1.0.0',
                            published_at='2026-10-16T12:00:00Z', assets=[])
        for name, content in self.payloads.items():
            (self.folder/name).write_bytes(content)
            self.release['assets'].append(dict(name=name, state='uploaded', size=len(content),
                digest='sha256:'+hashlib.sha256(content).hexdigest(),
                browser_download_url=f'https://github.com/{REPO}/releases/download/v1.0.0/{name}'))

    def build(self):
        return prepare(self.release, self.folder, 1)[0]

    def test_format_and_identity(self):
        s = self.build()
        self.assertEqual(s['storeInfo']['version'], 3)
        self.assertEqual(s['storeContent'][0]['info']['title_ids'], [0xF1042])
        self.assertEqual(s['storeContent'][0]['info']['llm_generation'], 'yes')

    def test_reject_unreleased(self):
        for key in ['draft', 'prerelease']:
            with self.subTest(key=key):
                r = copy.deepcopy(self.release); r[key] = True
                with self.assertRaises(ValueError): prepare(r, self.folder, 1)

    def test_reject_development_version(self):
        for tag in ['v0.11.0', 'v1.0.0-rc1', '../latest', 'latest']:
            r = copy.deepcopy(self.release); r['tag_name'] = tag
            with self.assertRaises(ValueError): prepare(r, self.folder, 1)

    def test_reject_foreign_repository(self):
        self.release['html_url'] = 'https://github.com/other/project/releases/tag/v1.0.0'
        with self.assertRaises(ValueError): self.build()

    def test_reject_missing_asset(self):
        self.release['assets'].pop()
        with self.assertRaises(ValueError): self.build()

    def test_reject_duplicate_asset(self):
        self.release['assets'].append(self.release['assets'][0])
        with self.assertRaises(ValueError): self.build()

    def test_reject_hash_or_size_change(self):
        (self.folder/NAMES[0]).write_bytes(b'changed')
        with self.assertRaises(ValueError): self.build()

    def test_reject_untrusted_download(self):
        self.release['assets'][0]['browser_download_url'] = 'https://example.com/app.3dsx'
        with self.assertRaises(ValueError): self.build()

    def test_reject_missing_digest(self):
        self.release['assets'][0].pop('digest')
        with self.assertRaises(ValueError): self.build()

    def test_reject_invalid_revision(self):
        with self.assertRaises(ValueError): prepare(self.release, self.folder, 0)

    def simulate(self, key, fail_download=None, fail_install=False):
        files = {APP+'3ds-library.3dsx': b'old app', APP+'3ds-library.smdh': b'old icon',
                 'sdmc:/boot.firm': b'boot', APP+'language.txt': b'0\n',
                 APP+'covers/game.png': b'cover', 'sdmc:/roms/gba/game.sav': b'save'}
        done = []; downloads = 0
        script = self.build()['storeContent'][0][key]['script']
        for a in script:
            typ = a['type']
            if typ == 'mkdir': continue
            if typ == 'downloadFile':
                downloads += 1
                if downloads == fail_download: break
                files[a['output']] = self.payloads[a['file'].rsplit('/', 1)[1]]
            elif typ == 'move': files[a['new']] = files.pop(a['old'])
            elif typ == 'installCia':
                if fail_install: break
                self.assertIn(a['file'], files)
                done.append('installed')
            elif typ == 'deleteFile':
                self.assertIn('installed', done)
                del files[a['file']]
            else: self.fail('Unexpected command '+typ)
        for path, value in [('sdmc:/boot.firm', b'boot'), (APP+'language.txt', b'0\n'),
                            (APP+'covers/game.png', b'cover'), ('sdmc:/roms/gba/game.sav', b'save')]:
            self.assertEqual(files[path], value)
        return files, done

    def test_cia_installs_companion_3dsx_without_deleting_files(self):
        files, done = self.simulate('Install / Update CIA + 3DSX')
        self.assertEqual(done, ['installed'])
        self.assertEqual(files[APP+'3ds-library.3dsx'], self.payloads[NAMES[0]])
        self.assertIn(CACHE+NAMES[2], files)

    def test_3dsx_option_does_not_install_cia(self):
        files, done = self.simulate('Install / Update 3DSX')
        self.assertFalse(done)
        self.assertEqual(files[APP+'3ds-library.smdh'], self.payloads[NAMES[1]])

    def test_every_download_failure_preserves_existing_app(self):
        for i in range(1, len(NAMES)+1):
            with self.subTest(download=i):
                files, done = self.simulate('Install / Update CIA + 3DSX', fail_download=i)
                self.assertEqual(files[APP+'3ds-library.3dsx'], b'old app')
                self.assertEqual(files[APP+'3ds-library.smdh'], b'old icon')
                self.assertFalse(done)

    def test_install_failure_does_not_delete_staged_cia(self):
        files, done = self.simulate('Install / Update CIA + 3DSX', fail_install=True)
        self.assertFalse(done)
        self.assertIn(CACHE+NAMES[2], files)

    def test_both_options_include_boot_and_copy_helpers(self):
        for key in ['Install / Update CIA + 3DSX','Install / Update 3DSX']:
            files,_=self.simulate(key)
            self.assertEqual(files[APP+'boot-dispatcher.3dsx'],self.payloads[NAMES[3]])
            self.assertEqual(files['sdmc:/gm9/scripts/VitarenewFlow-Copy.gm9'],self.payloads[NAMES[4]])
            targets=[a.get('new','') for a in self.build()['storeContent'][0][key]['script']]
            self.assertNotIn('sdmc:/boot.3dsx',targets)
            self.assertNotIn(APP+'autoboot-installed.3dsx',targets)


if __name__ == '__main__':
    unittest.main(verbosity=2)
