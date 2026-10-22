from pathlib import Path
import json,numpy as np,hashlib,re,datetime
import sys
gen=Path(__file__).resolve().parent;task=gen.parents[1];
manifest=json.loads((gen/'provenance.json').read_text(encoding='utf8'));entry=manifest['profiles'][1572];assert entry['name']=='AKG K712 (Dekoni sheepskin earpads)'
def read(p):
 a=np.loadtxt(p,delimiter=',',skiprows=1);assert a.ndim==2 and a.shape[1]==2 and np.isfinite(a).all();assert (np.diff(a[:,0])>0).all();return a
raw=read(gen/entry['input']);target=read(gen/manifest['target']['input'])
def interp(data,frequencies):return np.interp(np.log(np.maximum(frequencies,data[0,0])),np.log(data[:,0]),data[:,1])
relative=raw[:,1]-interp(raw,np.array([1000.]))[0]
low=raw[:,0]*2**(-1/24);high=raw[:,0]*2**(1/24)
smooth=np.array([relative[(raw[:,0]>=lo)&(raw[:,0]<=hi)].mean() for lo,hi in zip(low,high)])
delta=interp(target,raw[:,0])-interp(target,np.array([1000.]))[0]-smooth
bins=np.arange(1025)*44100/2048
desired=interp(np.column_stack((raw[:,0],delta)),bins)
positive=np.clip(10**(desired/20),.0316,31.62)
full=np.concatenate((positive,positive[-2:0:-1]));assert len(full)==2048
cep=np.fft.ifft(np.log(full));cep[1:1024]*=2;cep[1024:]=0
oracle=np.fft.ifft(np.exp(np.fft.fft(cep))).real[:1024]
header=task/'Source/dsp/fir_data.h';text=header.read_text(encoding='utf8');m=re.search(r'// AKG K712 \(Dekoni sheepskin earpads\) \(source: oratory1990\)\s*\nstatic const float\s+\w+\[1024\]\s*=\s*\{([^}]+)\};',text);assert m
actual=np.array([float(s.strip().rstrip('f')) for s in m[1].split(',') if s.strip()]);assert len(actual)==1024
error=float(np.max(np.abs(actual-oracle)));assert error<=6e-11,error
freq=np.geomspace(20,20000,257);basis=np.exp(-2j*np.pi*freq[:,None]*np.arange(1024)[None,:]/44100);a=basis@actual;b=basis@oracle
complexError=float(np.max(np.abs(a-b)));magnitudeError=float(np.max(np.abs(20*np.log10(np.abs(a/b)))));phaseError=float(np.max(np.abs(np.angle(a/b))));assert complexError<=1e-7 and magnitudeError<=.0001 and phaseError<=.00001
result={'schema':'honestmix.pad-input-independent-oracle/1','atUtc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'passed':True,'profileIndex':1572,'profileName':entry['name'],'numpyVersion':np.__version__,'separateImplementation':'Independent numpy interpolation/window means/FFT/cepstral reconstruction; no import of recoveredJS math. Original explicit Nyquist-cepstrum-zero convention preserved.','maxTenDecimalCoefficientAbsoluteError':error,'tenDecimalRoundingGate':6e-11,'frequencies':257,'maxComplexResponseError':complexError,'maxMagnitudeErrorDb':magnitudeError,'maxPhaseErrorRadians':phaseError,'rawInputSha256':hashlib.sha256((gen/entry['input']).read_bytes()).hexdigest(),'targetSha256':manifest['target']['sha256'],'headerSha256':hashlib.sha256(header.read_bytes()).hexdigest(),'actualInputPath':entry['originalMeasurementPath'],'oldWrongInput':entry['replacesWrongVariantBaseInput'],'noAudioOrMeasurementPermissionClaim':True}
output=Path(sys.argv[1]);assert not output.exists(),'Choose a new report path';output.write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf8');print(json.dumps(result,ensure_ascii=False))
