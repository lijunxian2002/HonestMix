#!/usr/bin/env node
// Optional extraction uses a NEW caller-specified directory. Never deletes.
import fs from 'node:fs';
import path from 'node:path';
import { execFileSync } from 'node:child_process';
import { digest } from './delivery-contract.mjs';
import { assertPeakTelemetry, assertFrozenPeakTelemetry } from './peak-telemetry-contract.mjs';
const argv = process.argv.slice(2);
const option = name => argv[argv.indexOf(name) + 1];
function verify(root, name) {
  const manifest = JSON.parse(fs.readFileSync(path.join(root, name), 'utf8'));
  if (!Array.isArray(manifest.files) || !manifest.files.length) throw new Error('Empty manifest: ' + name);
  for (const entry of manifest.files) {
    const file = path.resolve(root, entry.path), rel = path.relative(root, file);
    if (!rel || rel.startsWith('..') || path.isAbsolute(rel)) throw new Error('Manifest path escapes tree: ' + entry.path);
    let current = root;
    for (const part of rel.split(path.sep)) { current = path.join(current, part); if (fs.lstatSync(current).isSymbolicLink()) throw new Error('Manifest contains a link: ' + entry.path); }
    if (fs.statSync(file).size !== entry.bytes || digest(file) !== entry.sha256) throw new Error('Hash/size mismatch: ' + entry.path);
  }
  return manifest;
}
try {
  if (!argv[0] || argv[0].startsWith('--')) throw new Error('Usage: node tools/verify-delivery.mjs <extracted-package> [--extract-to <NEW-directory>] [--configure]');
  const pkg = path.resolve(argv[0]), outer = verify(pkg, 'SHA256.json');
  const delivery = JSON.parse(fs.readFileSync(path.join(pkg, 'evidence/delivery.json'), 'utf8'));
  if (delivery.version !== outer.version) throw new Error('Package version mismatch');
  const peakFolder = path.join(pkg, 'evidence/peak-telemetry');
  const peak = assertPeakTelemetry({ folder: peakFolder, version: outer.version });
  const frozen = JSON.parse(fs.readFileSync(path.join(pkg, 'evidence/delivery-freeze.json'), 'utf8'));
  if (frozen.schema !== 'honestmix.headphone.delivery-freeze/1' || frozen.version !== outer.version) throw new Error('Frozen delivery schema/version mismatch');
  assertFrozenPeakTelemetry(frozen, peakFolder);
  for (const key of ['version', 'reportSha256', 'buildBindingSha256', 'probeSourceSha256', 'probeExeSha256', 'sharedCodeSha256', 'files', 'images']) {
    if (delivery.peakTelemetry?.[key] !== peak[key]) throw new Error('Delivery peak telemetry identity mismatch: ' + key);
  }
  const archive = path.join(pkg, delivery.sourceArchive);
  if (digest(archive) !== delivery.sourceArchiveSha256) throw new Error('Corresponding-source ZIP hash mismatch');
  let source = null;
  if (argv.includes('--extract-to')) {
    source = path.resolve(option('--extract-to'));
    if (fs.existsSync(source)) throw new Error('Extraction target already exists: ' + source);
    const seven = process.env.HM_7Z || 'C:/Program Files/7-Zip/7z.exe';
    const listing = execFileSync(seven, ['l', '-slt', '-ba', archive], { encoding: 'utf8', maxBuffer: 16 * 1024 * 1024 });
    for (const match of listing.matchAll(/^Path = (.+)$/gm)) {
      const entry = match[1].trim();
      if (path.isAbsolute(entry) || /^[A-Za-z]:/.test(entry) || entry.split(/[\\/]/).includes('..')) throw new Error('Unsafe ZIP entry: ' + entry);
    }
    fs.mkdirSync(source, { recursive: false });
    execFileSync(seven, ['x', archive, '-o' + source, '-y'], { stdio: 'inherit' });
    const inner = verify(source, 'SOURCE-SHA256.json');
    if (inner.version !== outer.version) throw new Error('Corresponding-source version mismatch');
    assertPeakTelemetry({ folder: peakFolder, version: outer.version, sourceRoot: source });
    const indexed = new Map(inner.files.map(entry => [entry.path, entry.sha256]));
    for (const entry of frozen.files.filter(entry => entry.kind === 'source' || entry.kind === 'juce')) {
      const relative = entry.kind === 'juce' ? 'deps/juce/' + entry.path : entry.path;
      if (indexed.get(relative) !== entry.sha256) throw new Error('Source archive differs from frozen build input: ' + relative);
    }
    if (argv.includes('--configure')) {
      execFileSync('cmake', ['-S', source, '-B', path.join(source, '_configure'), '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=Release', '-DHM_JUCE=' + path.join(source, 'deps/juce')], { stdio: 'inherit' });
      const cache = fs.readFileSync(path.join(source, '_configure/CMakeCache.txt'), 'utf8');
      const configured = /^HM_JUCE:PATH=(.+)$/m.exec(cache)?.[1].trim();
      if (!configured || fs.realpathSync(configured) !== fs.realpathSync(path.join(source, 'deps/juce'))) throw new Error('Configure used an external JUCE dependency');
    }
  } else if (argv.includes('--configure')) throw new Error('--configure requires --extract-to <NEW-directory>');
  console.log(JSON.stringify({ passed: true, version: outer.version, packageFiles: outer.files.length, sourceArchiveSha256: digest(archive), sourceExtracted: source, portableConfigurePassed: argv.includes('--configure'), peakTelemetry: peak }));
} catch (error) { console.error(error.message); process.exitCode = 1; }
