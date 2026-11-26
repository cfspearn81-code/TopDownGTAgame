#define _POSIX_C_SOURCE 200809L
#include "../../src/linux_save.h"
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static void no_stages(const char *folder){
 DIR*d=opendir(folder);assert(d);
 for(struct dirent*e;(e=readdir(d));)assert(strncmp(e->d_name,".stage-",7));
 closedir(d);
}
int main(int argc,char **argv){
 assert(argc==2);
 char layout[1024],payload[304],loaded[304];
 const char*tmp=argv[1];assert(setenv("XDG_DATA_HOME",tmp,1)==0&&linux_disk_init());
 assert(snprintf(layout,sizeof layout,"%s/DeadChannel",tmp)<(int)sizeof layout);
 int parent=open(layout,O_DIRECTORY|O_RDONLY);assert(parent>=0);
 assert(!disk_present());
 memset(payload,0x35,sizeof payload);assert(disk_create(payload,sizeof payload));
 assert(!disk_create("different",9));
 assert(!disk_read(loaded,301)&&disk_read(loaded,304)&&!memcmp(loaded,payload,304));
 struct stat st;assert(fstatat(parent,"career.dat",&st,0)==0&&!(st.st_mode&077));
 memset(payload,0xa5,sizeof payload);assert(disk_write(payload,sizeof payload));
 assert(disk_read(loaded,304)&&!memcmp(loaded,payload,304));
 disk_slot(1);assert(!disk_present()&&symlinkat("career.dat",parent,"opening.dat")==0);
 assert(disk_present()&&!disk_read(loaded,304)&&!disk_create("no",2)&&disk_delete());
 disk_slot(2);assert(mkdirat(parent,"fresh-001.dat",0700)==0);
 assert(disk_present()&&!disk_write(payload,304)&&!disk_read(loaded,304));
 no_stages(layout);
 assert(unlinkat(parent,"fresh-001.dat",AT_REMOVEDIR)==0);close(parent);
 for(int pass=0;pass<16;pass++){
  /* Forked instances race to create the same fresh profile. Only one of
   * the two inode links may win; both stages must be cleaned up. */
  pid_t child=fork();assert(child>=0);memset(payload,child?'A':'B',304);
  int won=disk_create(payload,sizeof payload);
  if(!child)_exit(won?0:1);
  int result;assert(waitpid(child,&result,0)==child&&WIFEXITED(result));
  assert(won+(WEXITSTATUS(result)==0)==1&&disk_read(loaded,304));
  assert(loaded[0]==(won?'A':'B'));
  for(int i=1;i<304;i++)assert(loaded[i]==loaded[0]);
  assert(disk_delete());no_stages(layout);
 }
 linux_disk_shutdown();
 assert(setenv("XDG_DATA_HOME","relative-path-must-not-be-used",1)==0&&setenv("HOME",tmp,1)==0&&linux_disk_init());
 assert(!disk_present()&&disk_create(payload,304));
 assert(snprintf(layout,sizeof layout,"%s/.local/share/DeadChannel/career.dat",tmp)<(int)sizeof layout);
 assert(stat(layout,&st)==0&&st.st_size==304);
 linux_disk_shutdown();
 puts("POSIX disk: XDG and HOME fallback, exact length, private permissions, failed replacement, symlink refusal, 16 process races.");
}
