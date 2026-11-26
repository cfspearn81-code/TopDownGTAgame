/* Linux regression shim around the *same* native disk functions used by the
 * Windows executable. Real processes/files, simulated Win32: not an NTFS test. */
#define _GNU_SOURCE
#include "../../src/windows.c"
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

enum Fault { NONE, OPEN_DENIED, WRITE_FAILED, SHORT_WRITE, CLOSE_FAILED, MOVE_FAILED, ORPHAN };
static enum Fault fault;
static U error;
static char heldName[1024];
static int ready=-1,go=-1;

static void path(const U16 *in,char s[1024]){
 unsigned n=0;
 while(*in){assert(n<1023&&*in<128);s[n++]=*in=='\\'?'/':(char)*in;in++;}
 s[n]=0;
}
static U pid(void){return (U)getpid();}
static U last(void){return error;}
static U env(const U16*k,U16*out,U cap){return 0;}
static int directory(const U16*k,void*p){char s[1024];path(k,s);return mkdir(s,0700)==0;}
static Handle create(const U16 *p,U access,U share,void*sec,U mode,U attr,Handle h){
 char s[1024];path(p,s);
 int flags;
 if(mode==1){
  assert(access==0x40000000u&&!share);
  if(fault==ORPHAN){int fd=open(s,O_CREAT|O_EXCL|O_WRONLY,0600);assert(fd>=0&&write(fd,"orphan",6)==6&&!close(fd));strcpy(heldName,s);fault=NONE;}
  flags=O_WRONLY|O_CREAT|O_EXCL;
 }else{assert(mode==3&&access==0x80000000u&&share==5);flags=O_RDONLY;}
 if(fault==OPEN_DENIED){error=5;return (Handle)-1;}
 int fd=open(s,flags,0600);
 if(fd<0){error=errno==ENOENT?2:errno==EEXIST?80:5;return (Handle)-1;}
 return (Handle)(intptr_t)(fd+1);
}
static int rd(Handle h,void*d,U n,U *got,void*over){
 ssize_t k=read((int)(intptr_t)h-1,d,n);*got=k<0?0:(U)k;return k>=0;
}
static int wr(Handle h,const void*d,U n,U *got,void*over){
 if(fault==WRITE_FAILED){*got=0;return 0;}
 ssize_t k=write((int)(intptr_t)h-1,d,fault==SHORT_WRITE?n-1:n);
 *got=k<0?0:(U)k;return k>=0;
}
static int closeh(Handle h){return !close((int)(intptr_t)h-1)&&fault!=CLOSE_FAILED;}
static int del(const U16*p){char s[1024];path(p,s);return !unlink(s);}
static int mv(const U16 *a,const U16*b,U f){
 char src[1024],dest[1024];path(a,src);path(b,dest);assert(f==8||f==9);
 if(ready!=-1){
  char c=1;assert(write(ready,&c,1)==1&&read(go,&c,1)==1);ready=-1;
 }
 if(fault==MOVE_FAILED)return 0;
 if(f==9)return !rename(src,dest);
 /* Install without replacement; never expose a partial destination. */
 if(link(src,dest))return 0;
 assert(!unlink(src));return 1;
}
U (*__imp_GetCurrentProcessId)(void)=pid;
U (*__imp_GetLastError)(void)=last;
U (*__imp_GetEnvironmentVariableW)(const U16*,U16*,U)=env;
int (*__imp_CreateDirectoryW)(const U16*,void*)=directory;
Handle (*__imp_CreateFileW)(const U16*,U,U,void*,U,U,Handle)=create;
int (*__imp_ReadFile)(Handle,void*,U,U*,void*)=rd;
int (*__imp_WriteFile)(Handle,const void*,U,U*,void*)=wr;
int (*__imp_CloseHandle)(Handle)=closeh;
int (*__imp_DeleteFileW)(const U16*)=del;
int (*__imp_MoveFileExW)(const U16*,const U16*,U)=mv;
static const char prior[]="old!",one[]="1111",two[]="2222";
static void expect(const char *s){
 char b[sizeof prior];assert(disk_read(b,sizeof b)&&!memcmp(s,b,sizeof b));
}
static int files(void){
 DIR*d=opendir("DeadChannel");assert(d);int n=0;struct dirent*e;
 while((e=readdir(d)))if(strcmp(e->d_name,".")&&strcmp(e->d_name,".."))n++;
 assert(!closedir(d));return n;
}
static void race(int slot,int newCareer,int failPeer){
 disk_slot(slot);assert(!disk_present()&&!files());
 if(!newCareer)assert(disk_write(prior,sizeof prior));
 int p[2],q[2];assert(!pipe(p)&&!pipe(q));
 pid_t child=fork();assert(child>=0);
 if(!child){
  alarm(10);close(p[0]);close(q[1]);ready=p[1];go=q[0];
  int ok=newCareer?disk_create(one,sizeof one):disk_write(one,sizeof one);
  _exit(ok==(!newCareer||failPeer)?0:1);
 }
 close(p[1]);close(q[0]);char c;assert(read(p[0],&c,1)==1);
 fault=failPeer?MOVE_FAILED:NONE;
 int ok=newCareer?disk_create(two,sizeof two):disk_write(two,sizeof two);
 assert(ok==!failPeer);fault=NONE;
 if(ok)expect(two);else if(!newCareer)expect(prior);else assert(!disk_present());
 assert(write(q[1],&c,1)==1);close(q[1]);close(p[0]);
 int status;assert(waitpid(child,&status,0)==child&&WIFEXITED(status)&&!WEXITSTATUS(status));
 expect(newCareer&&!failPeer?two:one);assert(files()==1&&disk_delete()&&!files());
}
int main(void){
 alarm(40);char cwd[]="/tmp/dead-channel-disk-XXXXXX";assert(mkdtemp(cwd)&&!chdir(cwd));directories();
 assert(!disk_present()&&disk_create(prior,sizeof prior));expect(prior);
 char buf[16];assert(!disk_read(buf,sizeof prior-1)&&!disk_read(buf,sizeof prior+1));
 assert(!disk_create(one,sizeof one)&&files()==1);expect(prior);
 for(fault=OPEN_DENIED;fault<=MOVE_FAILED;){
  enum Fault current=fault;
  if(fault==OPEN_DENIED)assert(disk_present()); /* locked/unreadable is occupied */
  assert(!disk_write(one,sizeof one));
  fault=NONE;expect(prior);assert(files()==1);fault=current+1;
 }
 fault=ORPHAN;assert(disk_write(one,sizeof one));expect(one);
 int fd=open(heldName,O_RDONLY);assert(fd>=0&&read(fd,buf,16)==6&&!memcmp(buf,"orphan",6)&&!close(fd));
 assert(files()==2&&!unlink(heldName)&&disk_delete());
 for(int slot=0;slot<=251;slot+=(slot==0?1:slot==1?1:249))
  for(int fresh=0;fresh<2;fresh++)for(int fail=0;fail<2;fail++)race(slot,fresh,fail);
 assert(!rmdir("DeadChannel")&&!chdir("/")&&!rmdir(cwd));
 puts("Disk regression: exact length, denied/short/failed I/O, prior save, orphan reservation, and 16 two-process commit races on four slot paths passed.");
}
