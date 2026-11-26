enum {PULSE_MUZZLE_INK=0x9fcbe1,PULSE_SLIDE_INK=0x101c2d,PULSE_SLIDE_FACE=0x526984,PULSE_SLIDE_GLINT=0x9ab1c0};
static int pulse_pixel(float o){return (int)(o+(o<0?-.5f:.5f));}
static Pt pulse_ink_at(PulseSprite g,Pt p,float axial,float lateral){
 /* Local frame of the on-screen barrel; its cross section turns with it.
    A fixed -Y highlight would collapse along the north/south barrel. */
 return (Pt){p.x+pulse_pixel(g.co*axial-g.si*lateral),p.y+pulse_pixel(g.si*axial+g.co*lateral)};
}
static PulseSprite pulse_project(PulsePose g,int dx,int dy){
 PulseSprite p={.muzzle={(int)g.mx-dx,(int)g.my-dy},.co=g.co,.si=g.si,.hand=g.hand};
 p.slide=pulse_ink_at(p,p.muzzle,g.hand?-7.f:-9.f,0.f);
 float side=pulse_shoulder(g.co,g.si);
 int primary=side<-1.5f?-1:1;
 p.grip=pulse_ink_at(p,p.muzzle,g.hand?-5.5f:-7.f,g.hand?0.f:1.5f*primary);p.grip.y+=1;
 /* For the north shoulder pose the support hand crosses the chest
    closer to the grip, not all the way to the nose of the pistol. */
 float reach=4.f+.51f*absf(side);
 p.support=pulse_ink_at(p,p.muzzle,-reach,-1.5f*primary);p.support.y+=1;
 return p;
}
static void pulse_barrel(PulseSprite g,Pt back,Pt nose,float r1,float r2,U c){
 quad(pulse_ink_at(g,back,0,-r1),pulse_ink_at(g,nose,0,-r2),
      pulse_ink_at(g,nose,0,r2),pulse_ink_at(g,back,0,r1),c);
}
static void pulse_pistol_paint(const PulseSprite*g){
 const PulseSprite q=*g;Pt back=q.slide,m=q.muzzle;
 Pt butt=pulse_ink_at(q,q.grip,-2.5f,.2f);butt.y+=2;
 model_sleeve(q.grip,butt,2,PULSE_SLIDE_INK);
 model_sleeve(q.grip,butt,1,0x43566f);

 pulse_barrel(q,back,m,2.45f,1.95f,PULSE_SLIDE_INK);
 Pt shoulder=pulse_ink_at(q,back,1.f,0),nose=pulse_ink_at(q,m,-1.f,0);
 pulse_barrel(q,shoulder,nose,1.35f,1.15f,PULSE_SLIDE_FACE);

 /* The lit edge and inset stay to one side of the centerline at every
    angle. Muzzle ink stays at the precise firing anchor, on the cap. */
 Pt a=pulse_ink_at(q,shoulder,0,-1.25f),b=pulse_ink_at(q,m,-2.2f,-1.25f);
 line(a.x,a.y,b.x,b.y,PULSE_SLIDE_GLINT);
 if(sv.gear>0){Pt t=pulse_ink_at(q,m,-3.f,1.f);dot(t.x,t.y,TEAL);}
 dot(m.x,m.y,PULSE_MUZZLE_INK);
}
