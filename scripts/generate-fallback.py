from pathlib import Path
import math,struct,zlib,wave,json
rate=22050;seconds=16;n=rate*seconds;pcm=[0.0]*n
chords=[(48,52,55,59),(45,48,52,55),(41,45,48,52),(43,50,55,57)]
melody=[72,76,71,67,69,72,76,67,65,69,72,64,67,74,69,62]
def add(note,onset,duration,gain,pad=False):
 freq=440*2**((note-69)/12);start=round(onset*rate);count=round(duration*rate)
 for j in range(count):
  t=j/rate;x=j/count
  env=math.sin(math.pi*x)**2 if pad else (1-math.exp(-35*t))*math.exp(-2.8*t)*math.cos(math.pi*x/2)**2
  # Independently composed soft harmonics, not sampled console audio.
  sound=math.sin(2*math.pi*freq*t)+0.16*math.sin(4*math.pi*freq*t)+0.035*math.sin(6*math.pi*freq*t)
  pcm[(start+j)%n]+=gain*env*sound
for bar,chord in enumerate(chords):
 for note in chord:add(note,bar*4,5.2,0.10,True)
for beat,note in enumerate(melody):add(note,beat+0.12,2.4,0.12)
mean=sum(pcm)/n;pcm=[v-mean for v in pcm];peak=max(abs(v) for v in pcm);samples=[round(v/peak*0.20*32767) for v in pcm]
raw=struct.pack('<'+'h'*len(samples),*samples)
path=(Path(__file__).resolve().parents[1]/'launcher/data/fallback-preview.wav')
with wave.open(str(path),'wb') as w:w.setnchannels(1);w.setsampwidth(2);w.setframerate(rate);w.writeframes(raw)
waveBytes=path.read_bytes();packed=struct.pack('<I',len(waveBytes))+zlib.compress(waveBytes,9)
out=(Path(__file__).resolve().parents[1]/'launcher/data/fallback_bgm.bin');out.parent.mkdir(exist_ok=True);out.write_bytes(packed)
assert abs(samples[-1]-samples[0])<256
assert zlib.decompress(packed[4:])==waveBytes
print(json.dumps({'seconds':seconds,'sampleRate':rate,'channels':1,'peak':max(map(abs,samples)),'loopBoundaryStep':samples[0]-samples[-1],'packedBytes':len(packed),'decodedBytes':len(waveBytes),'originalComposition':True}))
