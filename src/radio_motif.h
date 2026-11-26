/* Two original station idents, arranged and synthesized at the mixing desk.
 * 16 kHz, mono signed PCM. No filesystem reads, mixer, floating point or
 * recorded assets. The winmm asynchronous API borrows the RIFF storage. */
#ifndef DC_RADIO_MOTIF_H
#define DC_RADIO_MOTIF_H
enum {RM_RATE=16000,RM_FRAMES=56000};
typedef struct{
 unsigned riff,bytes,wave,fmt,fmtBytes;
 unsigned short codec,channels;unsigned rate,byteRate;
 unsigned short align,bits;unsigned data,dataBytes;
 short pcm[RM_FRAMES];
} RadioWave;
_Static_assert(__builtin_offsetof(RadioWave,pcm)==44 && sizeof(short)==2,"x86-64 PCM ABI");
static int rm_min(int a,int b){return a<b?a:b;}
/* Bipolar parabolic approximation to a sine, periodic and zero crossing. */
static int rm_sine(unsigned phase){
 int x=(int)(phase>>17)&16383;
 int v=(x*(16384-x))>>12;
 return phase&0x80000000u?-v:v;
}
static void rm_note(short *pcm,int start,int len,int hz,int amp,int brass){
 unsigned phase=0,inc=(unsigned)(((unsigned long long)hz<<32)/RM_RATE);
 if(start+len>RM_FRAMES)len=RM_FRAMES-start;
 for(int i=0;i<len;i++,phase+=inc){
  int env=brass?rm_min(1024,i*3):rm_min(1024,i*12);
  env=env*rm_min(1024,(len-1-i)*4)/1024;
  if(!brass)env=env*5600/(5600+i*2);
  int t=rm_sine(phase)+(brass?rm_sine(phase*2)/2+rm_sine(phase*3)/3:rm_sine(phase*2)/6);
  int v=pcm[start+i]+(t*amp/16384)*env/1024;
  pcm[start+i]=(short)(v>30000?30000:v< -30000?-30000:v);
 }
}
static void rm_hit(short *pcm,int start,int len,int amp,unsigned seed,int brush){
 int previous=0;
 for(int i=0;i<len&&i+start<RM_FRAMES;i++){
  seed=seed*1664525u+1013904223u;
  int noise=(int)(seed>>17)-16384;
  int delta=noise-previous;previous=noise;
  int env=(len-1-i)*1024/len;
  env=env*rm_min(1024,i*(brush?9:110))/1024;
  pcm[start+i]+=(short)(((delta*amp/32768)*env)/1024);
 }
}
/* Original four-beat synth loops. No prerecorded songs, Internet or accounts.
 * Stage presets and district stations use different progressions and chroma.
 * The engine requests amplified or muffled alternate mixes of each station. */
static void rm_afterhours(RadioWave *w,int track){
 const int stage=track<=5, station=stage?track-3:(track-6)%3;
 const int bass=track>=9&&track<=11, outside=track>=12;
 static const unsigned short chords[6][4]={
  {147,175,220,262},{130,165,196,246},{164,196,247,294},
  {110,131,165,220},{131,156,196,233},{98,123,147,196}};
 static const unsigned char phrase[3][8]={
  {0,1,3,2,1,3,2,0},{1,0,2,3,0,2,0,1},{0,2,1,2,3,1,2,0}};
 const unsigned short *key=chords[station+(stage?0:3)];
 int b=stage?RM_FRAMES/8:(station==1?RM_FRAMES/4:RM_FRAMES/8);
 for(int i=0;i<8;i++){
  int t=(i*RM_FRAMES)/8;
  int f=key[phrase[station][i]];
  rm_note(w->pcm,t,b+690,(outside?key[0]/2:f*(station==0?2:1)),outside?1700:3600,station==2&&!outside);
  if((i&1)==0||station==2){
   rm_note(w->pcm,t,b,key[0]/2,bass?9700:outside?4700:stage?5200:4100,0);
   if(!outside)rm_hit(w->pcm,t,b/12,stage?1500:2080,197u+i+station*12u,0);
  }else if(!outside)rm_hit(w->pcm,t,b/10,1900,90u+i,station==1);
  if(stage&&i%2)rm_note(w->pcm,t+b/3,b/2,key[2]*2,1600,1);
 }
 /* Taper the tail; the loops re-start with their own phase and no hard click. */
 for(int i=0;i<192;i++)w->pcm[RM_FRAMES-192+i]=(short)(w->pcm[RM_FRAMES-192+i]*(191-i)/192);
}
static void rm_refreshment(RadioWave *w,int track){
 /* Four short, distinct fictional afterglow signals. They are not recordings. */
 static const int motif[4][5]={{370,494,587,740,880},{440,370,294,247,185},
                            {330,523,392,587,659},{293,440,523,392,587}};
 int id=track-15;
 for(int n=0;n<5;n++){
  int at=n*(id==1?1850:1460),f=motif[id][n];
  rm_note(w->pcm,at,9600,f,id==1?2050:2650,0);
  rm_note(w->pcm,at+1600,5000,id==2?f*2:f/2,740,0);
 }
 if(id==0||id==2)rm_hit(w->pcm,110,2080,710,89u+id,1);
 w->bytes=36+2*20000u;w->dataBytes=2*20000u;
}
static const RadioWave *rm_record(int track){
 static RadioWave archive[18];
 if(track<1||track>18)return 0;
 RadioWave *w=&archive[track-1];if(w->riff)return w;
 w->bytes=36+2*RM_FRAMES;w->wave=0x45564157;w->fmt=0x20746d66;w->fmtBytes=16;
 w->codec=w->channels=1;w->rate=RM_RATE;w->byteRate=2*RM_RATE;
 w->align=2;w->bits=16;w->data=0x61746164;w->dataBytes=2*RM_FRAMES;
 if(track==1){
  /* Seawall Nocturne: slow broken Ebmaj7 / Bbadd9, high answering piano,
   * and fine brushed snare on the offbeats. */
  static const unsigned short low[2][3]={{156,233,294},{117,220,262}};
  static const unsigned short high[]={392,349,294,262};
  for(int bar=0;bar<2;bar++)for(int n=0;n<3;n++)
   rm_note(w->pcm,bar*26000+n*1700,20500,low[bar][n],2350,0);
  for(int n=0;n<4;n++)rm_note(w->pcm,4000+n*12500,13100,high[n],5400,0);
  for(int n=0;n<4;n++)rm_hit(w->pcm,7900+n*12500,2800,1400,23u+n,1);
 }else if(track==2){
  /* Last Tram Home: syncopated D-major horn riff above double rail clicks. */
  static const unsigned short horn[][3]={{1,2,370},{4,1,440},{6,2,587},{9,1,554},{11,3,440},{16,2,392},{19,1,370},{21,2,330},{24,5,294}};
  for(unsigned n=0;n<sizeof horn/sizeof *horn;n++)rm_note(w->pcm,horn[n][0]*1800,horn[n][1]*1800,horn[n][2],4600,1);
  for(int n=0;n<8;n++){
   rm_note(w->pcm,n*6800,3800,n&1?110:147,3000,0);
   rm_hit(w->pcm,n*6800,370,4300,101u+n,0);
   rm_hit(w->pcm,n*6800+1150,220,3200,21u+n,0);
  }
 }else if(track<=14)rm_afterhours(w,track);
 else rm_refreshment(w,track);
 w->riff=0x46464952;return w;
}
#endif
