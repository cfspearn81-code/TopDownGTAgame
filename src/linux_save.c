#define _POSIX_C_SOURCE 200809L
#include "linux_save.h"
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int dirfd=-1;
static unsigned stage_sequence;
static char save_name[16]="career.dat";

void disk_slot(int slot){
 if(slot<=0)strcpy(save_name,"career.dat");
 else if(slot==1)strcpy(save_name,"opening.dat");
 else if(slot<=251)snprintf(save_name,sizeof save_name,"fresh-%03d.dat",slot-1);
 else save_name[0]=0;
}

/* Unlike the Windows host, the POSIX user-data hierarchy may not exist.
 * Never use a relative XDG directory or fall back to the game/install folder. */
static int make_dirs(char *absolute){
 for(char *p=absolute+1;;p++){
  if(*p&&*p!='/')continue;
  char separator=*p;*p=0;
  int status=mkdir(absolute,0700),saved=errno;
  *p=separator;
  if(status<0&&saved!=EEXIST){errno=saved;return 0;}
  if(!separator)return 1;
 }
}
int linux_disk_init(void){
 const char *base=getenv("XDG_DATA_HOME"),*leaf="/DeadChannel";
 if(!base||*base!='/'){
  base=getenv("HOME");
  if(!base||*base!='/'){
   struct passwd *account=getpwuid(getuid());
   base=account?account->pw_dir:NULL;
  }
  if(!base||*base!='/'){errno=ENOENT;return 0;}
  leaf="/.local/share/DeadChannel";
 }
 size_t n=strlen(base)+strlen(leaf)+1;
 char *path=malloc(n);
 if(!path)return 0;
 snprintf(path,n,"%s%s",base,leaf);
 int fd=make_dirs(path)?open(path,O_RDONLY|O_DIRECTORY|O_CLOEXEC|O_NOFOLLOW):-1;
 int saved=errno;free(path);errno=saved;
 if(fd<0)return 0;
 if(dirfd>=0)close(dirfd);
 dirfd=fd;disk_slot(0);return 1;
}
void linux_disk_shutdown(void){if(dirfd>=0)close(dirfd);dirfd=-1;}

int disk_present(void){
 struct stat info;
 if(dirfd<0||!save_name[0])return 1;
 if(fstatat(dirfd,save_name,&info,AT_SYMLINK_NOFOLLOW)==0)return 1;
 /* Do not reuse an unreadable, nonregular or otherwise suspicious slot. */
 return errno!=ENOENT;
}
int disk_delete(void){
 if(dirfd<0||!save_name[0]||unlinkat(dirfd,save_name,0)!=0)return 0;
 (void)fsync(dirfd);return 1;
}
int disk_read(void *bytes,unsigned size){
 if(dirfd<0||!save_name[0])return 0;
 int fd=openat(dirfd,save_name,O_RDONLY|O_CLOEXEC|O_NOFOLLOW|O_NONBLOCK);
 if(fd<0)return 0;
 struct stat st;
 int ok=fstat(fd,&st)==0&&S_ISREG(st.st_mode)&&st.st_size==(off_t)size;
 unsigned char *out=bytes,extra;
 unsigned remain=size;
 while(ok&&remain){
  ssize_t got=read(fd,out,remain);
  if(got<0&&errno==EINTR)continue;
  if(got<=0){ok=0;break;}
  out+=got;remain-=(unsigned)got;
 }
 if(ok){
  ssize_t n;
  do{n=read(fd,&extra,1);}while(n<0&&errno==EINTR);
  ok=n==0;
 }
 if(close(fd)<0)ok=0;
 return ok;
}
static int write_slot(const void *bytes,unsigned size,int replace){
 if(dirfd<0||!save_name[0])return 0;
 char stage[64];int fd=-1;
 for(int retry=0;retry<64;retry++){
  snprintf(stage,sizeof stage,".stage-%ld-%08x",(long)getpid(),++stage_sequence);
  fd=openat(dirfd,stage,O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC|O_NOFOLLOW,0600);
  if(fd>=0)break;
  if(errno!=EEXIST)return 0;
 }
 if(fd<0)return 0;
 const unsigned char *src=bytes;unsigned remain=size;
 while(remain){
  ssize_t n=write(fd,src,remain);
  if(n<0&&errno==EINTR)continue;
  if(n<=0)break;
  src+=n;remain-=(unsigned)n;
 }
 int ok=!remain&&fsync(fd)==0;
 if(close(fd)<0)ok=0;
 if(ok){
  /* Atomic no-replace for new careers: only a newly created directory
   * link can win. Existing saves replace atomically within the same dir. */
  if(replace)ok=renameat(dirfd,stage,dirfd,save_name)==0;
  else ok=linkat(dirfd,stage,dirfd,save_name,0)==0;
 }
 if(!ok||!replace)(void)unlinkat(dirfd,stage,0);
 if(ok)(void)fsync(dirfd);
 return ok;
}
int disk_write(const void *bytes,unsigned size){return write_slot(bytes,size,1);}
int disk_create(const void *bytes,unsigned size){return write_slot(bytes,size,0);}
