import fs from 'node:fs';
import path from 'node:path';
import crypto from 'node:crypto';
import { assertPeakTelemetry, peakTelemetryFiles } from './peak-telemetry-contract.mjs';

export const digest = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
export function filesUnder(dir) {
  if (!fs.existsSync(dir)) throw new Error(`Missing delivery input: ${dir}`);
  const files = [];
  for (const item of fs.readdirSync(dir, { withFileTypes: true })) {
    const full = path.join(dir, item.name);
    if (item.isSymbolicLink()) throw new Error(`Delivery tree contains a link; materialize it first: ${full}`);
    if (item.isDirectory()) files.push(...filesUnder(full));
    else if (item.isFile()) files.push(full);
  }
  return files.sort();
}
export function readReport(file) {
  return JSON.parse(fs.readFileSync(file, 'utf8').replace(/,\s*(?=[}\]])/g, ''));
}
export function optionsFor(root, argv = []) {
  const value = (flag, fallback) => {
    const at = argv.indexOf(flag);
    return at >= 0 && argv[at + 1] && !argv[at + 1].startsWith('--') ? argv[at + 1] : fallback;
  };
  const evidence = path.join(root, 'evidence', 'final');
  const cacheFile = path.join(root, 'build', 'CMakeCache.txt');
  const cache = fs.existsSync(cacheFile) ? fs.readFileSync(cacheFile, 'utf8') : '';
  const configured = /^HM_JUCE:PATH=(.+)$/m.exec(cache)?.[1].trim();
  const explicit = value('--juce', null);
  if (explicit && configured && fs.realpathSync(explicit) !== fs.realpathSync(configured)) throw new Error('--juce differs from the dependency used by this build');
  const juce = explicit || configured || path.join(root, 'deps', 'juce');
  return {
    root: path.resolve(root), juce: path.resolve(juce),
    native: path.resolve(value('--report', path.join(evidence, 'headphone-tests.json'))),
    fidelity: path.resolve(value('--fidelity-report', path.join(evidence, 'fidelity-check.json'))),
    response: path.resolve(value('--response-report', path.join(evidence, 'response-contract.json'))),
    provenance: path.resolve(value('--provenance-report',path.join(evidence,'profile-generation.json'))),
    tempo: path.resolve(value('--tempo-report', path.join(value('--evidence-dir', path.dirname(value('--report',path.join(evidence,'headphone-tests.json')))), 'tempo-recall','result.json'))),
    screenshots: path.resolve(value('--evidence-dir', path.dirname(value('--report', path.join(evidence, 'headphone-tests.json'))))),
    peakTelemetry: path.resolve(path.join(value('--evidence-dir', path.dirname(value('--report', path.join(evidence, 'headphone-tests.json')))), 'peak-telemetry')),
    freeze: path.resolve(value('--freeze-manifest', path.join(evidence, 'delivery-freeze.json')))
  };
}
export function captureDelivery(options) {
  const { root, juce } = options;
  const cmake = fs.readFileSync(path.join(root, 'CMakeLists.txt'), 'utf8');
  const version = /project\s*\(\s*\w+\s+VERSION\s+([\d.]+)/i.exec(cmake)?.[1];
  if (!version) throw new Error('Missing CMake project version');
  const cache = fs.readFileSync(path.join(root, 'build', 'CMakeCache.txt'), 'utf8');
  const home = /^CMAKE_HOME_DIRECTORY:INTERNAL=(.+)$/m.exec(cache)?.[1].trim();
  if (!home || fs.realpathSync(home) !== fs.realpathSync(root)) throw new Error('Build belongs to a different source checkout');
  const entries = [];
  const append = (kind, file, relative) => entries.push({ kind, path: relative.replace(/\\/g, '/'), bytes: fs.statSync(file).size, sha256: digest(file) });
  append('configuration', path.join(root, 'build', 'CMakeCache.txt'), 'build/CMakeCache.txt');
  for (const directory of ['Source', 'tests', 'licenses', 'tools']) for (const file of filesUnder(path.join(root, directory))) append('source', file, path.relative(root, file));
  for (const file of ['CMakeLists.txt', 'build.cmd', 'package.mjs', 'README-先看这里.md', 'README-BUILD.md', 'Collect-Diagnostics.ps1', 'README-远程联调.md', 'VST3-Delivery.ps1', 'Install-VST3.ps1', 'Rollback-VST3.ps1', 'Verify-Local.ps1', '运行离线自检.cmd']) append('source', path.join(root, file), file);
  const binaries = [
    'build/HonestMixHeadphone_artefacts/Release/Standalone/HONESTMIX Headphone.exe',
    'build/HeadphoneTests_artefacts/Release/HeadphoneTests.exe',
    'build/FidelityCheck_artefacts/Release/FidelityCheck.exe',
    'build/ResponseContractTests_artefacts/Release/ResponseContractTests.exe',
    'build/TempoRecallTests_artefacts/Release/TempoRecallTests.exe'
  ];
  for (const file of binaries) append('binary', path.join(root, file), file);
  for (const file of filesUnder(path.join(root, 'build/HonestMixHeadphone_artefacts/Release/VST3/HONESTMIX Headphone.vst3'))) append('binary', file, path.relative(root, file));
  for (const key of ['native', 'fidelity', 'response', 'tempo', 'provenance']) {
    const report = readReport(options[key]);
    if (report.passed !== true) throw new Error(`${key} report has not passed`);
    if (report.version !== undefined && report.version !== version) throw new Error(`${key} report version mismatch`);
    append('report', options[key], key);
  }
  for (const file of filesUnder(options.screenshots).filter(file => path.dirname(file) === options.screenshots && /^ui-.*\.png$/i.test(path.basename(file)))) append('screenshot', file, path.basename(file));
  for (const file of filesUnder(path.dirname(options.tempo)).filter(file=>/\.png$/i.test(file))) append('tempo-screenshot',file,path.relative(path.dirname(options.tempo),file));
  const peakFolder = options.peakTelemetry || path.join(options.screenshots, 'peak-telemetry');
  assertPeakTelemetry({ folder: peakFolder, version, sourceRoot: root, liveBuild: true });
  for (const file of peakTelemetryFiles(peakFolder)) append('peak-telemetry', file, path.relative(peakFolder, file));
  for (const file of filesUnder(juce)) append('juce', file, path.relative(juce, file));
  entries.sort((a, b) => `${a.kind}/${a.path}`.localeCompare(`${b.kind}/${b.path}`, 'en'));
  return { schema: 'honestmix.headphone.delivery-freeze/1', version, roots: options, files: entries };
}
export function assertFrozen(options) {
  const frozen = JSON.parse(fs.readFileSync(options.freeze, 'utf8'));
  const current = captureDelivery(options);
  if (frozen.schema !== current.schema || frozen.version !== current.version || JSON.stringify(frozen.roots) !== JSON.stringify(current.roots) || JSON.stringify(frozen.files) !== JSON.stringify(current.files)) throw new Error('Frozen sources, dependencies, binaries, reports or screenshots changed; rerun the affected checks and explicitly freeze again');
  return { path: options.freeze, sha256: digest(options.freeze), files: current.files.length, version: current.version };
}
