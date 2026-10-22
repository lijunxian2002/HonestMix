// Recovered mathematical pipeline, explicit pinned input manifest. No name fallback.
const fs=require('fs'),path=require('path'),crypto=require('crypto');
const SR=44100,IR_LEN=1024,FFT_SIZE=2048;
const manifest=JSON.parse(fs.readFileSync(path.join(__dirname,'provenance.json'),'utf8'));
const digest=b=>crypto.createHash('sha256').update(b).digest('hex');
function input(entry){const p=path.resolve(__dirname,entry.input);if(!p.startsWith(__dirname+path.sep))throw Error('Input escapes provenance directory');const bytes=fs.readFileSync(p);if(digest(bytes)!==entry.sha256)throw Error('Pinned input SHA changed: '+entry.input);return p;}
const output=process.argv[2];if(!output||process.argv.length!==3)throw Error('Use node regenerate.cjs NEW_OUTPUT.h; output must not exist');
if(fs.existsSync(output))throw Error('Never overwrite existing output; generate a new file then inspect its diff');
if(manifest.profiles.length!==1573||manifest.profiles.some((p,i)=>p.index!==i))throw Error('Profile order/count changed');
function readCSV(file) {
    return fs.readFileSync(file, 'utf8').trim().split('\n').slice(1).map(r => { const [f, v] = r.split(','); return [parseFloat(f), parseFloat(v)]; });
}
function interpLog(data, f) {
    if (f <= data[0][0]) return data[0][1];
    if (f >= data[data.length - 1][0]) return data[data.length - 1][1];
    const lf = Math.log(f);
    let lo = 0, hi = data.length - 1;
    while (hi - lo > 1) { const mid = (lo + hi) >> 1; if (data[mid][0] <= f) lo = mid; else hi = mid; }
    const t = (lf - Math.log(data[lo][0])) / (Math.log(data[hi][0]) - Math.log(data[lo][0]));
    return data[lo][1] + (data[hi][1] - data[lo][1]) * t;
}
function relTo1k(data, f) { return interpLog(data, f) - interpLog(data, 1000); }

// 1/12 octave 平滑（log 频率域滑动平均）
function smooth112(data) {
    const n = data.length;
    const out = new Array(n);
    const half = 0.5 / 12; // ±半窗 octave
    for (let i = 0; i < n; i++) {
        const lf = Math.log(data[i][0]);
        const loF = Math.exp(lf - half * Math.LN2), hiF = Math.exp(lf + half * Math.LN2);
        let j0 = i, j1 = i;
        while (j0 > 0 && data[j0 - 1][0] >= loF) j0--;
        while (j1 < n - 1 && data[j1 + 1][0] <= hiF) j1++;
        let s = 0;
        for (let j = j0; j <= j1; j++) s += data[j][1];
        out[i] = s / (j1 - j0 + 1);
    }
    return out;
}

// ── FFT（通用复数）──
function fft(re, im, invert) {
    const n = re.length;
    for (let i = 1, j = 0; i < n; i++) {
        let bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { [re[i], re[j]] = [re[j], re[i]]; [im[i], im[j]] = [im[j], im[i]]; }
    }
    for (let len = 2; len <= n; len <<= 1) {
        const ang = (invert ? 2 : -2) * Math.PI / len;
        const wRe = Math.cos(ang), wIm = Math.sin(ang);
        for (let i = 0; i < n; i += len) {
            let cRe = 1, cIm = 0;
            for (let k = 0; k < len / 2; k++) {
                const uRe = re[i + k], uIm = im[i + k];
                const vRe = re[i + k + len / 2] * cRe - im[i + k + len / 2] * cIm;
                const vIm = re[i + k + len / 2] * cIm + im[i + k + len / 2] * cRe;
                re[i + k] = uRe + vRe; im[i + k] = uIm + vIm;
                re[i + k + len / 2] = uRe - vRe; im[i + k + len / 2] = uIm - vIm;
                const nc = cRe * wRe - cIm * wIm, ni = cRe * wIm + cIm * wRe;
                cRe = nc; cIm = ni;
            }
        }
    }
    if (invert) for (let i = 0; i < n; i++) { re[i] /= n; im[i] /= n; }
}

