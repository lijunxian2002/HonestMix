import fs from 'node:fs';
import path from 'node:path';
import os from 'node:os';
import crypto from 'node:crypto';
import {spawnSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
const here=path.dirname(fileURLToPath(import.meta.url)),root=path.resolve(here,'../..');
const output=process.argv[2];if(!output||process.argv.length!==3||fs.existsSync(output))throw Error('Choose a NEW verification report path');
const sha=p=>crypto.createHash('sha256').update(fs.readFileSync(p)).digest('hex');
const report={schema:'honestmix.pinned-profile-generation/1',atUtc:new Date().toISOString(),passed:false,hardwareAudioOpened:false,publicNetworkUsed:false};
let temp;
try{
 const manifestFile=path.join(here,'provenance.json'),manifest=JSON.parse(fs.readFileSync(manifestFile,'utf8'));
 const cmake=fs.readFileSync(path.join(root,'CMakeLists.txt'),'utf8');report.version=/project\s*\(\s*\w+\s+VERSION\s+([\d.]+)/i.exec(cmake)[1];
 if(manifest.profiles.length!==1573||manifest.profiles.some((p,i)=>p.index!==i))throw Error('Profile order/count changed');
 const pad=manifest.profiles[1572];
 if(pad.name!=='AKG K712 (Dekoni sheepskin earpads)'||pad.originalMeasurementPath!=='oratory1990/data/over-ear/AKG K712 (Dekoni sheepskin Earpads).csv'||pad.sha256!=='3c7af9258478eac571409f33adad6bce2b44343996d9c2782fc407b3cbdad04b')throw Error('K712 pad-specific source not pinned');
 if(manifest.target.sha256!=='12797010a78a693adeaa3e91f961e88cca3d25abc7883d144369915fc7c18f7c')throw Error('Original target changed');
 for(const item of [manifest.target,...manifest.profiles]){
  const full=path.resolve(here,item.input);if(!full.startsWith(here+path.sep)||sha(full)!==item.sha256)throw Error('Pinned input missing, changed or outside tree');
 }
 temp=fs.mkdtempSync(path.join(os.tmpdir(),'hm-profile-proof-'));
 const generated=path.join(temp,'fresh.h'),generator=path.join(here,'regenerate.cjs');
 const run=spawnSync(process.execPath,[generator,generated],{encoding:'utf8',timeout:180000,windowsHide:true,maxBuffer:1024*1024});
 if(run.status!==0)throw Error('Exact pipeline regeneration failed: '+(run.stderr||run.error?.message||run.status));
 const header=path.join(root,'Source/dsp/fir_data.h');
 report.headerSha256=sha(header);report.regeneratedHeaderSha256=sha(generated);
 if(report.headerSha256!==report.regeneratedHeaderSha256)throw Error('Compiled FIR header is not byte-exact to pinned inputs/generator');
 const measured=path.join(temp,'measurement.h'),measurementGenerator=path.join(here,'generate-measurement.cjs');
 const measuredRun=spawnSync(process.execPath,[measurementGenerator,measured],{encoding:'utf8',timeout:180000,windowsHide:true,maxBuffer:1024*1024});
 if(measuredRun.status!==0)throw Error('Pinned raw measurement regeneration failed: '+(measuredRun.stderr||measuredRun.error?.message||measuredRun.status));
 report.measurementHeaderSha256=sha(path.join(root,'Source/UI/measured_data.h'));report.regeneratedMeasurementHeaderSha256=sha(measured);report.measurementGeneratorSha256=sha(measurementGenerator);
 if(report.measurementHeaderSha256!==report.regeneratedMeasurementHeaderSha256)throw Error('Compiled measurement header differs from exact pinned unsmoothed CSV values');
 report.rawMeasurementsExact=true;report.measurementPointsPerProfile=695;report.measuredFrequencyMin=20;report.measuredFrequencyMax=19955.54;
 report.manifestSha256=sha(manifestFile);report.generatorSha256=sha(generator);report.inputCount=manifest.profiles.length;report.targetSha256=manifest.target.sha256;report.correctedProfileIndex=1572;report.correctedPadInputSha256=pad.sha256;report.exactRegenerationPassed=true;report.passed=true;
}catch(error){report.error=error.message;process.exitCode=1;}finally{
 if(temp){for(const name of ['fresh.h','measurement.h']){const ownFile=path.join(temp,name);if(fs.existsSync(ownFile))fs.unlinkSync(ownFile);}fs.rmdirSync(temp);}
 fs.writeFileSync(output,JSON.stringify(report,null,2));console.log(JSON.stringify(report));
}
