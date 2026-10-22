#!/usr/bin/env node
// Copy exact already-run evidence only; never compiles, launches, or rewrites it.
import fs from 'node:fs';
import path from 'node:path';
import crypto from 'node:crypto';
import { assertPeakTelemetry, PEAK_IMAGE_STEMS } from './peak-telemetry-contract.mjs';
const hash = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const read = file => JSON.parse(fs.readFileSync(file, 'utf8'));
const check = (ok, message) => { if (!ok) throw new Error(message); };
try {
  const [probeArg, outArg] = process.argv.slice(2);
  check(probeArg && outArg, 'Usage: node tools/stage-peak-telemetry.mjs <actual-candidate-probe-directory> <NEW-evidence-directory>');
  const probe = path.resolve(probeArg), output = path.resolve(outArg);
  check(!fs.existsSync(output), 'Evidence target exists; preserve it and choose a new target');
  const binding = read(path.join(probe, 'build-binding.json'));
  const reportFile = path.join(probe, 'run-001/report.json');
  const report = read(reportFile);
  check(binding.mode === 'candidate' && report.mode === 'candidate' && report.version === binding.version
    && report.passed === true && report.candidateTelemetryPassed === true && report.candidateUiAcceptancePassed === true,
    'Actual full candidate evidence required');
  const sources = ['CMakeLists.txt', 'Source/PluginProcessor.h', 'Source/PluginProcessor.cpp', 'Source/PluginEditor.h', 'Source/PluginEditor.cpp'];
  const api = /^[a-z]:[\\/]/i.test(binding.source) ? path.win32 : path;
  const bound = new Map(binding.boundFiles.map(entry => [api.relative(binding.source, entry.path).replace(/\\/g, '/'), entry.sha256]));
  const sourceBindings = sources.map(relative => {
    const file = path.join(binding.source, relative);
    check(hash(file) === bound.get(relative), 'Tested product source changed: ' + relative);
    return { path: relative, sha256: bound.get(relative) };
  });
  const probeSource = binding.probeSource;
  check(hash(probeSource) === binding.probeSourceSha256, 'Actual compiled probe source changed');
  check(hash(path.join(probe, 'PeakTelemetryProbe.exe')) === binding.probeExeSha256, 'Actual built probe executable changed');
  check(hash(binding.realSharedCode) === binding.sharedCodeSha256, 'Actual tested SharedCode changed');
  const builder = path.join(path.dirname(probeSource), 'build-probe.py');
  const builderBinding = binding.boundFiles.find(entry => path.resolve(entry.path) === path.resolve(builder));
  check(builderBinding && hash(builder) === builderBinding.sha256, 'Actual probe builder changed');
  fs.mkdirSync(output, { recursive: true });
  const files = [];
  const copy = (from, relative) => {
    check(fs.statSync(from).isFile() && !fs.lstatSync(from).isSymbolicLink(), 'Regular evidence file required: ' + from);
    const to = path.join(output, relative);
    fs.mkdirSync(path.dirname(to), { recursive: true });
    fs.copyFileSync(from, to, fs.constants.COPYFILE_EXCL);
    const stat = fs.statSync(from);
    fs.utimesSync(to, stat.atime, stat.mtime);
    check(hash(from) === hash(to), 'Copied evidence hash mismatch');
    files.push({ path: relative, bytes: fs.statSync(to).size, sha256: hash(to) });
  };
  copy(reportFile, 'report.json');
  copy(path.join(probe, 'build-binding.json'), 'build-binding.json');
  copy(probeSource, 'PeakTelemetryProbe.cpp');
  copy(builder, 'build-probe.py');
  for (const name of ['PeakTelemetryProbe.exe', 'compile.log', 'link.log']) copy(path.join(probe, name), name);
  const images = PEAK_IMAGE_STEMS.flatMap(stem => [100, 200].map(scale => 'images/' + stem + '-' + scale + 'pct.png'));
  for (const name of images) copy(path.join(probe, 'run-001', path.basename(name)), name);
  const manifest = {
    schema: 'honestmix.peak-telemetry-evidence/1', version: report.version,
    createdUtc: new Date().toISOString(),
    originalProbeDirectory: probe,
    reportSha256: hash(reportFile), buildBindingSha256: hash(path.join(probe, 'build-binding.json')),
    sharedCodeSha256: binding.sharedCodeSha256, sourceBindings, images,
    files: files.sort((a, b) => a.path.localeCompare(b.path, 'en')),
    note: 'Exact copies of actual candidate report, build binding, probe source/executable, builder, logs and images. Raw build-binding.probeExecuted=false is retained because the builder never runs the probe. Runtime result comes from the separate actual report; no runtime exit code was invented.'
  };
  fs.writeFileSync(path.join(output, 'validation.json'), JSON.stringify(manifest, null, 2) + '\n', { encoding: 'utf8', flag: 'wx' });
  const validation = assertPeakTelemetry({ folder: output, version: report.version, sourceRoot: binding.source, liveBuild: true });
  console.log(JSON.stringify({ copiedActualEvidence: output, ...validation, newBuildOrAudioExecuted: false }));
} catch (error) {
  console.error(error.message);
  process.exitCode = 1;
}
