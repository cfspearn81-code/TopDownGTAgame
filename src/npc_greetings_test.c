#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "npc_greetings.inc"
static int lenmax;
static void validate(const char *s){
 int n=(int)strlen(s);
 assert(n>=5&&n<=62);
 for(int i=0;i<n;i++)assert((unsigned char)s[i]>=32&&(unsigned char)s[i]<127);
 if(n>lenmax)lenmax=n;
}
int main(void){
 const char *all[410];int n=0;
 for(int j=0;j<NPC_VOICE_CITY_POOL;j++)all[n++]=npc_voice_city[j];
 for(int i=0;i<NPC_VOICE_ROLES;i++){
  for(int j=0;j<NPC_VOICE_ROLE_POOL;j++)all[n++]=npc_voice_roles[i][j];
  for(int v=-50;v<50;v++){
   const char *s=npc_greet_text(i,v);validate(s);
   assert(s==npc_greet_text(i,v+50));
   for(int k=1;k<50;k++)assert(strcmp(s,npc_greet_text(i,v+k)));
  }
 }
 assert(n==410);
 for(int i=0;i<n;i++){validate(all[i]);for(int j=0;j<i;j++)assert(strcmp(all[i],all[j]));}
 /* 6,000 real-looking + edge IDs. Check role/table consistency, cycle
    boundary, repeat-free selection, and personal retrieval with no state. */
 for(int r=0;r<NPC_VOICE_ROLES;r++)for(unsigned i=0;i<600;i++){
  unsigned id=i<300?i:UINT_MAX-599u+i;
  const char *heard[50];int role_lines=0;
  for(unsigned j=0;j<50;j++){
   const char *s=npc_greet_line(r,id,j);int slot=npc_greet_variant(id,j);
   assert(slot>=0&&slot<50&&npc_greet_variant(id,j+50)==slot);
   for(unsigned k=0;k<j;k++){assert(strcmp(s,heard[k]));assert(npc_greet_variant(id,k)!=slot);}
   heard[j]=s;
   int hit=0;for(int k=0;k<36;k++)hit|=!strcmp(s,npc_voice_roles[r][k]);
   role_lines+=!!hit;
   assert(s==npc_greet_line(r,id,j+50u)&&s==npc_greet_line(r,id,j+100u));
  }
  assert(role_lines==32&&strcmp(heard[49],npc_greet_line(r,id,50)));
 }
 for(int i=0;i<50;i++)assert(npc_greet_line(-1,42u,i)==npc_greet_line(0,42u,i)&&npc_greet_text(999,i)==npc_greet_text(0,i));
 printf("PASS 410 globally distinct lines, max %d ASCII characters; 6000 distinct role/identity cases x 50 no-repeat visits and two full repeats.\n",lenmax);
 return 0;
}
