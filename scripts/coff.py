"""Convert the restricted GCC x86-64 ELF relocatable used here to AMD64 COFF."""
import struct as s,sys
from pathlib import Path
def pack(fmt,*x):return s.pack('<'+fmt,*x)
def get(fmt,b,o):return s.unpack_from('<'+fmt,b,o)
def cstr(b,o):return b[o:b.index(b'\0',o)].decode()
class COFF:
 def __init__(self):self.sec=[];self.sym=[];self.str=bytearray(b'\0'*4)
 def name(self,n):
  b=n.encode()
  if len(b)<=8:return b.ljust(8,b'\0')
  p=len(self.str);self.str+=b+b'\0';return pack('II',0,p)
 def section(self,n,data,flags):
  self.sec.append((n,bytearray(data),flags,[]));return len(self.sec)
 def symbol(self,n,sec,val=0,typ=0,cls=2):
  self.sym.append(self.name(n)+pack('IhHBB',val,sec,typ,cls,0));return len(self.sym)-1
 def reloc(self,sec,at,sym,typ):self.sec[sec-1][3].append((at,sym,typ))
 def emit(self,out):
  ptr=20+40*len(self.sec);headers=b'';body=b''
  for n,b,f,r in self.sec:
   raw=0 if f&0x80 else ptr+len(body)
   pos=ptr+len(body)+(len(b) if raw else 0) if r else 0
   headers+=n.encode().ljust(8,b'\0')+pack('IIIIIIHHI',0,0,len(b),raw,pos,0,len(r),0,f)
   if raw:body+=b
   body+=b''.join(pack('IIH',*x) for x in r)
  self.str[:4]=pack('I',len(self.str))
  Path(out).write_bytes(pack('HHIIIHH',0x8664,len(self.sec),0,ptr+len(body),len(self.sym),0,0)+headers+body+b''.join(self.sym)+self.str)
def elf(infile,out):
 b=Path(infile).read_bytes()
 assert b[:6]==b'\x7fELF\x02\x01'
 off,=get('Q',b,40); es,n,st=get('HHH',b,58)
 h=[get('IIQQQQIIQQ',b,off+i*es) for i in range(n)]
 def dat(i):return b[h[i][4]:h[i][4]+h[i][5]]
 names=dat(st); m={}; c=COFF(); flags={1:0x60500020,2:0x40500040,3:0xc0500040,4:0xc0500080}
 for i,(nm,t,f,_,o,sz,*_) in enumerate(h):
  if not f&2 or not sz:continue
  name=cstr(names,nm)
  j=1 if f&4 else 4 if t==8 else 3 if f&1 else 2
  m[i]=c.section({1:'.text',2:'.rdata',3:'.data',4:'.bss'}[j],b'\0'*sz if t==8 else dat(i),flags[j])
 for i,x in enumerate(h):
  if x[1]!=2:continue
  syms=dat(i); strs=dat(x[6]); sm={}
  for j in range(1,len(syms)//24):
   nm,info,other,sec,val,sz=get('IBBHQQ',syms,j*24)
   if sec not in m and sec!=0:continue
   name=cstr(strs,nm) if nm else 'local_'+str(j)
   sm[j]=c.symbol(name,m.get(sec,0),val,32 if info&15==2 else 0,3 if info>>4==0 else 2)
  for k,r in enumerate(h):
   if r[1]!=4 or r[7] not in m:continue
   idx=m[r[7]]; buf=c.sec[idx-1][1]; d=dat(k)
   for j in range(0,len(d),24):
    p,info,add=get('QQq',d,j); typ=info&0xffffffff; symbol=info>>32
    if typ in (2,4):sz=4;coff=4;add+=4
    elif typ in (10,11):sz=4;coff=2
    elif typ==1:sz=8;coff=1
    else:raise Exception('Unsupported ELF relocation '+str(typ))
    buf[p:p+sz]=add.to_bytes(sz,'little',signed=True)
    c.reloc(idx,p,sm[symbol],coff)
 c.emit(out)
def imports(out):
 imports={'KERNEL32.dll':'ExitProcess GetModuleHandleW LoadLibraryExW GetProcAddress Sleep GetTickCount64 QueryPerformanceCounter QueryPerformanceFrequency GetLastError GetCurrentProcessId GetEnvironmentVariableW CreateDirectoryW CreateFileW ReadFile WriteFile CloseHandle MoveFileExW DeleteFileW', 'USER32.dll':'RegisterClassExW CreateWindowExW DefWindowProcW ShowWindow PeekMessageW TranslateMessage DispatchMessageW GetAsyncKeyState GetClientRect GetDC ReleaseDC LoadCursorW PostQuitMessage SetProcessDPIAware GetForegroundWindow TrackMouseEvent MessageBoxW', 'GDI32.dll':'StretchDIBits PatBlt'}
 c=COFF(); rw=0xc0300040; ro=0x40300040
 desc=c.section('.idata$2',b'\0'*(20*len(imports)),rw)
 c.section('.idata$3',b'\0'*20,rw)
 ilt=c.section('.idata$4',b'',ro);iat=c.section('.idata$5',b'',rw);data=c.section('.idata$6',b'',ro)
 def put(sec,b):
  d=c.sec[sec-1][1]; pos=len(d); d+=b; return pos
 for j,(dll,funcs) in enumerate(imports.items()):
  key='dll'+str(j); s1=c.symbol(key+'_ilt',ilt,len(c.sec[ilt-1][1]),cls=3); s2=c.symbol(key+'_iat',iat,len(c.sec[iat-1][1]),cls=3); s3=c.symbol(key+'_name',data,put(data,dll.encode()+b'\0'),cls=3)
  for off,sym in [(0,s1),(12,s3),(16,s2)]:c.reloc(desc,j*20+off,sym,3)
  for f in funcs.split():
   d=c.sec[data-1][1]
   if len(d)%2:d+=b'\0'
   sym=c.symbol('hn_'+f,data,put(data,b'\0\0'+f.encode()+b'\0'),cls=3)
   p=put(ilt,b'\0'*8); q=put(iat,b'\0'*8)
   c.symbol('__imp_'+f,iat,q); c.reloc(ilt,p,sym,3); c.reloc(iat,q,sym,3)
  put(ilt,b'\0'*8);put(iat,b'\0'*8)
 c.emit(out)
if sys.argv[1]=='imports':imports(sys.argv[2])
else:elf(*sys.argv[1:])