// ── FIR 生成：correction dB → 1024 taps ──
// 方案：最小相位（cepstral 法）——log|H| → IFFT 倒谱 → 因果×2 → FFT → exp → IFFT
// 实测 M50x 全频段与官方偏差 ≤0.2dB（Hann 窗/峰值居中均破坏深谷型频谱，已弃）
function generateFir(correctionDB) {
    const nyquist = SR / 2;
    // 1. 插值到 bin 频率（0 ~ Nyquist，IR_LEN+1 点）
    const spec = [];
    for (let k = 0; k <= IR_LEN; k++) {
        const f = k * nyquist / IR_LEN;
        let db = interpLog(correctionDB, f);
        if (!isFinite(db)) db = 0;
        spec.push(Math.max(Math.min(Math.pow(10, db / 20), 31.62), 0.0316)); // ±30dB 硬保护，无 +6 封顶
    }
    // 2. 镜像实偶频谱
    const specFull = new Float64Array(FFT_SIZE);
    for (let k = 0; k <= IR_LEN; k++) specFull[k] = spec[k];
    for (let k = 1; k < IR_LEN; k++) specFull[FFT_SIZE - k] = spec[k];
    // 3. 最小相位转换（cepstral）
    const re = new Float64Array(FFT_SIZE), im = new Float64Array(FFT_SIZE);
    for (let k = 0; k < FFT_SIZE; k++) re[k] = Math.log(Math.max(specFull[k], 1e-9));
    fft(re, im, true);                              // IFFT → 倒谱
    for (let n = 1; n < FFT_SIZE / 2; n++) { re[n] *= 2; im[n] *= 2; } // 因果化
    re[FFT_SIZE / 2] = 0; im[FFT_SIZE / 2] = 0;
    for (let n = FFT_SIZE / 2 + 1; n < FFT_SIZE; n++) { re[n] = 0; im[n] = 0; }
    fft(re, im, false);                             // FFT → log H（复数）
    for (let k = 0; k < FFT_SIZE; k++) {            // exp → 最小相位频谱
        const e = Math.exp(re[k]);
        const nr = e * Math.cos(im[k]), ni = e * Math.sin(im[k]);
        re[k] = nr; im[k] = ni;
    }
    fft(re, im, true);                              // IFFT → 因果 FIR
    // 4. 截取前 IR_LEN（最小相位能量集中前部，截断损失最小；1kHz 归零由 updateCurve 处理）
    const shifted = new Float64Array(IR_LEN);
    for (let n = 0; n < IR_LEN; n++) shifted[n] = re[n];
    return shifted;
}


const target=readCSV(input(manifest.target));const tRef=interpLog(target,1000);const out=[];
for(const p of manifest.profiles){
 const raw=readCSV(input(p));if(raw.length<2||raw.some((x,i)=>!Number.isFinite(x[0])||!Number.isFinite(x[1])||x[0]<=0||(i>0&&x[0]<=raw[i-1][0])))throw Error('Invalid measurement grid');
 const ref=interpLog(raw,1000),relative=raw.map(x=>[x[0],x[1]-ref]),smooth=smooth112(relative);
 const correction=relative.map((x,i)=>[x[0],interpLog(target,x[0])-tRef-smooth[i]]);
 const fir=Array.from(generateFir(correction));if(fir.some(x=>!Number.isFinite(x)))throw Error('Invalid generated FIR');out.push({...p,fir});
}
const lines=['#pragma once','// Reproducible HONESTMIX FIR data; see tools/profile-generation/provenance.json','//44100Hz,1024 taps; recovered original pipeline; exact pinned measurement files.','// Source labels corrected; K712 Dekoni uses its actual pad-specific input.',''];
const names=out.map((p,i)=>'hp_'+i+'_'+p.name.replace(/[^A-Za-z0-9]/g,'_').toLowerCase());
out.forEach((p,i)=>{lines.push('// '+p.name+' (source: '+p.source+')','static const float '+names[i]+'[1024] = {');const v=p.fir.map(x=>x.toFixed(10)+'f');for(let j=0;j<v.length;j+=8)lines.push('    '+v.slice(j,j+8).join(', ')+(j+8<v.length?',':''));lines.push('};','');});
lines.push('struct HeadphoneInfo { const char* name; const float* fir; };','static constexpr int NUM_PROFILES = '+out.length+';','static const HeadphoneInfo kProfiles[NUM_PROFILES] = {');out.forEach((p,i)=>lines.push('    { '+JSON.stringify(p.name)+', '+names[i]+' },'));lines.push('};');
fs.writeFileSync(output,lines.join('\n'),'utf8');console.log(JSON.stringify({profiles:out.length,output,sha256:digest(fs.readFileSync(output)),manifestSha256:digest(fs.readFileSync(path.join(__dirname,'provenance.json')))}));
