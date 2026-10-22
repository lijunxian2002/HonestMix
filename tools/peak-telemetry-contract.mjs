// Read-only validation of actual offline peak telemetry evidence; never builds/runs.
import fs from 'node:fs';
import path from 'node:path';
import crypto from 'node:crypto';

const hash = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const check = (ok, message) => { if (!ok) throw new Error('Peak telemetry: ' + message); };
const json = file => JSON.parse(fs.readFileSync(file, 'utf8'));
const close = (a, b) => Number.isFinite(a) && Math.abs(a - b) <= 1e-7;
const sha = value => typeof value === 'string' && /^[a-f0-9]{64}$/.test(value);
const relative = value => typeof value === 'string' && value.length > 0
  && !path.isAbsolute(value) && !/^[a-z]:/i.test(value) && !value.includes('\\')
  && value.split('/').every(part => part && part !== '.' && part !== '..');
function fileIn(folder, name) {
  check(relative(name), 'unsafe relative evidence path: ' + name);
  let current = folder;
  check(!fs.lstatSync(current).isSymbolicLink(), 'linked evidence root');
  for (const part of name.split('/')) {
    current = path.join(current, part);
    check(!fs.lstatSync(current).isSymbolicLink(), 'linked evidence entry: ' + name);
  }
  check(fs.statSync(current).isFile(), 'not a regular file: ' + name);
  return current;
}
export function peakTelemetryFiles(folder) {
  check(fs.statSync(folder).isDirectory() && !fs.lstatSync(folder).isSymbolicLink(), 'missing/plain evidence directory required');
  const result = [];
  const walk = dir => {
    for (const item of fs.readdirSync(dir, { withFileTypes: true })) {
      const full = path.join(dir, item.name);
      check(!item.isSymbolicLink(), 'evidence tree contains link: ' + full);
      if (item.isDirectory()) walk(full);
      else { check(item.isFile(), 'unsupported evidence entry: ' + full); result.push(full); }
    }
  };
  walk(folder);
  return result.sort();
}
export const PEAK_IMAGE_STEMS = [
  '00-clean', '01-positive-control', '02-reset-clean',
  '03-after-callbacks-before-ui-poll', '04-after-first-ui-poll',
  '05-at500ms-after-smaller-pulse', '06-at1600ms',
  '07-no-warning--1.0', '07-no-warning-0.5', '07-no-warning-1.0',
  '08-prepare-clears-hold', '09-release-clears-hold',
  '10-reopen-discards-closed-editor-events'
];
const sourceFiles = ['CMakeLists.txt', 'Source/PluginProcessor.h', 'Source/PluginProcessor.cpp', 'Source/PluginEditor.h', 'Source/PluginEditor.cpp'];
const sharedRelative = 'build/HonestMixHeadphone_artefacts/Release/HONESTMIX Headphone_SharedCode.lib';
function bindingRelative(binding, entry) {
  const api = /^[a-z]:[\\/]/i.test(binding.source) ? path.win32 : path;
  return api.relative(binding.source, entry.path).replace(/\\/g, '/');
}
export function assertPeakTelemetry({ folder, version, sourceRoot = null, liveBuild = false }) {
  folder = path.resolve(folder);
  const manifest = json(fileIn(folder, 'validation.json'));
  check(manifest.schema === 'honestmix.peak-telemetry-evidence/1' && manifest.version === version, 'manifest schema/version mismatch');
  check(Array.isArray(manifest.files) && manifest.files.length > 0, 'empty evidence file manifest');
  const indexed = new Map();
  for (const entry of manifest.files) {
    check(relative(entry.path) && entry.path !== 'validation.json' && !indexed.has(entry.path), 'duplicate/unsafe manifest entry');
    check(sha(entry.sha256) && Number.isSafeInteger(entry.bytes) && entry.bytes >= 0, 'invalid file identity');
    const file = fileIn(folder, entry.path);
    check(fs.statSync(file).size === entry.bytes && hash(file) === entry.sha256, 'file hash/size mismatch: ' + entry.path);
    indexed.set(entry.path, entry);
  }
  const actual = peakTelemetryFiles(folder).map(file => path.relative(folder, file).replace(/\\/g, '/')).filter(name => name !== 'validation.json');
  check(actual.length === indexed.size && actual.every(name => indexed.has(name)), 'unlisted or missing evidence files');
  for (const name of ['report.json', 'build-binding.json', 'PeakTelemetryProbe.cpp', 'PeakTelemetryProbe.exe', 'build-probe.py', 'compile.log', 'link.log']) {
    check(indexed.has(name), 'required evidence missing: ' + name);
  }
  const report = json(fileIn(folder, 'report.json'));
  const binding = json(fileIn(folder, 'build-binding.json'));
  check(report.schema === 'honestmix.peak-telemetry-probe/1' && report.mode === 'candidate' && report.version === version, 'not a current candidate report');
  check(report.passed === true && report.candidateTelemetryPassed === true && report.candidateUiAcceptancePassed === true
    && report.uiExecuted === true && report.hasAccumulatedAccessorAndGeneration === true && report.baselineDefectReproduced === false, 'full candidate telemetry/UI did not pass');
  check(report.hardwareAudioOpened === false && report.realDawLaunched === false && report.networkRequested === false
    && report.ui?.windowShownOrAudioDeviceOpened === false, 'offline scope flags differ');
  check(report.sampleRate === 44100 && close(report.maximumAbsoluteSampleError, 0) && report.audioCallbackCppNew === 0
    && Number.isSafeInteger(report.actualAudioCallbacks) && report.actualAudioCallbacks > 0
    && Number.isSafeInteger(report.allocationMeasuredSteadyCallbacks) && report.allocationMeasuredSteadyCallbacks > 0
    && report.allocationMeasuredSteadyCallbacks <= report.actualAudioCallbacks, 'sample/steady allocation evidence differs');
  check(binding.schema === 'honestmix.peak-probe-build-plan/1' && binding.mode === 'candidate' && binding.version === version
    && binding.compileExecuted === true && binding.linkExecuted === true && binding.compileExitCode === 0 && binding.linkExitCode === 0
    && binding.preservesInheritedMacrosIncludesResourceObjectModuleObjectsAndSystemLibTokens === true, 'actual probe compile/link binding did not pass');
  // probeExecuted=false is the original builder's truthful "never runs" status.
  // Runtime success is read from the separate actual report, not fabricated here.
  check(binding.probeSourceSha256 === indexed.get('PeakTelemetryProbe.cpp').sha256
    && binding.probeExeSha256 === indexed.get('PeakTelemetryProbe.exe').sha256
    && sha(binding.sharedCodeSha256) && manifest.sharedCodeSha256 === binding.sharedCodeSha256, 'probe source/executable/SharedCode SHA differs');
  check(manifest.reportSha256 === indexed.get('report.json').sha256 && manifest.buildBindingSha256 === indexed.get('build-binding.json').sha256, 'report/build identity differs');
  check(Array.isArray(binding.inheritedLinkInputs) && binding.inheritedLinkInputs.some(entry =>
    entry.path.replace(/\\/g, '/').endsWith('/' + sharedRelative) && entry.sha256 === binding.sharedCodeSha256), 'real SharedCode absent from link-input binding');
  check(Array.isArray(binding.boundFiles) && Array.isArray(manifest.sourceBindings)
    && manifest.sourceBindings.length === sourceFiles.length, 'missing actual tested source bindings');
  const bound = new Map(binding.boundFiles.map(entry => [bindingRelative(binding, entry), entry.sha256]));
  const declared = new Map();
  for (const item of manifest.sourceBindings) {
    check(sourceFiles.includes(item.path) && !declared.has(item.path) && sha(item.sha256) && bound.get(item.path) === item.sha256, 'source binding mismatch: ' + item.path);
    declared.set(item.path, item.sha256);
    if (sourceRoot) check(hash(fileIn(sourceRoot, item.path)) === item.sha256, 'current/delivered source differs from tested source: ' + item.path);
  }
  if (sourceRoot) check(hash(fileIn(sourceRoot, 'tests/PeakTelemetryProbe.cpp')) === indexed.get('PeakTelemetryProbe.cpp').sha256,
    'frozen/delivered probe source differs from the actual compiled probe');
  if (liveBuild) {
    check(sourceRoot, 'live build validation requires source root');
    check(fs.realpathSync(binding.source) === fs.realpathSync(sourceRoot), 'probe built another checkout');
    check(fs.realpathSync(binding.realSharedCode) === fs.realpathSync(path.join(sourceRoot, sharedRelative))
      && hash(path.join(sourceRoot, sharedRelative)) === binding.sharedCodeSha256, 'current SharedCode differs from actual tested library');
  }
  const telemetry = report.telemetry?.rows;
  check(Array.isArray(telemetry) && telemetry.length === 7 && report.telemetry.uiRefreshesBetweenDeclaredCallbacks === 0, 'declared processor cases missing');
  const pulse = telemetry.find(row => row.case === 'actual1.25PulseThenCleanNoUiPoll');
  check(pulse && close(pulse.actualPulseOutputPeak, 1.25) && close(pulse.instantaneousAfterPulse, 1.25)
    && close(pulse.instantaneousAfterClean, 0) && close(pulse.firstRead, 1.25) && close(pulse.secondRead, 0)
    && pulse.candidatePreservedPeakObserved === true && pulse.baselineLostPeakObserved === false, 'actual transient preservation differs');
  const unity = telemetry.filter(row => row.case === 'subunityOrUnityUnchanged');
  check(unity.length === 3 && [-1, 0.5, 1].every(value => unity.some(row => close(row.input, value)
    && close(row.actualSamplePeak, Math.abs(value)) && close(row.readAfterClean, Math.abs(value)) && row.aboveUnity === false)), 'unity/subunity output evidence differs');
  check(close(telemetry.find(row => row.case === 'maximumNotLastCallback')?.readAfter1_5Then1_125ThenClean, 1.5), 'maximum-across-callbacks differs');
  for (const name of ['prepareClearsPendingAndInstantaneous', 'releaseClearsPendingAndInstantaneous']) {
    const row = telemetry.find(row => row.case === name);
    check(row && Number.isSafeInteger(row.generationBefore) && Number.isSafeInteger(row.generationAfter)
      && row.generationBefore !== row.generationAfter, 'reset lifecycle case missing: ' + name);
  }
  const ui = report.ui?.rows;
  check(Array.isArray(ui) && ui.length === 12 && JSON.stringify(report.ui.scalesPercent) === '[100,200]'
    && report.ui.usesNewProductUiTelemetryTestGetters === false, 'full real editor/scales evidence missing');
  const expectations = {
    positiveControl: true, resetAfterPositiveControl: false, afterCallbacksBeforeUiPoll: false,
    afterFirstUiPoll: true, at500msAfterSmallerPulse: true, at1600msSmallerDidNotExtend: false,
    prepareClearsUiHold: false, releaseClearsUiHold: false, reopenDiscardsStalePendingPeak: false
  };
  for (const [stage, warning] of Object.entries(expectations)) {
    const rows = ui.filter(row => row.stage === stage);
    check(rows.length === 1, 'UI stage missing/duplicated: ' + stage);
    const row = rows[0];
    check(row.expectedWarning === warning, 'UI warning expectation differs: ' + stage);
    for (const scale of [100, 200]) {
      const changed = row['footerChangedPixels' + scale];
      check(Number.isSafeInteger(changed) && (warning ? changed > 10 : changed === 0), 'footer pixel state differs: ' + stage);
      if (!['prepareClearsUiHold', 'releaseClearsUiHold'].includes(stage)) check(row['outsideFooterChangedPixels' + scale] === 0, 'stable pixels outside footer changed: ' + stage);
    }
  }
  const noWarning = ui.filter(row => row.stage === 'unityOrBelowNoWarning');
  check(noWarning.length === 3 && noWarning.every(row => row.expectedWarning === false
    && [100, 200].every(scale => row['footerChangedPixels' + scale] === 0 && row['outsideFooterChangedPixels' + scale] === 0)), 'unity/below UI cases differ');
  const elapsed500 = ui.find(row => row.stage === 'at500msAfterSmallerPulse').actualElapsedMs;
  const elapsed1600 = ui.find(row => row.stage === 'at1600msSmallerDidNotExtend').actualElapsedMs;
  check(Number.isFinite(elapsed500) && elapsed500 >= 500 && elapsed500 < 1400
    && Number.isFinite(elapsed1600) && elapsed1600 >= 1600, 'actual hold-clock stages differ');
  const images = PEAK_IMAGE_STEMS.flatMap(stem => [100, 200].map(scale => 'images/' + stem + '-' + scale + 'pct.png'));
  check(Array.isArray(manifest.images) && manifest.images.length === images.length
    && images.every(name => manifest.images.includes(name) && indexed.has(name)), 'required full100/200 snapshot set missing');
  for (const name of images) {
    const png = fs.readFileSync(fileIn(folder, name));
    const scale = name.endsWith('-200pct.png') ? 2 : 1;
    check(png.length > 24 && png.subarray(0, 8).equals(Buffer.from([137, 80, 78, 71, 13, 10, 26, 10]))
      && png.readUInt32BE(16) === 1000 * scale && png.readUInt32BE(20) === 480 * scale, 'snapshot PNG dimensions differ: ' + name);
  }
  for (const scale of [100, 200]) {
    const imageSha = stem => indexed.get('images/' + stem + '-' + scale + 'pct.png').sha256;
    const clean = imageSha('00-clean'), warning = imageSha('04-after-first-ui-poll');
    check(clean !== warning, 'warning and clean screenshots identical');
    for (const stem of ['02-reset-clean', '03-after-callbacks-before-ui-poll', '06-at1600ms',
      '07-no-warning--1.0', '07-no-warning-0.5', '07-no-warning-1.0', '10-reopen-discards-closed-editor-events']) check(imageSha(stem) === clean, 'clean screenshot differs: ' + stem);
    for (const stem of ['01-positive-control', '05-at500ms-after-smaller-pulse']) check(imageSha(stem) === warning, 'held warning screenshot differs: ' + stem);
  }
  return {
    version, reportSha256: indexed.get('report.json').sha256, buildBindingSha256: indexed.get('build-binding.json').sha256,
    probeSourceSha256: binding.probeSourceSha256, probeExeSha256: binding.probeExeSha256,
    sharedCodeSha256: binding.sharedCodeSha256, files: actual.length + 1, images: images.length,
    scope: 'Actual offline SharedCode and hidden editor; no hardware/DAW/true-peak claim.',
    summary: 'Current candidate transient/hold/reset report, real probe/SharedCode SHA and26 snapshots bound'
  };
}
export function assertFrozenPeakTelemetry(frozen, folder) {
  const entries = frozen.files?.filter(entry => entry.kind === 'peak-telemetry') ?? [];
  const files = peakTelemetryFiles(folder);
  check(entries.length === files.length && entries.length > 0, 'frozen peak subtree is absent/incomplete');
  const seen = new Set();
  for (const entry of entries) {
    check(!seen.has(entry.path), 'duplicate frozen peak input');
    seen.add(entry.path);
    const file = fileIn(folder, entry.path);
    check(fs.statSync(file).size === entry.bytes && hash(file) === entry.sha256, 'delivered evidence differs from frozen peak input: ' + entry.path);
  }
  check(files.every(file => seen.has(path.relative(folder, file).replace(/\\/g, '/'))), 'unfrozen peak evidence entry');
}
