/* Control-only long tours: purchase, walk to a physical staff gate/counter,
 * and stay for a complete active-play pay/production cycle. */
#include "route_prefix.inc"
#include <assert.h>
static void choose(int action){
 makeMenu();int row=-1;for(int i=0;i<nopt;i++)if(opt[i].action==action){row=i;break;}
 if(row<0){fprintf(stderr,"missing action %d menu %d scene %d: %s\n",action,menu,scene,notice);abort();}
 for(int guard=0;sel!=row&&guard<=nopt;guard++)press_key(DOWN);
 assert(sel==row);press_key(ENTER);
}
int main(int argc,char**argv){
 assert(argc==3);
 int grow=!strcmp(argv[1],"grow"),cinder=!strcmp(argv[1],"cinder"),li=-1;
 if(!strcmp(argv[1],"legal"))li=0;else if(!strcmp(argv[1],"ferry"))li=1;else if(!strcmp(argv[1],"wrecker"))li=2;
 assert(grow||cinder||li>=0);int v=grow?4:cinder?5:legal_specs[li].kind;
 set_session_seed(17);init_city();starter_save();sv.intro=4;sv.cash=grow?3440:cinder?10600:legal_specs[li].cost+175;
 sv.dirty=grow||cinder?33:legal_specs[li].limit;sv.bank=400;sv.units=grow?3:cinder?6:0;sv.dayClock=1360;
 sv.x=loc[v].x;sv.y=loc[v].y;sv.hash=savehash();FILE*f=fopen(argv[2],"wb");assert(f&&fwrite(&sv,sizeof sv,1,f)==1&&!fclose(f));
 profiles_activate();akeys[total++]=0;akeys[total++]=ENTER;
 for(int i=0;i<4;i++)record(0);press_key(USE);assert(scene==3&&interior_active_loc==v);g_kind=loc[v].kind;room_name=loc[v].label;
 go(DESK_G);press_key(USE);assert(menu==g_kind);
 if(grow||cinder)choose(cinder?41:40);else {choose(LEGAL_OPEN);choose(LEGAL_BUY_FIRST+li);}
 assert(grow?sv.b[0].own:cinder?sv.b[1].own:sv.legit[li].owned);
 if(!cinder){press_key(ESC);go(GATE_G);press_key(USE);assert(interior_subroom);go(DESK_G);press_key(USE); }
 if(grow){assert(menu==4);choose(44);assert(sv.b[0].timer>0&&sv.units==2);choose(50);
  assert(sv.b[0].crew==1&&sv.cash==310);int n=0;while(!sv.stocks[0]&&n++<7300)record(0);
  assert(n>7100&&n<7250&&sv.stocks[0]==5&&sv.units==1&&sv.b[0].batches==1&&sv.b[0].timer>117.f);
 }else if(cinder){
  assert(menu==5);choose(45);choose(51);choose(51);
  assert(sv.b[1].crew==2&&sv.cash==260&&sv.units==4);
  int n=0;while(!sv.b[1].batches&&n++<7300)record(0);
  assert(n>7100&&n<7250&&sv.b[1].batches==1&&sv.b[1].timer>117.f);
  assert(sv.dirty==304&&sv.sales==6&&sv.units==2&&!sv.stocks[2]);
 }else{assert(menu==LEGAL_REAR_FIRST+li);choose(LEGAL_DEPOSIT_FIRST+li);
  int net=(int[]){165,240,360}[li];assert(sv.legit[li].pending==net&&sv.dirty==0);
  int n=0;while(sv.legit[li].cooldown>0&&n++<9000)record(0);
  assert(!sv.legit[li].cooldown);choose(LEGAL_CLAIM_FIRST+li);assert(sv.cash==175+net);
  n=0;while(sv.cash==175+net&&n++<7400)record(0);assert(sv.cash==175+net+legal_specs[li].payout&&!sv.legit[li].pending);
 }
 press_key(ESC);press_key(SAVE);
 printf("%d 0\n",total);for(int i=0;i<total;){int j=i+1;while(j<total&&akeys[j]==akeys[i])j++;printf("%d %u\n",j-i,akeys[i]);i=j;}
 fprintf(stderr,"%s %d actual input frames; clean %d dirty %d bank %d units %d stock %d\n",room_name,total,sv.cash,sv.dirty,sv.bank,sv.units,sv.stocks[0]);
}
