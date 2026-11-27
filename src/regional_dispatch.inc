/* Atlas solver scratch is shared; dispatch keeps an independent route and
   always restores the personal pin, polyline, and displayed distance. Keep
   these emergency foot routes near the kerb instead of in traffic whenever
   a collision-tested, continuous offset exists along the surveyed road. */
static int dispatch_region_clear(DispatchPoint a,DispatchPoint b){
 return street_line_open(a.x,a.y,b.x,b.y,5)&&!night_phone_lane_stop(a.x,a.y,b.x,b.y,4);
}
static int dispatch_region_add(DispatchPoint *v,int n,int cap,DispatchPoint p){
 if(n>=cap)return n;
 if(!n||d2(v[n-1].x-p.x,v[n-1].y-p.y)>=.5f)v[n++]=p;
 return n;
}
static int dispatch_regional_plan(DispatchPoint from,DispatchPoint to,DispatchPoint*out,int cap){
 NavPoint saved[LEN(navNodes)];int oldN=navCount,oldD=navDist,n=0;
 cp(saved,navNodes,sizeof saved);
 nav_solve((NavPoint){(int)from.x,(int)from.y},(NavPoint){(int)to.x,(int)to.y});
 if(navCount>=2){
  n=dispatch_region_add(out,n,cap,from);
  for(int k=1;k<navCount;k++){
   if(n+3>=cap){n=0;break;}
   DispatchPoint a={navNodes[k-1].x,navNodes[k-1].y},b={navNodes[k].x,navNodes[k].y};
   float dx=b.x-a.x,dy=b.y-a.y,len=sq(d2(dx,dy));
   int used=0;
   if(len>=48.f){
    /* A short on-ramp from the prior segment and an off-ramp to the next
       make a true continuous walk. The shared graph rejects closed links. */
    float ux=dx/len,uy=dy/len,t=len<80.f?len*.25f:20.f;
    for(int side=1;side>=-1&&!used;side-=2){
     float vx=-uy*22.f*side,vy=ux*22.f*side;
     DispatchPoint a1={a.x+ux*t+vx,a.y+uy*t+vy},b1={b.x-ux*t+vx,b.y-uy*t+vy};
     if(!dispatch_region_clear(out[n-1],a1)||!dispatch_region_clear(a1,b1)||!dispatch_region_clear(b1,b))continue;
     n=dispatch_region_add(out,n,cap,a1);n=dispatch_region_add(out,n,cap,b1);used=1;
    }
   }
   if(!dispatch_region_clear(out[n-1],b)){n=0;break;}
   n=dispatch_region_add(out,n,cap,b);
  }
  if(n&&dispatch_region_clear(out[n-1],to))n=dispatch_region_add(out,n,cap,to);
  else n=0;
 }
 cp(navNodes,saved,sizeof saved);navCount=oldN;navDist=oldD;
 return n;
}
