#!/usr/bin/env node
/* Candidate delivery. Version comes from CMake. No builds or tests are run here.
 * Requires four current native QA reports, actual peak telemetry/UI evidence,
 * and an explicit frozen hash manifest. Fresh owner-marked stages, UTF-8 ZIP flags
 * and no-overwrite rules remain mandatory. See README-BUILD.md for commands. */
import fs from 'node:fs';
import path from 'node:path';
import crypto from 'node:crypto';
import { execFileSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import { optionsFor, assertFrozen } from './tools/delivery-contract.mjs';
import { assertPeakTelemetry } from './tools/peak-telemetry-contract.mjs';

// ---------------------------------------------------------------------------
// 固定常量（工程身份 / 默认证据位置 / 依赖回退）
// ---------------------------------------------------------------------------

const PRODUCT_NAME = 'HONESTMIX Headphone';
const EXPECTED_BUNDLE_ID = 'cn.honestmix.headphone';
const EXPECTED_MANUFACTURER_CODE = 'HnMx';
const EXPECTED_PLUGIN_CODE = 'HmH6';

/** Current candidate evidence; explicit paths are accepted. */
const DEFAULT_EVIDENCE_DIR = path.join(path.dirname(fileURLToPath(import.meta.url)), 'evidence', 'final');
/** Fresh FidelityCheck runtime report, not a historical patch-integration report. */
const DEFAULT_FIDELITY_REPORT = path.join(DEFAULT_EVIDENCE_DIR, 'fidelity-check.json');

/** Portable fallback; configured development dependencies come from CMakeCache. */
const JUCE_FALLBACK = path.join(path.dirname(fileURLToPath(import.meta.url)), 'deps', 'juce');

/** Current UI dependencies, including header-only models. */
const REQUIRED_SKIN_FILES = ['Source/UI/Skin.h', 'Source/UI/Skin.cpp', 'Source/UI/TempoModel.h', 'Source/UI/ResponsePlotModel.h'];

/** Required current DSP and independent regression sources. */
const REQUIRED_FIDELITY_FILES = [
  'Source/PluginProcessor.h',
  'Source/PluginProcessor.cpp',
  'Source/dsp/CorrectionEngine.h',
  'Source/dsp/CorrectionEngine.cpp',
  'Source/dsp/WetDryMixer.h',
  'Source/dsp/WetDryMixer.cpp',
  'Source/dsp/IRConversion.h',
  'Source/dsp/CorrectionResponse.h',
  'tests/HeadphoneTests.cpp',
  'tests/FidelityCheck.cpp',
  'tests/ResponseContractTests.cpp',
  'tests/TempoRecallTests.cpp',
  'Source/UI/MeasuredHeadphone.h', 'Source/UI/MeasuredHeadphone.cpp', 'Source/UI/measured_data.h', 'Source/UI/HeadphonePrediction.h',
  'tests/PredictionContract.h', 'tests/PredictionUi.h', 'tests/MotionUi.h'
];

/** Exact screenshot names emitted by the current native UI suite. */
const FIXED_SCREENSHOTS = ['ui-search-empty.png', 'ui-tempo-host.png', 'ui-tempo-manual.png', 'ui-main-hover.png', 'ui-minimum.png',
  'ui-motion-away-100pct.png', 'ui-motion-away-200pct.png', 'ui-motion-hover-100pct.png', 'ui-motion-hover-200pct.png',
  'ui-motion-left-100pct.png', 'ui-motion-left-200pct.png', 'ui-motion-faded-100pct.png', 'ui-motion-faded-200pct.png'];
/** 4 档缩放截图：候选写 ui-100pct.png，0.6.0 写 ui-100.png —— 两种都接受，但必须四档齐全 */
const SCALE_SCREENSHOT_RE = /^ui-(100|125|150|200)(pct)?\.png$/;
const REQUIRED_SCALES = ['100', '125', '150', '200'];
/** 报告同目录里顺带进包的文本证据（存在就带，不存在不算失败） */
const OPTIONAL_SIBLING_FILES = ['native-suite.log', 'fidelity.log', 'response-contract.log', 'response-contract-first-failure.json', 'response-contract-first-failure.log', 'visual-preview-origin.json', 'verification.json'];

const SUB_REPORTS = ['firAndTranslation', 'stateAndAutomation', 'rateBlockMatrix', 'listenFilters', 'profileAutomation', 'vst3', 'ui'];
/**
 * 当前原生报告的附加子段（并入 required 列表，缺一个就失败 —— 这是**加严**不是放宽）：
 *   automationSmoothing  自动化平滑（tests/HeadphoneTests.cpp smoothingTests）
 *   bypass               干湿对齐 / 报告延迟 == 实测 bypass 延迟（保真修复的验收项）
 *   faderMapping         推子映射（顶200/中100/底0 + 键盘）
 *   axisAlignment        曲线与频率刻度对齐
 */
const EXTRA_SUB_REPORTS = ['automationSmoothing', 'bypass', 'faderMapping', 'axisAlignment', 'predictedHeadphoneUi', 'presentationMotion'];
const EXPECTED_SCALES = [1, 1.25, 1.5, 2];

// ---------------------------------------------------------------------------
// 命令行
// ---------------------------------------------------------------------------

const root = path.dirname(fileURLToPath(import.meta.url));
const defaultTmpRoot = path.join(root, 'tmp');
const releaseRoot = path.join(root, 'release');

const argv = process.argv.slice(2);
const has = name => argv.includes(name);
const value = (name, fallback) => {
  const i = argv.indexOf(name);
  return i >= 0 && i + 1 < argv.length && !argv[i + 1].startsWith('--') ? argv[i + 1] : fallback;
};

const PREFLIGHT = has('--preflight');
const KEEP_STAGE = has('--keep-stage');
const DISCARD_ARG = value('--discard-stage', null);
const MIN_FREE_MB = Number(value('--min-free-mb', '900'));
const SKIP_OPTIONAL_QA = has('--skip-optional-qa');
const FIDELITY_REPORT = path.resolve(value('--fidelity-report', DEFAULT_FIDELITY_REPORT));
const RESPONSE_REPORT = path.resolve(value('--response-report', path.join(DEFAULT_EVIDENCE_DIR, 'response-contract.json')));

/**
 * stage 落点。D 盘只剩约 3 GB，一次打包的 stage 峰值约 250 MB；
 * 空间不够时用 --stage-root 或环境变量 HM_STAGE_ROOT 把 stage 指到别的卷
 * （例如 C:\Users\Administrator\Documents\Codex\2026-10-16\c-3\work\pkg-stage-headphone）。
 * 发行包本体仍然写在 <工程>/release —— 固定的交付位置。
 * 跨卷时 fs.renameSync 会抛 EXDEV，脚本会自动降级为「复制 + 校验后删除源」。
 */
const stageRootArg = value('--stage-root', process.env.HM_STAGE_ROOT || null);
const tmpRoot = stageRootArg ? path.resolve(stageRootArg) : defaultTmpRoot;

// ---------------------------------------------------------------------------
// 基础工具
// ---------------------------------------------------------------------------

const log = (...a) => console.log(...a);
const step = t => log(`\n=== ${t} ===`);
const fail = message => { throw new Error(message); };
const sha256 = file => crypto.createHash('sha256').update(fs.readFileSync(file)).digest('hex');
const readText = file => fs.readFileSync(file, 'utf8');
const readJson = file => JSON.parse(readText(file));

/**
 * JUCE 写出的 JSON（moduleinfo.json、JSON::toString 的报告）允许对象/数组末尾
 * 带尾随逗号，不是严格 JSON。这里只在解析前去掉「逗号紧跟 } 或 ]」的情形。
 */
function readJuceJson(file, label) {
  const text = readText(file);
  const normalized = text.replace(/,\s*(?=[}\]])/g, '');
  if (normalized !== text) log(`  ! ${label}：含尾随逗号，已规范化后解析（JUCE 输出不是严格 JSON）`);
  return JSON.parse(normalized);
}

function writeJson(file, value) {
  fs.mkdirSync(path.dirname(file), { recursive: true });
  fs.writeFileSync(file, JSON.stringify(value, null, 2) + '\n', 'utf8');
}

/** 递归列出目录下所有普通文件（绝对路径，按路径排序，保证清单可复现） */
function allFiles(dir) {
  if (!fs.existsSync(dir)) return [];
  const out = [];
  const walk = current => {
    for (const entry of fs.readdirSync(current, { withFileTypes: true }).sort((a, b) => a.name.localeCompare(b.name))) {
      const full = path.join(current, entry.name);
      if (entry.isDirectory()) walk(full);
      else if (entry.isFile()) out.push(full);
    }
  };
  walk(dir);
  return out.sort();
}

const copyFile = (from, to) => { fs.mkdirSync(path.dirname(to), { recursive: true }); fs.copyFileSync(from, to); };
const copyTree = (from, to) => { for (const file of allFiles(from)) copyFile(file, path.join(to, path.relative(from, file))); };

