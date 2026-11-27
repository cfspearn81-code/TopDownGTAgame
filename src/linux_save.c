#define _POSIX_C_SOURCE 200809L
#include "linux_save.h"
#include "save_receipt_layout.h"
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>

static int dirfd=-1,lockfd=-1;
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
int linux_disk_lock(void){
 if(dirfd<0){errno=EBADF;return 0;}
 if(lockfd>=0)return 1;
 lockfd=openat(dirfd,".afterhours.lock",O_RDWR|O_CREAT|O_CLOEXEC|O_NOFOLLOW|O_NONBLOCK,0600);
 if(lockfd<0)return 0;
 if(flock(lockfd,LOCK_EX|LOCK_NB)==0)return 1;
 int saved=errno;close(lockfd);lockfd=-1;errno=saved;return 0;
}
void linux_disk_shutdown(void){
 if(lockfd>=0)close(lockfd);lockfd=-1;
 if(dirfd>=0)close(dirfd);dirfd=-1;
}

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
static int write_local(const char *name,const void *bytes,unsigned size,int replace){
 if(dirfd<0||!name[0])return 0;
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
  if(replace)ok=renameat(dirfd,stage,dirfd,name)==0;
  else ok=linkat(dirfd,stage,dirfd,name,0)==0;
 }
 if(!ok||!replace)(void)unlinkat(dirfd,stage,0);
 if(ok)(void)fsync(dirfd);
 return ok;
}
static int write_slot(const void *bytes,unsigned size,int replace){
 return write_local(save_name,bytes,size,replace);
}
int linux_export_frame_times(const void *bytes,unsigned size){
 return write_local("frame-times.csv",bytes,size,1);
}
/* Keep the verified prior-format bytes on the first expanded write. The
 * filename includes the old checksum so a re-used slot never overwrites a
 * previous career's backup. Hidden backups do not enter the slot browser. */
static int read_exact_fd(int fd, unsigned char *out, size_t size){
 size_t off=0;
 while(off<size){
  ssize_t count=pread(fd,out+off,size-off,(off_t)off);
  if(count<0&&errno==EINTR)continue;
  if(count<=0)return 0;
  off+=(size_t)count;
 }
 return 1;
}
static int matches_backup(const char *name,const unsigned char *old,size_t n){
 int fd=openat(dirfd,name,O_RDONLY|O_CLOEXEC|O_NOFOLLOW|O_NONBLOCK);
 if(fd<0)return 0;
 unsigned char actual[1044];struct stat st;
 int ok=n<=sizeof actual&&!fstat(fd,&st)&&S_ISREG(st.st_mode)&&st.st_size==(off_t)n&&
        read_exact_fd(fd,actual,n)&&!memcmp(actual,old,n);
 return !close(fd)&&ok;
}
static uint32_t migration_hash(const unsigned char *buf,size_t n){
 uint32_t h=2166136261u;
 while(n--)h=(h^*buf++)*16777619u;
 return h;
}
int linux_disk_backup_upgrade(int stock_patches){
 if(dirfd<0||!save_name[0])return 0;
 int fd=openat(dirfd,save_name,O_RDONLY|O_CLOEXEC|O_NOFOLLOW|O_NONBLOCK);
 if(fd<0)return errno==ENOENT;
 struct stat st;
 if(fstat(fd,&st)||!S_ISREG(st.st_mode)){close(fd);return 0;}
 if(st.st_size>304&&st.st_size!=1044){close(fd);return 0;}
 uint32_t family=0,receipt=0;
 if(st.st_size==1044){
  if(pread(fd,&family,sizeof family,DC_SAVE11_FAMILY_AT)!=sizeof family||
     pread(fd,&receipt,sizeof receipt,DC_SAVE11_OLD_SEIZURE_AT)!=sizeof receipt){close(fd);return 0;}
  if((family&0x80000000u)&&!dc_legacy_receipt_marker(receipt)){
   unsigned char patches;uint32_t hybrid;
   if(pread(fd,&patches,sizeof patches,267)!=sizeof patches||
      pread(fd,&hybrid,sizeof hybrid,684)!=sizeof hybrid){close(fd);return 0;}
   /* Old Base Village cannot read a stocked extension. Preserve the
      existing compatibility backup only on the first such transition. */
   if(!stock_patches||patches||(hybrid&0x80000000u)){close(fd);return 1;}
  }
 }
 /* Snapshot the actual file once, not the pathname after close. Never
    replace a conflicting backup left by a failed restore or file copy. */
 unsigned char old[1044];
 uint32_t header[2],digest,base_digest;
 if(st.st_size<176||st.st_size>(off_t)sizeof old){close(fd);return 0;}
 size_t len=(size_t)st.st_size;
 int ok=read_exact_fd(fd,old,len);
 if(close(fd)||!ok)return 0;
 memcpy(header,old,sizeof header);memcpy(&digest,old+len-4,4);
 if(header[0]!=0x44433131u||header[1]<2||header[1]>11||
    (header[1]==11?(len!=1044&&len!=304):len>304)||
    migration_hash(old,len-4)!=digest)return 0;
 if(len==1044){memcpy(&base_digest,old+300,4);if(migration_hash(old,300)!=base_digest)return 0;}
 char dest[60];
 const char* label=header[1]!=11?"afterhours":dc_legacy_receipt_marker(receipt)?"stability":len==1044&&!(family&0x80000000u)?"outerloop":"crossroads";
 snprintf(dest,sizeof dest,".pre-%s-%s-r%u-%08x.bak",label,save_name,header[1],digest);
 /* A previous backup may legitimately be a distinct inode. Retry only
    accepts byte-identical, regular files; a conflicting file is untouched. */
 if(!matches_backup(dest,old,len)&&!write_local(dest,old,(unsigned)len,0)&&!matches_backup(dest,old,len))return 0;
 return fsync(dirfd)==0||errno==EINVAL||errno==ENOTSUP;
}
int linux_disk_backup_legacy(void){return linux_disk_backup_upgrade(0);}
int disk_write(const void *bytes,unsigned size){return write_slot(bytes,size,1);}
int disk_create(const void *bytes,unsigned size){return write_slot(bytes,size,0);}
