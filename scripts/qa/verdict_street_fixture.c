#define DC_TEST
#include "game.c"
#include <stdio.h>
int main(int argc,char**argv){init_city();sv.intro=4;sv.legal=1;sv.x=argc>1?1748:1482;sv.y=argc>1?1067:992;sv.dirty=0;sv.cash=0;sv.magic=0x44433131;sv.rev=10;sv.hash=savehash();return fwrite(&sv,sizeof sv,1,stdout)!=1;}