function realOrFail(target, label) {
  try {
    return fs.realpathSync.native ? fs.realpathSync.native(target) : fs.realpathSync(target);
  } catch {
    fail(`${label} 不存在或不可解析：${target}`);
  }
}

/** child 是否严格位于 parent 之内（都先 realpath） */
function isInside(child, parent) {
  const c = realOrFail(child, '路径');
  const p = realOrFail(parent, '边界目录');
  if (c === p) return false;
  return c.startsWith(p + path.sep);
}

/** 从 parent 到 child 之间，任何一层是符号链接/junction 就拒绝 */
function assertNoLinks(child, parent) {
  const rel = path.relative(path.resolve(parent), path.resolve(child));
  if (rel.startsWith('..') || path.isAbsolute(rel)) fail(`路径不在 ${parent} 之内：${child}`);
  let current = path.resolve(parent);
  for (const part of rel.split(path.sep).filter(Boolean)) {
    current = path.join(current, part);
    if (!fs.existsSync(current)) break;
    if (fs.lstatSync(current).isSymbolicLink()) fail(`拒绝：路径含符号链接/junction —— ${current}`);
  }
  return path.resolve(child);
}

// ---------------------------------------------------------------------------
// 检查收集器：一次跑完全部检查，把所有失败一次报出来
// ---------------------------------------------------------------------------

const gates = [];
/**
 * @param {string} name 检查名
 * @param {() => any} body 检查体；抛异常 = 失败，返回值 = 结果（供后续使用），
 *                          返回的字符串会作为摘要打印
 */
function gate(name, body) {
  try {
    const result = body();
    const summary = typeof result === 'string' ? result : (result && result.summary) || '';
    gates.push({ name, ok: true, summary });
    log(`  ✓ ${name}${summary ? `  ${summary}` : ''}`);
    return result;
  } catch (error) {
    gates.push({ name, ok: false, message: error.message });
    log(`  ✗ ${name}`);
    for (const line of String(error.message).split('\n')) log(`      ${line}`);
    return undefined;
  }
}

function requireAll() {
  const bad = gates.filter(g => !g.ok);
  if (!bad.length) return;
  log(`\n=== ${bad.length} / ${gates.length} 项检查未通过，拒绝继续 ===`);
  for (const g of bad) log(`  ✗ ${g.name}\n      ${String(g.message).split('\n').join('\n      ')}`);
  throw new Error(`${bad.length} 项检查未通过`);
}

// ---------------------------------------------------------------------------
// Stage 生命周期（唯一允许删除的位置）
// ---------------------------------------------------------------------------

let runToken = null;
const newRunToken = () => crypto.randomBytes(16).toString('hex');

function newStageDir(version) {
  const stamp = new Date().toISOString().replace(/[-:T]/g, '').slice(0, 14);
  const rand = crypto.randomBytes(4).toString('hex');
  return path.join(tmpRoot, `stage-${version}-${stamp}-${rand}`);
}

function createStage(version) {
  runToken = newRunToken();
  assertStageRootSane();
  fs.mkdirSync(tmpRoot, { recursive: true });
  const stage = newStageDir(version);
  if (fs.existsSync(stage)) fail(`stage 目录已存在（不应发生）：${stage}`);
  fs.mkdirSync(stage, { recursive: false });
  writeJson(path.join(stage, 'stage-owner.json'), {
    token: runToken,
    pid: process.pid,
    createdAt: new Date().toISOString(),
    version,
    note: '本文件是本脚本删除该 stage 的唯一凭据；删除要求路径严格位于 <工程>/tmp 且 token 匹配。'
  });
  return stage;
}

/**
 * 删除 stage 的硬门槛（全部满足才删）：
 *   1. 目录存在
 *   2. realpath 严格位于当前 stage 根目录之内
 *   3. 目录名以 stage- 开头
 *   4. 从 stage 根到该目录的路径中不含符号链接/junction
 *   5. 目录内有 stage-owner.json，且 token 与调用方持有的 token 一致
 * 另：stage 根目录不得是工程根本身、也不得是它的祖先（避免误删工程内容）。
 */
function assertStageRootSane() {
  const rel = path.relative(path.resolve(tmpRoot), path.resolve(root));
  if (rel === '') fail(`拒绝：stage 根目录不能就是工程根（${tmpRoot}）。`);
  if (rel.startsWith('..') || path.isAbsolute(rel)) return; // stage 根在工程之外 —— 允许（--stage-root 的用途）
  fail(`拒绝：stage 根目录不能是工程根的祖先（${tmpRoot} 包含 ${root}）。`);
}

function removeStage(stage, token) {
  assertStageRootSane();
  if (!fs.existsSync(stage)) fail(`stage 不存在，无需删除：${stage}`);
  if (!isInside(stage, tmpRoot)) fail(`拒绝删除：${stage} 不在 stage 根 ${tmpRoot} 之内。`);
  const resolved = assertNoLinks(stage, tmpRoot);
  if (!path.basename(resolved).startsWith('stage-')) fail(`拒绝删除：${path.basename(resolved)} 不以 stage- 开头。`);
  const marker = path.join(resolved, 'stage-owner.json');
  if (!fs.existsSync(marker)) fail(`拒绝删除：缺少 stage-owner.json（不是本脚本创建的 stage）。`);
  const owner = readJson(marker);
  if (!token || owner.token !== token) {
    fail(`拒绝删除：owner token 不匹配（目录内 ${owner.token}，本次 ${token || '(未提供)'}）。`);
  }
  fs.rmSync(resolved, { recursive: true, force: false });
  log(`已删除 stage：${resolved}`);
}

/**
 * 把 stage 里的产物搬进 <工程>/release。
 * 同一个卷 → rename（原子、零额外空间）；跨卷 → 复制 + 校验 + 删除源。
 * 目录与单文件都要支持（包里既有目录也有 zip）。校验不通过时**绝不删除源**。
 */
function moveInto(from, to, token) {
  const isDir = fs.statSync(from).isDirectory();
  try {
    fs.renameSync(from, to);
    return 'rename';
  } catch (error) {
    if (error.code !== 'EXDEV') throw error;
    log(`  · stage 与 release 不同卷，改用复制：${path.basename(from)}`);
    fs.cpSync(from, to, { recursive: true, dereference: false, errorOnExist: true, force: false });
    if (isDir) {
      const srcRel = allFiles(from).map(f => path.relative(from, f)).sort();
      const dstRel = allFiles(to).map(f => path.relative(to, f)).sort();
      if (srcRel.length !== dstRel.length || srcRel.some((r, i) => r !== dstRel[i])) {
        fail(`跨卷复制后文件清单不一致（${srcRel.length} vs ${dstRel.length}），已保留源目录：${from}`);
      }
      for (const r of srcRel) {
        if (sha256(path.join(from, r)) !== sha256(path.join(to, r))) {
          fail(`跨卷复制校验失败（${r}），已保留源目录：${from}`);
        }
      }
      log(`    已校验 ${srcRel.length} 个文件，逐文件 SHA-256 一致`);
    } else {
      const a = sha256(from), b = sha256(to);
      if (a !== b) fail(`跨卷复制校验失败（${path.basename(from)}：${a.slice(0, 16)}… ≠ ${b.slice(0, 16)}…），已保留源文件：${from}`);
      log(`    已校验单文件 SHA-256：${a.slice(0, 16)}…`);
    }
    if (path.basename(from).startsWith('stage-')) removeStage(from, token);
    else fs.rmSync(from, { recursive: true, force: false });
    return 'copy-verified';
  }
}

// ---------------------------------------------------------------------------
// 版本 / PE 版本资源 / ZIP
// ---------------------------------------------------------------------------

