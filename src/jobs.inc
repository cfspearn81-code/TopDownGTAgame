/* Jobs own their vehicles and objectives for the current session. Save files
 * record the outcome and payout, with no reference to transient city actors. */
static const int courierRoutes[][2]={{2,3},{8,10},{11,2},{3,6},{10,8}};
static int courierRoute,sprintBest;
static const char *courier_objective(void){
 static char message[80];
 if(job.step==2)return "Return the signed manifest to Mara.";
 char*end=append(message,"Deliver the parcel to ");
 end=append(end,loc[courierRoutes[courierRoute][job.step]].label);append(end,".");
 return message;
}
static char*sprint_time(char*out,int tenths){
 out=number(out,tenths/10);out=append(out,".");out=number(out,tenths%10);return append(out,"s");
}
static int jobBuildings[2]={-1,-1};
static int rivalsDeployed;
enum{JOB_RIVAL_FIRST=STREET_CIVILIANS,JOB_RIVAL_COUNT=2};

static int job_venue(void){
 if(job.kind==JOB_COURIER)return job.step==2?0:courierRoutes[courierRoute][job.step];
 if(job.kind==JOB_VAULT&&job.step==1)return 1;
 if(job.kind==JOB_RIVALS&&job.step==1)return 7;
 return -1;
}
static int job_room(void){
 if(job.kind==JOB_SIGNAL||(job.kind==JOB_VAULT&&job.step==0))return jobBuildings[job.step];
 return -1;
}
enum{RESTITUTION_SHIFTS=3};
static int job_is_service(int kind){
 return sv.charges&&(kind==JOB_COURIER||kind==JOB_RECOVERY);
}
static int job_has_clock(void){
 return job.kind==JOB_TAXI||job.kind==JOB_HOT||(job.kind==JOB_SPRINT&&job.step>0);
}
static int job_vehicle_target(void){
 if(jobRide<0)return -1;
 switch(job.kind){
 case JOB_RECOVERY:
  if(job.step==0)return jobRide;
  break;
 case JOB_HOT:
  if(job.step==0)return jobRide;
  break;
 case JOB_TAXI:
 case JOB_SPRINT:
  if(!cars[jobRide].cond)return -1;
  if((job.kind==JOB_TAXI||job.step==0)&&p.car>=0&&cars[p.car].owner)return -1;
  break;
 default:return -1;
 }
 return p.car==jobRide?-1:jobRide;
}
static void end_job(void){
 if(job.kind==JOB_RIVALS){
  for(int i=JOB_RIVAL_FIRST;i<JOB_RIVAL_FIRST+JOB_RIVAL_COUNT;i++)peds[i]=(Ped){0};
 }
 job.kind=jobRide=-1;
 rivalsDeployed=0;
}
static void job_clock_tick(float dt){
 /* A dispatched loaner can be wrecked before pickup or before crossing
  * the start line. Never leave a gate or a passenger as an impossible GPS
  * objective. Prefer the registered ride, then another usable licensed car. */
 if((job.kind==JOB_TAXI||job.kind==JOB_SPRINT)&&job.step==0){
  if(jobRide<0||cars[jobRide].cond<=0||!cars[jobRide].owner){
   int best=-1;float closest=1.e10f;
   for(int i=0;i<nc;i++)if(cars[i].owner&&cars[i].cond>0){
    float d=d2(p.x-cars[i].x,p.y-cars[i].y);
    if(d<closest||i==p.car){closest=d;best=i;if(i==p.car)break;}
   }
   jobRide=best;
   if(best<0){end_job();tell("No drivable licensed car. Return to the contact for a new vehicle.");return;}
   tell(best==p.car?"Your current ride is licensed. Continue the job.":"Replacement available. GPS marks your licensed car.");
  }
 }
 if(job_has_clock()&&(job.time-=dt)<=0){
  end_job();tell("Job expired. Return to the contact to retry.");
 }
}
static void job_set_point(int step,int x,int y){
 job.xs[step]=x;job.ys[step]=y;
}
static void job_set_venue(int step,int venue){job_set_point(step,loc[venue].x,loc[venue].y);}
static void beginJob(int kind){
 if(ownedJob())return;
 static const int totals[]={3,2,2,6,2,2,2,2};
 static const int pay[]={115,0,160,205,260,270,300,360};
 static const float time[]={160,0,165,95,180,155,165,170};
 Job next={kind,0,totals[kind],pay[kind],time[kind],{0},{0},job_is_service(kind)};
 int ride=-1,x=(int)p.x,y=(int)p.y;
 Car vehicle={.type=0,.owner=2,.cond=100};

 switch(kind){
 case JOB_TAXI:case JOB_SPRINT:
  if(p.car<0||!cars[p.car].owner)ride=nc-1;
  if(kind==JOB_SPRINT)vehicle.type=1;
  break;
 case JOB_RECOVERY:
  ride=nc-1;vehicle.type=4;vehicle.cond=0;x=1792;y=1270;
  break;
 case JOB_HOT:
  ride=nc-1;vehicle.type=1;vehicle.owner=0;x=1360;y=1038;
  break;
 default:break;
 }
 if(ride>=0&&!street_find_car_space(ride,&vehicle,x,y,kind==JOB_HOT?30:0)){
  tell("The staging area is blocked. Try the job again shortly.");return;
 }
 if(kind==JOB_VAULT){
  jobBuildings[0]=contract_room(kind,840,1460,-1);
  if(jobBuildings[0]<0){tell("The parcel hall is unavailable.");return;}
 }
 if(kind==JOB_SIGNAL){
  jobBuildings[0]=contract_room(kind,950,1200,-1);
  jobBuildings[1]=contract_room(kind,430,1740,jobBuildings[0]);
  if(jobBuildings[0]<0||jobBuildings[1]<0){tell("A relay studio is unavailable.");return;}
 }
 if(kind==JOB_TAXI&&!taxi_offer()){tell("No fare available. Try again shortly.");return;}
 if(kind!=JOB_TAXI)job=next;
 jobRide=ride;
 navigation_follow_job();
 rivalsDeployed=0;
 if(ride>=0)street_stage_car(ride,vehicle);
 menu=-1;
 switch(kind){
 case JOB_COURIER:
  if(sv.intro==1)sv.intro=2;
  courierRoute=sv.intro<=2?0:sv.legal%LEN(courierRoutes);
  job_set_venue(0,courierRoutes[courierRoute][0]);job_set_venue(1,courierRoutes[courierRoute][1]);job_set_venue(2,0);
  if(courierRoute){
   int miles=0,from=0;
   for(int i=0;i<3;i++){int to=i==2?0:courierRoutes[courierRoute][i];
    miles+=taxi_road_distance(loc[from].x,loc[from].y,loc[to].x,loc[to].y);from=to;}
   job.price=100+miles/48;
  }
  tell(courier_objective());
  break;
 case JOB_TAXI:tell("Take a licensed car to the rider and stop at the curb.");break;
 case JOB_RECOVERY:
  job_set_point(0,(int)vehicle.x,(int)vehicle.y);
  job_set_point(1,loc[10].x,1011);
  tell("Dax issued a jump pack. Find the stalled work van.");
  break;
 case JOB_SPRINT:{
  const int startX=loc[6].x;
  static const int gates[][2]={{1280,256},{1280,512},{1024,512},{1024,256}};
  job_set_point(0,startX,256);
  for(int i=0;i<LEN(gates);i++)job_set_point(i+1,gates[i][0],gates[i][1]);
  job_set_point(job.total-1,startX,256);
  tell("Cross the start line in a licensed car to start the clock.");
  break;}
 case JOB_VAULT:
  job_set_point(0,ordinaryDoorX[jobBuildings[0]],ordinaryDoorY[jobBuildings[0]]);
  job_set_venue(1,1);
  tell("Take Ivo's lockbox from the marked parcel counter.");
  break;
 case JOB_HOT:
  job_set_point(0,(int)vehicle.x,(int)vehicle.y);
  job_set_point(1,loc[7].x,755);
  tell("Steal the marked Vectra. Deliver it to Kiosk 09 when clear.");
  break;
 case JOB_SIGNAL:
  for(int i=0;i<2;i++)job_set_point(i,ordinaryDoorX[jobBuildings[i]],ordinaryDoorY[jobBuildings[i]]);
  tell("Pull the relay cartridge from the marked radio studio.");
  break;
 case JOB_RIVALS:
  job_set_point(0,1558,1058);job_set_venue(1,7);
  tell("Recover the seized stock from the rival crew in East Freight.");
  break;
 }
}
static void advanceJob(void){
 if(job.kind<0||++job.step<job.total)return;
 int kind=job.kind,service=job_is_service(job.kind),unpaid=job.unpaid||service;
 int tip=kind==JOB_TAXI?taxi_tip_amount(job.price,taxi.crashes,taxi.damage,taxi.rough):0;
 int bonus=kind==JOB_COURIER&&sv.intro==2?25:0;
 char race[80];const char *result=jobNames[kind];
 if(kind<JOB_VAULT){
  sv.legal++;
  if(kind==JOB_TAXI)sv.taxiFares++;
  if(!unpaid){
   if(sv.intro<3){sv.intro=3;result="First paycheck. Your first car is $125 at Nightglass.";}
   else if(sv.intro==3)sv.intro=4;
  }
 }else sv.crime++;
 if(kind==JOB_SPRINT){
  int time=max(1,(int)((95.f-job.time)*10+.5f));
  int record=!sprintBest||time<sprintBest;
  if(record)sprintBest=time;
  char*end=sprint_time(append(race,"Finish "),time);
  if(record)append(end," / session best.");else sprint_time(append(end," / best "),sprintBest);
  result=race;
 }
 if(service){
  /* Count the final shift against the case before clearing it. An unpaid
   * shift must not spend the opening paycheck bonus or claim one was paid. */
  if(++sv.restitution>=RESTITUTION_SHIFTS){
   revenue_close_case();
   tell("Restitution cleared. Final service shift paid $0. Future shifts pay again.");
  }else{
   char msg[96];char*q=number(append(msg,"Community service: "),sv.restitution);
   q=number(append(q,"/"),RESTITUTION_SHIFTS);append(q," shifts complete. Shift paid $0.");
   tell(msg);
  }
  noticeInk=GOLD;
 }else if(unpaid){
  tell("Unpaid shift completed. Your case is already closed. Shift paid $0.");noticeInk=GOLD;
 }else gain(kind<JOB_VAULT,job.price+tip+bonus,result);
 end_job();request_save();
}
static int interior_job_drop(int venue){
 if(job.kind<0||job_venue()!=venue)return 0;
 if(job.kind==JOB_VAULT&&p.heat>0){
  tell("Ivo won't take the lockbox while police are looking for you.");return 1;
 }
 advanceJob();
 if(job.kind==JOB_COURIER)
  tell(courier_objective());
 return 1;
}
static const char *job_room_action(int building){
 if(job_room()!=building)return 0;
 if(job.kind==JOB_VAULT)return "Take the sealed lockbox";
 return job.step==0?"Remove the relay cartridge":"Install the cartridge in the backup relay";
}
static void job_use_room(void){
 int kind=job.kind,step=job.step;
 if(kind==JOB_VAULT){
  heat(105);advanceJob();tell("Alarm tripped. Lose police before handing the lockbox to Ivo.");
 }else if(step==0){
  heat(42);advanceJob();tell("Security alarm. Install the cartridge at the backup studio.");
 }else advanceJob();
}
static void job_rival_encounter(void){
 if(job.kind!=JOB_RIVALS||job.step!=0||rivalsDeployed)return;
 int x=job.xs[0],y=job.ys[0];
 if(d2(p.x-x,p.y-y)>104*104)return;
 static const int offsets[][2]={{-42,-19},{30,-39},{-65,25},{38,19},{-22,49},{-88,-10}};
 int guards=0;
 for(int i=0;i<LEN(offsets)&&guards<JOB_RIVAL_COUNT;i++){
  int gx=x+offsets[i][0],gy=y+offsets[i][1];
  if(!street_foot_clear(gx,gy,-1)||!street_line_open(x,y,gx,gy,3)||d2(p.x-gx,p.y-gy)<24*24)continue;
  peds[JOB_RIVAL_FIRST+guards++]=(Ped){gx,gy,look(p.x-gx,p.y-gy),0,2,2,0,0,1};
 }
 if(!guards)return;
 rivalsDeployed=1;
 tell("The rival crew is guarding the consignment.");
}
static int job_rivals_active(void){
 for(int i=JOB_RIVAL_FIRST;i<JOB_RIVAL_FIRST+JOB_RIVAL_COUNT;i++)if(peds[i].hp>0)return 1;
 return 0;
}
static int job_near_point(void){
 return job.kind>=0&&d2(p.x-job.xs[job.step],p.y-job.ys[job.step])<=35*35;
}
static const char *job_street_prompt(void){
 if(!job_near_point())return 0;
 if(job.kind==JOB_RECOVERY&&job.step==0&&p.car<0)return "E  RESTART WORK VAN";
 if((job.kind==JOB_RECOVERY||job.kind==JOB_HOT)&&job.step==1&&p.car==jobRide){
  if(job.kind==JOB_HOT&&p.heat>0)return "HANDOFF BLOCKED / LOSE THE POLICE";
  if(absf(cars[p.car].s)>7)return "STOP IN THE MARKED BAY";
  return job.kind==JOB_RECOVERY?"E  HAND OVER WORK VAN":"E  HAND OVER VECTRA";
 }
 if(job.kind==JOB_RIVALS&&job.step==0&&p.car<0&&rivalsDeployed&&!job_rivals_active())
  return "E  COLLECT CONSIGNMENT";
 return 0;
}
static int job_use_street(void){
 if(job.kind!=JOB_RECOVERY&&job.kind!=JOB_HOT&&job.kind!=JOB_RIVALS)return 0;
 if(!job_near_point())return 0;
 if(job.kind==JOB_RIVALS){
  if(job.step!=0||p.car>=0)return 0;
  if(!rivalsDeployed||job_rivals_active())tell("Deal with the guards before taking the stock.");
  else{advanceJob();tell("Consignment recovered. Return it to Vera at Kiosk 09.");}
  return 1;
 }
 if(job.kind==JOB_HOT&&job.step==0)return 0;
 if(job.kind==JOB_RECOVERY&&job.step==0){
  if(p.car>=0)return 0;
  Car *van=cars+jobRide;
  if(!street_line_open(p.x,p.y,van->x,van->y,2))return 0;
  van->cond=65;advanceJob();tell("Van running. Drive it to Dax's marked service bay.");
  return 1;
 }
 if(p.car!=jobRide){
  if(p.car>=0||street_near_car()==jobRide)return 0;
  tell("Bring the marked vehicle to the handoff.");return 1;
 }
 if(absf(cars[p.car].s)>7){tell("Stop inside the marked bay to hand over the vehicle.");return 1;}
 if(job.kind==JOB_HOT&&p.heat>0){tell("Lose the police before handing over the car.");return 1;}
 if(!street_exit_car()){tell("Leave room to open the door, then try the handoff again.");return 1;}
 // These cars occupy the reserved contract slot. The receiving crew takes
 // the vehicle out of circulation until another contract uses that slot.
 street_stage_car(jobRide,(Car){.x=-100,.y=-100});
 if(lastCar==jobRide)lastCar=-1;
 advanceJob();return 1;
}
static const char *job_objective(void){
 switch(job.kind){
 case JOB_COURIER:return courier_objective();
 case JOB_RECOVERY:return job.step==0?"Find the stalled van. Press E beside it to restart it.":
     p.car!=jobRide?"Get into the recovered van.":"Stop in Dax's marked service bay and press E.";
 case JOB_SPRINT:return p.car<0||!cars[p.car].owner?"Enter a licensed car for the sprint.":
     job.step==0?"Cross the start line to begin timing.":
     p.car!=jobRide?"Return to the car you started the sprint in.":"Follow the marked circuit to the finish.";
 case JOB_VAULT:return job.step==0?"Take the lockbox from the marked parcel counter.":
     p.heat>0?"Lose the police with the lockbox.":"Hand the lockbox to Ivo at Vault 39.";
 case JOB_HOT:return job.step==0?"Steal the marked Vectra.":
     p.car!=jobRide?"Return to the marked Vectra.":
     p.heat>0?"Lose the police with the marked car.":"Stop at Kiosk 09's marked bay and press E.";
 case JOB_SIGNAL:return job.step==0?"Take the cartridge from the radio console.":
     "Install the cartridge at the backup studio.";
 case JOB_RIVALS:return job.step==1?"Return the recovered stock to Vera at Kiosk 09.":
     !rivalsDeployed?"Find the crew guarding the East Freight cache.":
     job_rivals_active()?"Deal with the guards at the cache.":"Get out and press E to collect the stock.";
 default:return 0;
 }
}
/* Sprint gates are the only jobs scored by swept movement. */
static void check_arrival(float ox,float oy){
 if(job.kind!=JOB_SPRINT||p.dead||p.car<0||!cars[p.car].owner||absf(cars[p.car].s)<15)return;
 if(job.step>0&&p.car!=jobRide)return;
 float tx=job.xs[job.step],ty=job.ys[job.step];
 float dx=p.x-ox,dy=p.y-oy,ds=d2(dx,dy);
 float t=ds>0?((tx-ox)*dx+(ty-oy)*dy)/ds:0;
 if(t<0)t=0;if(t>1)t=1;
 if(d2(ox+t*dx-tx,oy+t*dy-ty)>34.f*34.f)return;
 if(job.step==0){jobRide=p.car;tell("Sprint clock started. Follow the gates around the block.");}
 advanceJob();
}
