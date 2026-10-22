#!/usr/bin/env node
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { optionsFor, captureDelivery, digest } from './delivery-contract.mjs';
const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
try {
  const options = optionsFor(root, process.argv.slice(2));
  if (fs.existsSync(options.freeze)) throw new Error(`Freeze already exists; preserve it and use a new --freeze-manifest path: ${options.freeze}`);
  const frozen = { ...captureDelivery(options), createdUtc: new Date().toISOString(), note: 'Hash binding only. Detailed assertions remain in the four reports including tempo recall and mandatory actual peak telemetry evidence; this command does not run tests.' };
  fs.mkdirSync(path.dirname(options.freeze), { recursive: true });
  fs.writeFileSync(options.freeze, JSON.stringify(frozen, null, 2) + '\n', { encoding: 'utf8', flag: 'wx' });
  console.log(JSON.stringify({ freeze: options.freeze, sha256: digest(options.freeze), files: frozen.files.length, version: frozen.version }));
} catch (error) { console.error(error.message); process.exitCode = 1; }