/** 版本唯一来源：CMakeLists.txt 的 project(<name> VERSION x.y.z ...) */
function versionFromCmake(file = path.join(root, 'CMakeLists.txt')) {
  const text = readText(file);
  const project = /project\s*\(\s*[A-Za-z0-9_]+\s+VERSION\s+([0-9]+(?:\.[0-9]+){1,3})/i.exec(text);
  if (!project) fail(`无法从 ${file} 读取 project(... VERSION ...)`);
  return project[1];
}

function requireFresh(target, inputs, label) {
  if (!fs.existsSync(target)) fail(`${label} 不存在：${target}`);
  const t = fs.statSync(target).mtimeMs;
  const stale = inputs.filter(f => fs.existsSync(f) && fs.statSync(f).mtimeMs > t);
  if (stale.length) {
    fail(
      `${label} 过期：${path.relative(root, target)} 早于以下输入，请先重新构建/重跑测试：\n  - ` +
        stale.map(f => path.relative(root, f)).join('\n  - ')
    );
  }
}

function compileInputsFor(binary, sourceInputs) {
  // Test-only source edits do not rebuild unrelated targets in Ninja. Bind
  // each QA target to its own translation unit and all shared product sources.
  const common = sourceInputs.filter(file => {
    const relative = path.relative(paths.testsDir, file);
    const isTest = relative !== '' && !relative.startsWith('..') && !path.isAbsolute(relative);
    // CMake/build orchestration is bound by final hashes, cache and PE version.
    // Adding a test target can legitimately leave unrelated product binaries unchanged.
    return !isTest && file !== paths.buildCmd && file !== paths.cmake;
  });
  const tests = binary === paths.testsExe ? ['HeadphoneTests.cpp'] : binary === paths.fidelityExe ? ['FidelityCheck.cpp'] : binary === paths.responseExe ? ['ResponseContractTests.cpp'] : [];
  return [...common, ...tests.map(name => path.join(paths.testsDir, name))];
}

/** 用 PowerShell 读取 PE 版本资源（Windows 自带，无第三方依赖） */
function peVersion(file) {
  const escaped = file.replace(/'/g, "''");
  const script = `$v=(Get-Item -LiteralPath '${escaped}').VersionInfo; "$($v.FileVersion)|$($v.ProductVersion)"`;
  const out = execFileSync('powershell.exe', ['-NoProfile', '-NonInteractive', '-Command', script], {
    encoding: 'utf8',
    stdio: ['ignore', 'pipe', 'pipe']
  }).trim();
  const [fileVersion, productVersion] = out.split('|');
  return { fileVersion, productVersion };
}

function sevenZipPath() {
  return process.env.HM_7Z || path.join(process.env.ProgramFiles || 'C:/Program Files', '7-Zip', '7z.exe');
}

/**
 * 中文条目必须带 UTF-8 标志（general purpose bit 11 / 0x0800）。
 * Windows 的 tar.exe 写出的名字是 UTF-8 字节但**不置位**该标志，
 * 于是别的解压器按 CP936 解读 → 乱码。7-Zip 的 -mcu=on 会置位。
 */
function zipCreate(zipFile, entries, cwd) {
  const seven = sevenZipPath();
  if (!fs.existsSync(seven)) fail(`打包需要 7-Zip（正确写中文 ZIP 名）。请安装或设置 HM_7Z。当前：${seven}`);
  execFileSync(seven, ['a', '-tzip', '-mcu=on', '-y', zipFile, ...entries], { cwd, stdio: 'pipe' });
  if (!fs.existsSync(zipFile)) fail(`7-Zip 未产出 zip：${zipFile}`);
}

/** 读 ZIP 中央目录，返回 {name, utf8} 列表（只读） */
function zipCentralDirectory(zipFile) {
  const buf = fs.readFileSync(zipFile);
  let eocd = -1;
  const lowest = Math.max(0, buf.length - 22 - 0xffff);
  for (let i = buf.length - 22; i >= lowest; i--) {
    if (buf.readUInt32LE(i) === 0x06054b50) { eocd = i; break; }
  }
  if (eocd < 0) fail(`不是可解析的 ZIP（找不到 EOCD）：${zipFile}`);
  const count = buf.readUInt16LE(eocd + 10);
  let p = buf.readUInt32LE(eocd + 16);
  const out = [];
  for (let i = 0; i < count; i++) {
    if (buf.readUInt32LE(p) !== 0x02014b50) fail(`ZIP 中央目录头损坏 @${p}：${zipFile}`);
    const flags = buf.readUInt16LE(p + 8);
    const nameLen = buf.readUInt16LE(p + 28);
    const extraLen = buf.readUInt16LE(p + 30);
    const commentLen = buf.readUInt16LE(p + 32);
    const raw = buf.subarray(p + 46, p + 46 + nameLen);
    out.push({ flags, utf8: (flags & 0x0800) !== 0, raw, name: raw.toString('utf8') });
    p += 46 + nameLen + extraLen + commentLen;
  }
  return out;
}

/** 校验：凡名字含非 ASCII 字节的条目，UTF-8 标志必须置位 */
function assertZipUtf8(zipFile) {
  const entries = zipCentralDirectory(zipFile);
  const nonAscii = entries.filter(e => e.raw.some(b => b > 0x7f));
  const bad = nonAscii.filter(e => !e.utf8);
  if (bad.length) {
    fail(
      `ZIP 中文条目缺少 UTF-8 标志（0x0800）${bad.length} 个，解压方会乱码：\n  - ` +
        bad.map(e => e.name).join('\n  - ')
    );
  }
  if (!nonAscii.length) fail('ZIP 内没有任何非 ASCII 名字的条目 —— 中文 README 丢了，拒绝交付。');
  return { entries: entries.length, nonAscii: nonAscii.length, names: nonAscii.map(e => e.name) };
}

function freeBytes(dir) {
  const st = fs.statfsSync(dir);
  return Number(st.bavail) * Number(st.bsize);
}

// ---------------------------------------------------------------------------
// 工程内固定路径
// ---------------------------------------------------------------------------

const version = versionFromCmake();
const packageName = `HONESTMIX-Headphone-${version}`;

const paths = {
  cmake: path.join(root, 'CMakeLists.txt'),
  buildCmd: path.join(root, 'build.cmd'),
  sourceDir: path.join(root, 'Source'),
  testsDir: path.join(root, 'tests'),
  testSource: path.join(root, 'tests', 'HeadphoneTests.cpp'),
  licensesDir: path.join(root, 'licenses'),
  readmeUser: path.join(root, 'README-先看这里.md'),
  readmeBuild: path.join(root, 'README-BUILD.md'),
  self: fileURLToPath(import.meta.url),
  exe: path.join(root, 'build', 'HonestMixHeadphone_artefacts', 'Release', 'Standalone', `${PRODUCT_NAME}.exe`),
  vst3Bundle: path.join(root, 'build', 'HonestMixHeadphone_artefacts', 'Release', 'VST3', `${PRODUCT_NAME}.vst3`),
  vst3Binary: path.join(root, 'build', 'HonestMixHeadphone_artefacts', 'Release', 'VST3', `${PRODUCT_NAME}.vst3`, 'Contents', 'x86_64-win', `${PRODUCT_NAME}.vst3`),
  vst3ModuleInfo: path.join(root, 'build', 'HonestMixHeadphone_artefacts', 'Release', 'VST3', `${PRODUCT_NAME}.vst3`, 'Contents', 'Resources', 'moduleinfo.json'),
  testsExe: path.join(root, 'build', 'HeadphoneTests_artefacts', 'Release', 'HeadphoneTests.exe'),
  fidelityExe: path.join(root, 'build', 'FidelityCheck_artefacts', 'Release', 'FidelityCheck.exe'),
  responseExe: path.join(root, 'build', 'ResponseContractTests_artefacts', 'Release', 'ResponseContractTests.exe'),
  tempoExe: path.join(root, 'build', 'TempoRecallTests_artefacts', 'Release', 'TempoRecallTests.exe')
};

const reportPath = path.resolve(value('--report', path.join(DEFAULT_EVIDENCE_DIR, 'headphone-tests.json')));
const evidenceDir = path.resolve(value('--evidence-dir', path.dirname(reportPath)));
const TEMPO_REPORT = path.resolve(value('--tempo-report', path.join(evidenceDir,'tempo-recall','result.json')));
const evidenceRoot = path.join(root, 'evidence');

// ---------------------------------------------------------------------------
// 检查 1：工程身份 / 版本 / 源码构成
// ---------------------------------------------------------------------------

const cmakeText = () => readText(paths.cmake);

function checkProjectIdentity() {
  step('检查工程身份与版本来源');
  gate('版本来自 CMakeLists.txt', () => {
    if (!/^\d+\.\d+\.\d+$/.test(version)) fail(`版本号形态异常：${version}`);
    return `version = ${version}`;
  });
  gate('插件身份未变（BUNDLE_ID / 厂商码 / 插件码 / 产品名）', () => {
    const text = cmakeText();
    const problems = [];
    if (!text.includes(`BUNDLE_ID ${EXPECTED_BUNDLE_ID}`)) problems.push(`BUNDLE_ID 不是 ${EXPECTED_BUNDLE_ID}`);
    if (!new RegExp(`PLUGIN_MANUFACTURER_CODE\\s+${EXPECTED_MANUFACTURER_CODE}\\b`).test(text)) problems.push(`PLUGIN_MANUFACTURER_CODE 不是 ${EXPECTED_MANUFACTURER_CODE}`);
    if (!new RegExp(`PLUGIN_CODE\\s+${EXPECTED_PLUGIN_CODE}\\b`).test(text)) problems.push(`PLUGIN_CODE 不是 ${EXPECTED_PLUGIN_CODE}`);
    if (!text.includes(`PRODUCT_NAME "${PRODUCT_NAME}"`)) problems.push(`PRODUCT_NAME 不是 "${PRODUCT_NAME}"`);
    if (problems.length) fail(`插件身份被改动，老工程会话会失效：\n  - ${problems.join('\n  - ')}`);
    return `${EXPECTED_BUNDLE_ID} / ${EXPECTED_MANUFACTURER_CODE} / ${EXPECTED_PLUGIN_CODE}`;
  });
  gate('新 UI 皮肤在源码里且在 target_sources 里', () => {
    const text = cmakeText();
    const missing = REQUIRED_SKIN_FILES.filter(f => !fs.existsSync(path.join(root, f)));
    if (missing.length) fail(`当前 UI 文件缺失：\n  - ${missing.join('\n  - ')}`);
    // 只有编译单元（.cpp）才该出现在 target_sources 里
    const units = REQUIRED_SKIN_FILES.filter(f => f.endsWith('.cpp'));
    const notCompiled = units.filter(f => !text.includes(f));
    if (notCompiled.length) fail(`新皮肤编译单元没进 target_sources，编出来的还是旧皮肤：\n  - ${notCompiled.join('\n  - ')}`);
    if (fs.statSync(path.join(root, 'licenses/GPL-3.0.txt')).size < 30000) fail('Full GPL-3.0 text is missing');
    return REQUIRED_SKIN_FILES.map(f => `${f}(${fs.statSync(path.join(root, f)).size} B)`).join('、');
  });
  gate('当前 DSP/UI 与独立测试源码在场', () => {
    const missing = REQUIRED_FIDELITY_FILES.filter(f => !fs.existsSync(path.join(root, f)));
    if (missing.length) fail(`当前交付所需源码缺失：\n  - ${missing.join('\n  - ')}`);
    return `${REQUIRED_FIDELITY_FILES.length} 个`;
  });

}

// ---------------------------------------------------------------------------
// 检查 2：产物齐全 + 与源码同版本
// ---------------------------------------------------------------------------

function checkArtifacts() {
  step('检查产物与版本一致性');
  gate('五个二进制与 VST3 元数据齐全', () => {
    for (const [label, file] of [
      ['Standalone EXE', paths.exe],
      ['VST3 主二进制', paths.vst3Binary],
      ['VST3 moduleinfo.json', paths.vst3ModuleInfo],
      ['测试程序', paths.testsExe],
      ['FidelityCheck', paths.fidelityExe],
      ['ResponseContractTests', paths.responseExe]
    ]) {
      if (!fs.existsSync(file)) fail(`${label} 不存在：${file}\n请先运行 build.cmd（不要用本脚本代跑构建）。`);
    }
    return '6 项';
  });
  gate('VST3 moduleinfo.json 版本 = 源码版本', () => {
    const moduleInfo = readJuceJson(paths.vst3ModuleInfo, 'moduleinfo.json');
    if (moduleInfo.Version !== version) fail(`VST3 moduleinfo.json 版本 ${moduleInfo.Version} ≠ CMake 版本 ${version}`);
    return `Version = ${moduleInfo.Version}`;
  });
  gate('五个二进制的 PE 版本资源 = 源码版本', () => {
    const bad = [];
    for (const [label, file] of [
      ['Standalone EXE', paths.exe],
      ['VST3 主二进制', paths.vst3Binary],
      ['测试程序', paths.testsExe],
      ['FidelityCheck', paths.fidelityExe],
      ['ResponseContractTests', paths.responseExe]
    ]) {
      const { fileVersion, productVersion } = peVersion(file);
      if (fileVersion !== version || productVersion !== version) {
        bad.push(`${label}：File=${fileVersion} Product=${productVersion}`);
      }
    }
    if (bad.length) {
      fail(
        `${bad.join('\n')}\n` +
          `  与源码版本 ${version} 不一致。\n` +
          `  版本资源过期，请由构建负责人修复资源依赖并重新构建全部目标。`
      );
    }
    return `File/Product = ${version}`;
  });
}

// ---------------------------------------------------------------------------
// 检查 3：构建比源码新
// ---------------------------------------------------------------------------

function checkBuildFreshness() {
  step('检查构建新鲜度（二进制必须晚于全部源码输入）');
  const inputs = [...allFiles(paths.sourceDir), ...allFiles(paths.testsDir), paths.cmake, paths.buildCmd];
  gate(`五个二进制晚于各自编译依赖（共 ${inputs.length} 个候选输入）`, () => {
    for (const [label, file] of [
      ['Standalone EXE', paths.exe],
      ['VST3 主二进制', paths.vst3Binary],
      ['测试程序', paths.testsExe],
      ['FidelityCheck', paths.fidelityExe],
      ['ResponseContractTests', paths.responseExe]
    ]) {
      requireFresh(file, compileInputsFor(file, inputs), label);
    }
    return `${inputs.length} 个输入`;
  });
  return inputs;
}

// ---------------------------------------------------------------------------
// 检查 3b：必需 QA 二进制（FidelityCheck 与 ResponseContractTests）
// ---------------------------------------------------------------------------

function checkOptionalQa(sourceInputs) {
  gate('必需 QA 二进制不能跳过', () => { if (SKIP_OPTIONAL_QA) fail('--skip-optional-qa is not supported: FidelityCheck and ResponseContractTests are required'); });
  return { files: [paths.fidelityExe, paths.responseExe, paths.tempoExe].map(file => ({ path: file, name: path.basename(file) })), skipped: false };
}

function checkCurrentContracts(sourceInputs) {
  gate('当前 BPM 页面记忆与宿主生命周期回归', () => {
    const report=readJuceJson(TEMPO_REPORT,'TempoRecallTests');
    const expected=['manual-precision-survives-editor-reopen','feel-survives-editor-reopen','manual-and-feel-survive-binary-project-roundtrip','explicit-host-source-survives-without-host','open-editor-syncs-project-recall','hostile-preferences-use-safe-defaults-without-changing-audio-parameters','old-project-without-tempo-properties-remains-compatible','saved-explicit-manual-is-not-hijacked-by-new-host','host-source-uses-full-fractional-value','stale-host-never-silently-displays-manual-calculation','host-resumes-after-fresh-callback','missing-host-stays-explicit-unavailable','manual-value-and-feel-survive-host-loss-roundtrip','manual-settings-survive-audio-release'];
    if(report.passed!==true||report.version!==version||report.hardwareAudioOpened!==false||report.thirdPartyDaw!==false)fail('Tempo recall version/scope/pass gate failed');
    if(!Array.isArray(report.checks)||report.checks.length!==expected.length||report.checks.some(row=>row.passed!==true)||new Set(report.checks.map(row=>row.name)).size!==expected.length||expected.some(name=>!report.checks.some(row=>row.name===name&&row.passed===true)))fail('Missing, duplicated or failed tempo recall checks');
    requireFresh(TEMPO_REPORT,[...compileInputsFor(paths.tempoExe,sourceInputs),paths.tempoExe],'TempoRecallTests report');
    return '14 exact non-hardware BPM state/source/host-lifecycle checks passed';
  });
  gate('当前 FidelityCheck 绝对幅相报告', () => {
    const report = readJuceJson(FIDELITY_REPORT, 'FidelityCheck');
    if (report.passed !== true || report.mode !== 'patched hard-gate validation' || report.hardwareAudioOpened !== false) fail('FidelityCheck must be the current hard-gate offline run');
    const provenanceFile=path.join(evidenceDir,'profile-generation.json');
    const provenance=JSON.parse(fs.readFileSync(provenanceFile,'utf8'));
    if(provenance.passed!==true||provenance.exactRegenerationPassed!==true||provenance.version!==version||provenance.inputCount!==1573||provenance.correctedProfileIndex!==1572||provenance.correctedPadInputSha256!=='3c7af9258478eac571409f33adad6bce2b44343996d9c2782fc407b3cbdad04b'||provenance.headerSha256!==sha256(path.join(root,'Source/dsp/fir_data.h'))||provenance.manifestSha256!==sha256(path.join(root,'tools/profile-generation/provenance.json'))||provenance.generatorSha256!==sha256(path.join(root,'tools/profile-generation/regenerate.cjs')))fail('Pinned input regeneration or pad provenance proof missing/stale');
    if(provenance.rawMeasurementsExact!==true||provenance.measurementPointsPerProfile!==695||provenance.measurementHeaderSha256!==sha256(path.join(root,'Source/UI/measured_data.h'))||provenance.measurementGeneratorSha256!==sha256(path.join(root,'tools/profile-generation/generate-measurement.cjs')))fail('Current exact raw measurement data proof missing or stale');
    requireFresh(provenanceFile,[path.join(root,'Source/dsp/fir_data.h'),path.join(root,'Source/UI/measured_data.h'),...allFiles(path.join(root,'tools/profile-generation'))],'Pinned profile generation proof');
    if (report.rows?.length !== 28 || report.rows.some(row => row.passed !== true)) fail('Expected all 7 profiles × 4 rates including corrected K712 pad in FidelityCheck');
    const combinations = new Set(report.rows.map(row => `${row.index}/${row.rate}`));
    for (const profile of [0, 1, 924, 802, 730, 756, 1572]) for (const rate of [44100, 48000, 96000, 192000]) if (!combinations.has(`${profile}/${rate}`)) fail('Missing fidelity profile/rate combination');
    for (const key of ['maxAbsoluteIrDbError','maxAbsoluteSineDbError']) if (!(Number.isFinite(report[key]) && report[key] < .01)) fail(key + ' exceeds the absolute 0.01dB gate');
    if (!(Number.isFinite(report.maxSinePhaseErrorRadians) && report.maxSinePhaseErrorRadians < .001)) fail('Fidelity phase gate failed');
    requireFresh(FIDELITY_REPORT, [...compileInputsFor(paths.fidelityExe, sourceInputs), paths.fidelityExe], 'FidelityCheck report');
    return '28 rows; original6 plus corrected pad absolute amplitude/phase gates passed';
  });
  gate('当前 ResponseContractTests 报告', () => {
    const report = readJuceJson(RESPONSE_REPORT, 'ResponseContractTests');
    if (report.passed !== true || report.version !== version || report.hardwareAudioOpened !== false || report.audioCppNews !== 0) fail('Response contract version/scope/allocation gate failed');
    for (const key of ['parametersAndTempo','actualProfileSwitch','failedLoadRetry','monitor','bypassSnapshot','response']) if (report[key]?.passed !== true) fail('Missing/pending response contract: ' + key);
    for(const rate of [44100,48000,96000,192000]) if(!report.response.rows?.some(row=>row.profile===1572&&row.sampleRate===rate&&row.degrees===9&&row.passed===true)) fail('Missing corrected pad full0-200 response/audio row');
    requireFresh(RESPONSE_REPORT, [...compileInputsFor(paths.responseExe, sourceInputs), paths.responseExe], 'ResponseContractTests report');
    return 'response, actual switches, failed-load recovery, BPM and monitor contracts passed';
  });
}

// ---------------------------------------------------------------------------
// 检查 4：测试报告真的成功（门槛逐条对应候选 tests/HeadphoneTests.cpp）
// ---------------------------------------------------------------------------

const sameNumberArray = (a, b) => Array.isArray(a) && a.length === b.length && a.every((v, i) => Number(v) === b[i]);

/** 从报告里找出任何形如 sha256 的字符串 */
function collectHexDigests(value, out = []) {
  if (typeof value === 'string') {
    if (/^[a-f0-9]{64}$/i.test(value)) out.push(value.toLowerCase());
  } else if (Array.isArray(value)) {
    for (const item of value) collectHexDigests(item, out);
  } else if (value && typeof value === 'object') {
    for (const item of Object.values(value)) collectHexDigests(item, out);
  }
  return out;
}

function checkReport(sourceInputs) {
  step('检查测试报告（门槛来自候选 tests/HeadphoneTests.cpp 的实际实现）');
  log(`  报告：${reportPath}`);
  log(`  截图目录：${evidenceDir}`);

  const report = gate('报告存在且可在证据目录找到全部截图', () => {
    if (!fs.existsSync(reportPath)) {
      fail(
        `找不到测试报告：${reportPath}\n` +
          `  请先运行（两个参数缺一不可）：\n` +
          `    build\\HeadphoneTests_artefacts\\Release\\HeadphoneTests.exe "<证据目录>" "build\\HonestMixHeadphone_artefacts\\Release\\VST3\\${PRODUCT_NAME}.vst3"\n` +
          `  第 1 个参数（证据目录）决定 passed、ui 段与截图；第 2 个参数（VST3 路径）决定 vst3 段。\n` +
          `  当前工程 evidence/final 是默认目录，也可用 --report / --evidence-dir 指定。`
      );
    }
    if (!fs.existsSync(evidenceDir) || !fs.statSync(evidenceDir).isDirectory()) {
      fail(`截图目录不存在：${evidenceDir}（用 --evidence-dir 指定）`);
    }
    const shots = allFiles(evidenceDir).map(f => path.basename(f));
    const missing = FIXED_SCREENSHOTS.filter(name => !shots.includes(name));
    // 4 档缩放：逐档找，缺哪档报哪档
    const byScale = new Map();
    for (const name of shots) {
      const m = SCALE_SCREENSHOT_RE.exec(name);
      if (m && !byScale.has(m[1])) byScale.set(m[1], name);
    }
    for (const scale of REQUIRED_SCALES) if (!byScale.has(scale)) missing.push(`ui-${scale}[pct].png`);
    if (missing.length) fail(`证据目录缺少截图：${missing.map(m => `\n  - ${m}`).join('')}\n  实际目录：${evidenceDir}`);
    const expected = [...FIXED_SCREENSHOTS, ...REQUIRED_SCALES.map(s => byScale.get(s))];
    for (const name of expected) {
      const file = path.join(evidenceDir, name);
      if (fs.statSync(file).size === 0) fail(`截图是 0 字节：${file}`);
    }
    log(`  ✓ 固定名截图 ${FIXED_SCREENSHOTS.length} 张 + 缩放截图 ${REQUIRED_SCALES.map(s => byScale.get(s)).join(', ')}`);
    log('  ✓ 报告与截图均在证据目录');
    return { report: readJuceJson(reportPath, '测试报告'), expected };
  });
  if (!report) return { report: undefined, screenshots: [] };

  gate('报告自报 passed / version', () => {
    if (report.report.uiPreviewOnly === true) fail('A UI preview is not the full native/DSP/VST3 acceptance report');
    if (report.report.passed !== true) {
      fail(`报告 passed 不是 true（实际：${JSON.stringify(report.report.passed)}）。` + (report.report.error ? `\n  失败原文：${report.report.error}` : ''));
    }
    if (report.report.version !== version) fail(`报告自报版本 ${report.report.version} ≠ CMake 版本 ${version}。`);
    return `passed=true、version=${report.report.version}`;
  });

  gate(`${SUB_REPORTS.length + EXTRA_SUB_REPORTS.length} 个子段存在且 passed`, () => {
    const all = [...SUB_REPORTS, ...EXTRA_SUB_REPORTS];
    const missing = all.filter(name => !report.report[name] || typeof report.report[name] !== 'object');
    if (missing.length) {
      fail(
        `报告缺少子段：${missing.join(', ')}\n` +
          `  ui 段需要第 1 个参数（证据目录）；vst3 段需要第 2 个参数（VST3 路径）——两者都要给。`
      );
    }
    const predictedUi=report.report.predictedHeadphoneUi;
    if(predictedUi?.passed!==true||predictedUi.cases?.length!==5||predictedUi.actualNativeRenders!==10||predictedUi.zeroRawCurveHeightLogicalPixels<=10||predictedUi.unmeasuredHoverSuppressed!==true||predictedUi.legacyGraphSuppressed!==true||predictedUi.bypassShowsOriginal!==true||predictedUi.inactiveSuppressed!==true)fail('Native predicted-response UI evidence missing');
    const failed = all.filter(name => report.report[name].passed !== true);
    if (failed.length) fail(`子段 passed 不是 true：${failed.join(', ')}`);
    if (report.report.stateAndAutomation.legacyDegreeCases !== 27 || report.report.stateAndAutomation.modernParameterAuthoritative !== true || report.report.faderMapping.ignoredNonPrimaryInputs !== 3 || report.report.faderMapping.disabledInputsIgnored !== true || report.report.faderMapping.duplicatePressSafe !== true) fail('Missing current legacy-state/input-boundary regression proof');
    return all.join(', ');
  });

  gate('Measured headphone prediction raw/DSP/audio contract', () => {
    const result=JSON.parse(fs.readFileSync(RESPONSE_REPORT,'utf8')).headphonePrediction;
    if(!result||result.passed!==true||result.measurementProfiles!==1573||result.measurementKnotsPerProfile!==695||result.rows?.length!==28||result.actualSineCases!==1260||result.zeroMeansOriginalMeasurement!==true||result.fixedOriginal1kReferenceAtAllDegrees!==true||result.noForcedFlatOrInventedResidual!==true||result.bypassMeansOriginalMeasurement!==true||result.noExtrapolation!==true||result.predictedGraphDbError>.01||result.actualSinePredictedDbError>.02)fail('Predicted headphone/slider/audio contract missing or inaccurate');
    return 'Pinned raw measurement + actual complex DSP,28 rows x9 degrees,1260 real sines';
  });

  gate('vst3 子段的插件身份与版本与源码一致', () => {
    const vst3 = report.report.vst3;
    if (vst3.name !== PRODUCT_NAME) fail(`vst3.name = ${JSON.stringify(vst3.name)}，期望 "${PRODUCT_NAME}"。`);
    if (vst3.version !== version) fail(`vst3.version = ${JSON.stringify(vst3.version)}，与源码版本 ${version} 不一致。`);
    return `name=${vst3.name}、version=${vst3.version}、maximumImpulseError=${vst3.maximumImpulseError}`;
  });

  gate('ui 子段：四档缩放 + 当前内嵌页面交互', () => {
    const ui = report.report.ui;
    if (!sameNumberArray(ui.actualResizeScales, EXPECTED_SCALES)) {
      fail(`ui.actualResizeScales = ${JSON.stringify(ui.actualResizeScales)}，必须是 ${JSON.stringify(EXPECTED_SCALES)}。`);
    }
    for (const field of ['searchNoMatch', 'profileSelection', 'compareRestore', 'embeddedTempoInteractive', 'hostTempoLiveDouble', 'manualInvalidRejected', 'bilingualActionSubtitles', 'advancedLegacyModes', 'offlineGuidePureExport']) {
      if (ui[field] !== true) fail(`ui.${field} 必须是 true（实际 ${JSON.stringify(ui[field])}）。`);
    }
    if(ui.mainFooterActions!==2||ui.minimumWidth!==900||ui.minimumHeight!==432)fail('Current two-action bilingual footer/minimum layout assertion missing');
    return `缩放 ${JSON.stringify(ui.actualResizeScales)}、搜索/选择/对比/内嵌 TEMPO/小数宿主速度/非法手动输入 全为 true`;
  });

  gate('无真实音频设备 / 无公网 / 回调 C++ new 计数为零', () => {
    if (report.report.hardwareAudioOpened !== false || report.report.publicNetworkUsed !== false) {
      fail('报告自报打开了真实音频设备或使用了公网；本交付不接受这种运行。');
    }
    if (report.report.callbackCppNewCount !== 0) fail(`回调线程 C++ new 计数为 ${report.report.callbackCppNewCount}，要求 0。`);
    return 'hardwareAudioOpened=false、publicNetworkUsed=false、callbackCppNewCount=0';
  });

  gate('translation 0/100/200 与 rate×block 矩阵逐条 passed', () => {
    const cases = report.report.firAndTranslation.translationCases;
    if (!Array.isArray(cases) || cases.length < 3 || cases.some(c => c.passed !== true)) {
      fail('firAndTranslation.translationCases 必须至少 3 条且全部 passed（对应 translation 0/100/200）。');
    }
    const matrix = report.report.rateBlockMatrix.cases;
    if (!Array.isArray(matrix) || matrix.length === 0 || matrix.some(c => c.passed !== true)) {
      fail('rateBlockMatrix.cases 不能为空且必须全部 passed。');
    }
    return `translationCases=${cases.length} 条、rateBlockMatrix=${matrix.length} 条`;
  });

  gate('bypass 各采样率 reported == measured（保真修复验收项）', () => {
    const cases = report.report.bypass?.cases;
    if (!Array.isArray(cases) || !cases.length) fail('报告 bypass.cases 缺失或为空。');
    const bad = cases.filter(c => c.passed !== true || Number(c.reportedLatency) !== Number(c.measuredBypassLatency));
    if (bad.length) {
      fail(
        'bypass 未通过或报告延迟 ≠ 实测 bypass 延迟：\n  - ' +
          bad.map(c => `${c.rate}Hz: reported=${c.reportedLatency} measured=${c.measuredBypassLatency} passed=${c.passed}`).join('\n  - ')
      );
    }
    return cases.map(c => `${c.rate}→${c.reportedLatency}/${c.measuredBypassLatency}`).join('、');
  });

  gate('原生报告晚于 EXE/VST3/HeadphoneTests 及其源码', () => {
    const inputs = [...new Set([paths.exe, paths.vst3Binary, paths.testsExe, ...compileInputsFor(paths.testsExe, sourceInputs)])];
    requireFresh(reportPath, inputs, '测试报告');
    return `晚于 ${inputs.length} 个输入`;
  });

  gate('报告与二进制哈希绑定', () => {
    const hashes = { exe: sha256(paths.exe), vst3: sha256(paths.vst3Binary), tests: sha256(paths.testsExe) };
    const digests = collectHexDigests(report.report);
    if (!digests.length) {
      return '报告未含哈希字段 → 采用 mtime 绑定 + 打包时独立计算并写入 delivery.json';
    }
    const known = new Set(Object.values(hashes));
    const unknown = digests.filter(d => !known.has(d));
    if (unknown.length) fail(`报告里出现了无法对应到当前五个二进制的哈希：${unknown.join(', ')}\n  说明报告与当前构建不是同一次产出。`);
    return `报告中 ${digests.length} 个哈希均与当前二进制一致`;
  });

  gate('当前VST3已通过独立pluginval等级5', () => {
    const folder=path.join(evidenceDir,'external-vst3');const external=readJson(path.join(folder,'validation.json'));
    if(external.passed!==true||external.version!==version||external.pluginBinarySha256!==sha256(paths.vst3Binary)||external.settings?.strictnessLevel!==5||external.run?.exitCode!==0||external.run?.watchdogTriggered!==false)fail('Missing current external validator level-5 evidence or mismatched VST3');
    for(const file of external.files??[]){if(path.basename(file.path)!==file.path||sha256(path.join(folder,file.path))!==file.sha256)fail('External validator log hash mismatch');}
    return 'Exact VST3 SHA and complete validator logs bound; external DAW/hardware scope remains separate';
  });

  return { report: report.report, screenshots: report.expected };
}

// ---------------------------------------------------------------------------
// 检查 5：JUCE 依赖
// ---------------------------------------------------------------------------

function resolveJuce() {
  step('解析 JUCE 依赖');
  return gate('JUCE 源码目录可用', () => {
    const cache = path.join(root, 'build', 'CMakeCache.txt');
    let juce = value('--juce', null);
    if (!juce && fs.existsSync(cache)) {
      const hit = /^HM_JUCE:PATH=(.+)$/m.exec(readText(cache));
      if (hit) juce = hit[1].trim();
    }
    if (!juce) {
      const bundled = path.join(root, 'deps', 'juce');
      juce = fs.existsSync(path.join(bundled, 'CMakeLists.txt')) ? bundled : JUCE_FALLBACK;
    }
    juce = path.resolve(juce.replace(/\//g, path.sep));
    if (!fs.existsSync(path.join(juce, 'CMakeLists.txt'))) {
      fail(`HM_JUCE 指向的位置没有 JUCE 源码：${juce}\n  CMakeLists.txt:11-13 要求该目录存在 CMakeLists.txt，且不联网下载。`);
    }
    for (const rel of ['CMakeLists.txt','LICENSE.md','modules/juce_core/juce_core.h','extras/Build/CMake/JUCEUtils.cmake']) if (!fs.existsSync(path.join(juce, rel))) fail('Incomplete JUCE dependency: ' + rel);
    const files = allFiles(juce);
    return { dir: juce, files };
  });
}

// ---------------------------------------------------------------------------
// 检查 6：磁盘余量（分别检查当前 stage 与 release 卷）
// ---------------------------------------------------------------------------

/** 一次打包的估算占用（MB）：stage 峰值 + release 常驻
 *  实测依据（0.6.0 包，2026-10-16）：
 *    stage/source-tree 88.66 MB、包目录约 76.5 MB、外层 zip 68.7 MB → stage 峰值 ≈ 234 MB
 *    release 常驻 = 包目录 76.5 + zip 68.7 ≈ 145 MB
 *  这里各留约 10% 余量。 */
const ESTIMATE = { stageMb: 255, releaseMb: 160 };

function nearestExisting(dir) {
  let current = path.resolve(dir);
  while (!fs.existsSync(current)) {
    const parent = path.dirname(current);
    if (parent === current) break;
    current = parent;
  }
  return current;
}

function checkDisk() {
  step('检查磁盘余量');
  log(`  stage 根：${tmpRoot}${tmpRoot === defaultTmpRoot ? '（工程内默认）' : '（--stage-root / HM_STAGE_ROOT 指定）'}`);
  log(`  发行包落点：${releaseRoot}`);
  gate(`stage 卷可用空间 >= ${MIN_FREE_MB} MB`, () => {
    const freeMb = freeBytes(nearestExisting(tmpRoot)) / 1048576;
    if (freeMb < MIN_FREE_MB) {
      fail(
        `stage 卷（${nearestExisting(tmpRoot)}）只有 ${freeMb.toFixed(0)} MB，低于门槛 ${MIN_FREE_MB} MB。\n` +
          `  估算 stage 峰值约 ${ESTIMATE.stageMb} MB（source-tree 含 JUCE + 包目录 + 外层 zip）。\n` +
          `  D 盘紧张时用 --stage-root <目录> 或 HM_STAGE_ROOT 把 stage 指到别的卷，例如：\n` +
          `    $env:HM_STAGE_ROOT='C:\\Users\\Administrator\\Documents\\Codex\\2026-10-16\\c-3\\work\\pkg-stage-headphone'\n` +
          `  本脚本不清理磁盘；不要为了让检查通过而降低门槛。`
      );
    }
    return `${freeMb.toFixed(0)} MB 可用`;
  });
  return gate(`release 卷可用空间 >= ${MIN_FREE_MB} MB`, () => {
    const freeMb = freeBytes(nearestExisting(releaseRoot)) / 1048576;
    if (freeMb < MIN_FREE_MB) {
      fail(
        `release 卷（${nearestExisting(releaseRoot)}）只有 ${freeMb.toFixed(0)} MB，低于门槛 ${MIN_FREE_MB} MB。\n` +
          `  估算 release 常驻约 ${ESTIMATE.releaseMb} MB（包目录 + 外层 zip）。本脚本不清理磁盘。`
      );
    }
    return `${freeMb.toFixed(0)} MB 可用`;
  });
}

// ---------------------------------------------------------------------------
// 打包
// ---------------------------------------------------------------------------

function buildPackageTree(stage, expectedShots, optionalQa) {
  step('组装发行包目录');
  const pkg = path.join(stage, packageName);
  if (fs.existsSync(pkg)) fail(`stage 内已存在 ${packageName}（不应发生）：${pkg}`);
  fs.mkdirSync(pkg, { recursive: true });

  copyFile(paths.exe, path.join(pkg, 'client', path.basename(paths.exe)));
  log(`  ✓ client/  ${path.basename(paths.exe)}`);
  copyTree(paths.vst3Bundle, path.join(pkg, 'plugin', path.basename(paths.vst3Bundle)));
  log(`  ✓ plugin/${path.basename(paths.vst3Bundle)}/  （${allFiles(paths.vst3Bundle).length} 个文件）`);
  copyFile(paths.testsExe, path.join(pkg, 'qa', path.basename(paths.testsExe)));
  log(`  ✓ qa/  ${path.basename(paths.testsExe)}`);
  for (const item of optionalQa.files) {
    copyFile(item.path, path.join(pkg, 'qa', item.name));
    log(`  ✓ qa/  ${item.name}（必需 QA 二进制）`);
  }

  copyTree(paths.licensesDir, path.join(pkg, 'licenses'));
  copyTree(path.join(root, 'tools'), path.join(pkg, 'tools'));
  log(`  ✓ licenses/  ${allFiles(paths.licensesDir).length} 个文件`);

  copyFile(paths.readmeUser, path.join(pkg, 'README-先看这里.md'));
  copyFile(paths.readmeBuild, path.join(pkg, 'README-BUILD.md'));
  for (const file of ['Collect-Diagnostics.ps1','README-远程联调.md','VST3-Delivery.ps1','Install-VST3.ps1','Rollback-VST3.ps1','Verify-Local.ps1','运行离线自检.cmd']) copyFile(path.join(root,file),path.join(pkg,file));
  log('  ✓ README-先看这里.md、README-BUILD.md');

  return pkg;
}

/**
 * 证据落盘：
 *  - 报告本体 + 本次校验过的截图 + 可选同目录文本证据 → <pkg>/evidence/offline/
 *  - <工程>/evidence 顶层文件（若存在）→ <pkg>/evidence/
 * 报告在工程外时**只复制上面点名的文件**，绝不整目录搬运外部内容。
 */
function stageReportEvidence(pkg, expectedShots) {
  const evidenceOut = path.join(pkg, 'evidence');
  const offlineOut = path.join(evidenceOut, 'offline');

  const topLevel = 0; // Only current, explicitly validated evidence is copied.

  copyFile(reportPath, path.join(offlineOut, 'headphone-tests.json'));
  copyFile(FIDELITY_REPORT, path.join(offlineOut, 'fidelity-check.json'));
  copyFile(RESPONSE_REPORT, path.join(offlineOut, 'response-contract.json'));
  copyFile(path.join(evidenceDir,'profile-generation.json'),path.join(offlineOut,'profile-generation.json'));
  copyFile(optionsFor(root, argv).freeze, path.join(evidenceOut, 'delivery-freeze.json'));
  copyTree(path.join(evidenceDir,'external-vst3'),path.join(evidenceOut,'external-vst3'));
  copyTree(path.join(evidenceDir,'peak-telemetry'),path.join(evidenceOut,'peak-telemetry'));
  copyTree(path.dirname(TEMPO_REPORT),path.join(evidenceOut,'tempo-recall'));
  if(fs.existsSync(path.join(evidenceDir,'first-wrapper-failure')))copyTree(path.join(evidenceDir,'first-wrapper-failure'),path.join(evidenceOut,'first-wrapper-failure'));
  const staged = [...new Set([...expectedShots,...fs.readdirSync(evidenceDir).filter(name=>/^ui-.*\.png$/i.test(name)&&fs.statSync(path.join(evidenceDir,name)).isFile())])];
  for (const name of OPTIONAL_SIBLING_FILES) {
    if (fs.existsSync(path.join(evidenceDir, name))) staged.push(name);
  }
  const copied = ['headphone-tests.json', 'fidelity-check.json', 'response-contract.json'];
  for (const name of staged) {
    const from = path.join(evidenceDir, name);
    if (!fs.existsSync(from)) continue;
    copyFile(from, path.join(offlineOut, name));
    copied.push(name);
  }
  log(`  ✓ evidence/offline/  ${copied.length} 个文件：${copied.join(', ')}`);
  return { offlineDir: offlineOut, copied, topLevel };
}

function buildSourceArchive(stage, pkg, juce) {
  step('组装对应源码并打成源码 zip');
  const sourceRoot = path.join(stage, 'source-tree');
  if (fs.existsSync(sourceRoot)) fail(`stage 内已存在 source-tree（不应发生）：${sourceRoot}`);
  fs.mkdirSync(sourceRoot, { recursive: true });

  copyTree(paths.sourceDir, path.join(sourceRoot, 'Source'));
  copyTree(paths.testsDir, path.join(sourceRoot, 'tests'));
  copyTree(path.join(root, 'tools'), path.join(sourceRoot, 'tools'));
  copyTree(paths.licensesDir, path.join(sourceRoot, 'licenses'));
  copyFile(paths.cmake, path.join(sourceRoot, 'CMakeLists.txt'));
  copyFile(paths.buildCmd, path.join(sourceRoot, 'build.cmd'));
  copyFile(paths.self, path.join(sourceRoot, 'package.mjs'));
  copyFile(paths.readmeUser, path.join(sourceRoot, 'README-先看这里.md'));
  copyFile(paths.readmeBuild, path.join(sourceRoot, 'README-BUILD.md'));
  for (const file of ['Collect-Diagnostics.ps1','README-远程联调.md','VST3-Delivery.ps1','Install-VST3.ps1','Rollback-VST3.ps1','Verify-Local.ps1','运行离线自检.cmd']) copyFile(path.join(root,file),path.join(sourceRoot,file));
  copyTree(juce.dir, path.join(sourceRoot, 'deps', 'juce'));
  log(`  ✓ deps/juce/  ${juce.files.length} 个文件（源码包自带 JUCE，可独立配置）`);

  // 源码包必须真的带上新皮肤与保真修复文件（收集是整目录，这里是事后断言）
  for (const rel of [...REQUIRED_SKIN_FILES, ...REQUIRED_FIDELITY_FILES]) {
    if (!fs.existsSync(path.join(sourceRoot, ...rel.split('/')))) fail(`源码包里缺 ${rel} —— 打包逻辑有问题，拒绝继续。`);
  }
  log(`  ✓ 已核对新皮肤 ${REQUIRED_SKIN_FILES.length} 个 + 保真修复 ${REQUIRED_FIDELITY_FILES.length} 个文件在源码包内`);

  const frozen = readJson(optionsFor(root, argv).freeze);
  for (const entry of frozen.files.filter(entry => entry.kind === 'source' || entry.kind === 'juce')) {
    const relative = entry.kind === 'juce' ? path.join('deps', 'juce', entry.path) : entry.path;
    if (sha256(path.join(sourceRoot, relative)) !== entry.sha256) fail(`Staged corresponding source differs from frozen input: ${relative}`);
  }

  const sourceFiles = allFiles(sourceRoot);
  writeJson(path.join(sourceRoot, 'SOURCE-SHA256.json'), {
    product: PRODUCT_NAME,
    version,
    createdUtc: new Date().toISOString(),
    note: '本清单不包含 SOURCE-SHA256.json 自身。',
    files: sourceFiles.map(f => ({ path: path.relative(sourceRoot, f).replace(/\\/g, '/'), bytes: fs.statSync(f).size, sha256: sha256(f) }))
  });

  const sourceZip = path.join(pkg, 'source', `${packageName}-source.zip`);
  fs.mkdirSync(path.dirname(sourceZip), { recursive: true });
  zipCreate(sourceZip, ['.'], sourceRoot);
  const utf8 = assertZipUtf8(sourceZip);
  log(`  ✓ 源码 zip：${path.relative(root, sourceZip)}（${(fs.statSync(sourceZip).size / 1048576).toFixed(1)} MB；中文条目 ${utf8.nonAscii} 个，UTF-8 标志已置位）`);

  return { sourceZip, sourceFileCount: sourceFiles.length, utf8, sourceRoot };
}

function writeManifestsAndZip(stage, pkg, juce, report, sourceInfo, binding, evidenceInfo, fidelity, optionalQa) {
  step('写清单、交付记录并打 Windows zip');

  writeJson(path.join(pkg, 'evidence', 'delivery.json'), {
    product: PRODUCT_NAME,
    version,
    createdUtc: new Date().toISOString(),
    platform: process.platform,
    arch: process.arch,
    node: process.version,
    packageScript: 'package.mjs',
    packageScriptSha256: sha256(paths.self),
    pluginIdentity: { bundleId: EXPECTED_BUNDLE_ID, manufacturerCode: EXPECTED_MANUFACTURER_CODE, pluginCode: EXPECTED_PLUGIN_CODE, productName: PRODUCT_NAME },
    exeSha256: binding.hashes.exe,
    vst3Sha256: binding.hashes.vst3,
    testExeSha256: binding.hashes.tests,
    vst3BundleFiles: allFiles(paths.vst3Bundle).length,
    testReport: reportPath.replace(/\\/g, '/'),
    testReportSha256: sha256(reportPath),
    testReportVersion: report.version,
    testReportBindingMode: binding.mode,
    testReportDigests: binding.reportDigests,
    evidenceDirectory: evidenceDir.replace(/\\/g, '/'),
    evidenceFilesCopied: evidenceInfo.copied,
    callbackCppNewCount: report.callbackCppNewCount,
    uiResizeScales: report.ui.actualResizeScales,
    vst3ReportedName: report.vst3.name,
    vst3ReportedVersion: report.vst3.version,
    uiSkinFiles: REQUIRED_SKIN_FILES.map(f => ({ path: f, sha256: sha256(path.join(root, f)) })),
    fidelity: fidelity,
    requiredQaBinaries: optionalQa.files.map(item => ({ path: path.basename(item.path), sha256: sha256(item.path) })),
    frozenInputs: binding.freeze,
    peakTelemetry: binding.peakTelemetry,
    sourceArchive: `source/${packageName}-source.zip`,
    sourceArchiveSha256: sha256(sourceInfo.sourceZip),
    sourceArchiveUtf8Entries: sourceInfo.utf8.nonAscii,
    sourceFileCount: sourceInfo.sourceFileCount,
    bundledJuce: { path: 'deps/juce', files: juce.files.length, developmentSource: juce.dir, resolution: 'CMakeCache HM_JUCE (or matching explicit --juce); copied into source archive' },
    realHardwareTested: false,
    realWindowsDpiTested: false,
    thirdPartyDawTested: false,
    commercialDataLicenseVerified: false,
    scope: '本地开发交付；真实声卡、真实 Windows 显示缩放、第三方宿主与数据商业授权均为独立验收项，本包不宣称已完成。'
  });

  const manifestFiles = allFiles(pkg).filter(f => path.basename(f) !== 'SHA256.json');
  writeJson(path.join(pkg, 'SHA256.json'), {
    product: PRODUCT_NAME,
    version,
    createdUtc: new Date().toISOString(),
    note: '本清单不包含 SHA256.json 自身；路径相对发行包根目录。',
    files: manifestFiles.map(f => ({ path: path.relative(pkg, f).replace(/\\/g, '/'), bytes: fs.statSync(f).size, sha256: sha256(f) }))
  });
  log(`  ✓ SHA256.json（${manifestFiles.length} 个文件）`);

  const zipPath = path.join(stage, `${packageName}-Windows-x64.zip`);
  zipCreate(zipPath, [packageName], stage);
  const utf8 = assertZipUtf8(zipPath);
  log(`  ✓ stage 内已生成 ${path.basename(zipPath)}（${(fs.statSync(zipPath).size / 1048576).toFixed(1)} MB）`);
  log(`  ✓ 中文条目 ${utf8.nonAscii} 个，UTF-8 标志（0x0800）全部置位：${utf8.names.join(', ')}`);

  return { zipPath, manifestFiles: manifestFiles.length, utf8 };
}

// ---------------------------------------------------------------------------
// 主流程
// ---------------------------------------------------------------------------

function main() {
  log('HONESTMIX Headphone 当前候选交付打包（版本来自 CMake）');
  log(`  工程根：${root}`);
  log(`  版本（来自 CMakeLists.txt）：${version}`);
  log(`  模式：${PREFLIGHT ? '仅检查（--preflight）' : '打包'}`);
  assertStageRootSane(); // 先挡掉「stage 根 = 工程根 / 工程根的祖先」这种危险配置

  checkProjectIdentity();
  checkArtifacts();
  const sourceInputs = checkBuildFreshness();
  const optionalQa = checkOptionalQa(sourceInputs);
  const reportInfo = checkReport(sourceInputs);
  checkCurrentContracts(sourceInputs);
  const peakTelemetry = gate('当前实际峰值遥测/双档界面保持与重置证据完整且绑定当前SharedCode', () =>
    assertPeakTelemetry({ folder: path.join(evidenceDir, 'peak-telemetry'), version, sourceRoot: root, liveBuild: true }));
  const frozen = gate('当前源码/依赖/六个二进制/报告/截图匹配最终冻结 SHA', () => assertFrozen(optionsFor(root, argv)));
  const juce = resolveJuce();
  checkDisk();

  requireAll();

  const binding = {
    hashes: { exe: sha256(paths.exe), vst3: sha256(paths.vst3Binary), tests: sha256(paths.testsExe) },
    mode: 'mtime-plus-explicit-frozen-sha256',
    freeze: frozen,
    peakTelemetry,
    reportDigests: collectHexDigests(reportInfo.report)
  };

  if (PREFLIGHT) {
    log('\n全部检查通过。未写入任何文件。');
    log('去掉 --preflight 即可执行打包。');
    return 0;
  }

  const zipPath = path.join(releaseRoot, `${packageName}-Windows-x64.zip`);
  const pkgDir = path.join(releaseRoot, packageName);
  for (const existing of [pkgDir, zipPath]) {
    if (fs.existsSync(existing)) {
      fail(`已存在同名产物，拒绝覆盖：${existing}\n  请人工确认并移走后重试（本脚本不会自动删除你的文件）。`);
    }
  }

  const stage = createStage(version);
  log(`\n  stage（本次唯一）：${stage}`);
  log(`  owner token：${runToken}`);

  const fidelity = {
    reportPath: FIDELITY_REPORT.replace(/\\/g, '/'),
    reportSha256: sha256(FIDELITY_REPORT),
    responseReportSha256: sha256(RESPONSE_REPORT),
    requiredFiles: REQUIRED_FIDELITY_FILES.map(f => ({ path: f, sha256: sha256(path.join(root, f)) }))
  };

  try {
    const pkg = buildPackageTree(stage, reportInfo.screenshots, optionalQa);
    const evidenceInfo = stageReportEvidence(pkg, reportInfo.screenshots);
    const sourceInfo = buildSourceArchive(stage, pkg, juce);
    const { zipPath: madeZip, manifestFiles } = writeManifestsAndZip(stage, pkg, juce, reportInfo.report, sourceInfo, binding, evidenceInfo, fidelity, optionalQa);

    // Catch mutations during copying/archiving before publishing any final tree.
    assertFrozen(optionsFor(root, argv));

    fs.mkdirSync(releaseRoot, { recursive: true });
    const movedPkg = moveInto(pkg, pkgDir, runToken);
    const movedZip = moveInto(madeZip, zipPath, runToken);
    if (movedPkg === 'copy-verified' || movedZip === 'copy-verified') {
      log('  · 跨卷搬运已完成并逐文件校验（源 stage 副本已删除）');
    }

    step('完成');
    log(`  发行包目录：${path.relative(root, pkgDir)}`);
    log(`  Windows zip：${path.relative(root, zipPath)}`);
    log(`  zip SHA256 ：${sha256(zipPath)}`);
    log(`  包内文件数 ：${manifestFiles}`);
    log(`  源码文件数 ：${sourceInfo.sourceFileCount}`);

    writeJson(path.join(root, 'tmp', `package-result-${version}.json`), {
      version,
      createdUtc: new Date().toISOString(),
      output: path.relative(root, pkgDir),
      zip: path.relative(root, zipPath),
      zipBytes: fs.statSync(zipPath).size,
      zipSha256: sha256(zipPath),
      packageFiles: manifestFiles,
      sourceFiles: sourceInfo.sourceFileCount,
      optionalQaBinaries: optionalQa.files.map(f => f.name),
      testReport: reportPath,
      evidenceDirectory: evidenceDir,
      fidelityReport: FIDELITY_REPORT,
      reportBindingMode: binding.mode,
      stage,
      stageOwnerToken: runToken
    });

    log(`  stage 保留在 ${stage}`);
    log('  确认无误后清理（校验路径与 owner token 后才会删除）：');
    log(`    node package.mjs --discard-stage "${stage}" --token ${runToken}`);
    return 0;
  } catch (error) {
    log(`\n打包失败，stage 保留在：${stage}`);
    log(`  原因：${error.message}`);
    log('  清理方式（校验路径与 owner token 后才会删除）：');
    log(`    node package.mjs --discard-stage "${stage}" --token ${runToken}`);
    throw error;
  }
}

try {
  if (DISCARD_ARG) {
    const token = value('--token', null);
    if (!token) fail('清理 stage 需要显式 --token <owner token>（见该目录内 stage-owner.json 的 token 字段）。');
    removeStage(path.resolve(DISCARD_ARG), token);
    process.exitCode = 0;
  } else {
    process.exitCode = main();
  }
} catch (error) {
  console.error(`\n[失败] ${error.message}`);
  process.exitCode = 1;
}
