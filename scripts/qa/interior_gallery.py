#!/usr/bin/env python3
"""Make proof images from the game's own rendered frames, not concept art.

  python3 scripts/qa/interior_gallery.py build/room-frames build/room-gallery
Requires Pillow. Produces an eight-room detail sheet, full 82-frame paginated
contact sheets and a PDF of all pages. The frame manifest is written by
src/interior_routes_test.c. A capture directory for a partial run is rejected.
"""
import argparse
import csv
from dataclasses import dataclass
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

BG="#101821"
CARD="#18242e"
INK="#e8eae0"
MUTED="#a4b6b2"
GOLD="#f2c976"
TEAL="#8ae5c5"
CATEGORIES={"venue":"CITY VENUE","rear":"STAFF FLOOR","address":"STREET ADDRESS"}
ACCENTS={"venue":"#8de0c7","rear":"#f6c47d","address":"#adc7f2"}

def font(size, bold=False, mono=False):
    names=(["DejaVuSansMono-Bold.ttf","LiberationMono-Bold.ttf"] if bold else ["DejaVuSansMono.ttf","LiberationMono-Regular.ttf"]) if mono else (["DejaVuSans-Bold.ttf","LiberationSans-Bold.ttf"] if bold else ["DejaVuSans.ttf","LiberationSans-Regular.ttf"])
    for name in names:
        try:
            return ImageFont.truetype(name,size)
        except OSError:
            pass
    try:
        return ImageFont.load_default(size=size)
    except TypeError:
        return ImageFont.load_default()

@dataclass
class Room:
    kind:str
    num:int
    path:Path
    name:str
    detail:str

def header(draw, width, title, strap, page):
    draw.line((44,32,width-44,32),fill=TEAL,width=3)
    draw.text((44,47),"DEAD CHANNEL     /     PORT VANTA",font=font(19,True,True),fill=TEAL)
    draw.text((width-44,48),page,font=font(19,False,True),fill=MUTED,anchor="ra")
    draw.text((41,78),title,font=font(56,True),fill=INK)
    draw.text((45,155),strap,font=font(21),fill=MUTED)

def textfit(d,text,w,size,bold=False):
    for sz in range(size,13,-1):
        f=font(sz,bold)
        if d.textlength(text,font=f)<=w:return f
    return font(13,bold)

def tile(canvas,room,x,y,width,height,section=False,num=0):
    """Details are cropped only in the selected sheet; atlas shows uncropped playframes."""
    d=ImageDraw.Draw(canvas)
    with Image.open(room.path) as opened:
        raw=opened.convert('RGB')
    assert raw.size==(960,540),f'Expected 960x540 native frame, found {raw.size} in {room.path}'
    if section: raw=raw.crop((21,66,939,470))
    shot=raw.resize((width,height),Image.Resampling.LANCZOS)
    d.rectangle((x,y,x+width-1,y+height+(85 if section else 72)),fill=CARD)
    canvas.paste(shot,(x,y))
    d.rectangle((x,y+height,x+3,y+height+(85 if section else 72)),fill=ACCENTS[room.kind])
    ident=f"{CATEGORIES[room.kind]}    /    {num:02d}"
    if room.kind=="rear": name=(room.detail if room.name in room.detail else f"{room.name}  /  {room.detail.split('/')[-1].strip()}")
    else: name=room.name
    d.text((x+15,y+height+8),ident,font=font(13,True,True),fill=ACCENTS[room.kind])
    d.text((x+14,y+height+29),name,font=textfit(d,name,width-28,26 if section else 21,True),fill=INK)
    if room.kind!="rear":
        d.text((x+16,y+height+(60 if section else 54)),room.detail,
               font=textfit(d,room.detail,width-30,14 if section else 13),fill=MUTED)

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('frames',type=Path);ap.add_argument('out',type=Path);a=ap.parse_args()
    with (a.frames/'rooms.tsv').open() as f:
        rooms=[Room(k,int(n),a.frames/p,name,detail) for k,n,p,name,detail in csv.reader(f,delimiter='\t')]
    counts={k:sum(r.kind==k for r in rooms) for k in CATEGORIES}
    if counts!={'venue':15,'rear':4,'address':63} or len({(r.kind,r.num) for r in rooms})!=82:
        raise SystemExit(f"Incomplete room set: {counts}. Run interior_routes_test again.")
    for r in rooms:
        if not r.path.is_file():raise SystemExit("Missing: "+str(r.path))
    a.out.mkdir(parents=True,exist_ok=True)
    venue=[r for r in rooms if r.kind=='venue']
    rear=[r for r in rooms if r.kind=='rear']
    address=sorted((r for r in rooms if r.kind=='address'),key=lambda r:(r.name,r.num))
    # Select by venue identity, not on a particular role/iteration layout.
    chosen=[(venue,('MOTORS',)),(address,('CINEMA','THEATER','THEATRE')),
            (venue,('CLINIC',)),(address,('AQUARIUM',)),
            (address,('TRAM',)),(venue,('APARTMENT',)),
            (venue,('POLICE',)),(rear,('GROW',))]
    selected=[next(r for r in group if any(t in r.name.upper() for t in terms))
              for group,terms in chosen]
    # The flagship view uses a playfield crop across the complete room silhouette to bring decor forward.
    c=Image.new('RGB',(1720,2087),BG);d=ImageDraw.Draw(c)
    header(d,c.width,"INSIDE THE CITY","Eight interiors, shown at playfield scale. All images come from the native game renderer.","FIELD GUIDE  /  01")
    for i,r in enumerate(selected):
        row,col=divmod(i,2);tile(c,r,44+col*828,206+row*452,804,354,True,i+1)
    d.line((44,2020,c.width-44,2020),fill='#3c5f60',width=2)
    d.text((44,2042),"CAPTURE NOTES     Entrance views after the cutaway roof opens. HUD cropped for this plate.",font=font(17,False,True),fill=MUTED)
    c.save(a.out/'Dead-Channel-Interior-Preview.png',optimize=True)
    ordered=venue+rear+address;pages=[];totalpages=(len(ordered)+8)//9
    for start in range(0,len(ordered),9):
        page=start//9+1;c=Image.new('RGB',(1592,1444),BG);d=ImageDraw.Draw(c)
        header(d,c.width,"THE INTERIOR ATLAS","15 city venues  /  4 staff floors  /  63 street addresses","CONTACT SHEET  /  %02d OF %02d"%(page,totalpages))
        for j,r in enumerate(ordered[start:start+9]):
            row,col=divmod(j,3);tile(c,r,44+col*508,209+row*390,488,275,False,start+j+1)
        d.line((44,1381,c.width-44,1381),fill='#3c5f60',width=2)
        d.text((44,1400),"NATIVE 960 x 540 RENDER   /   FULL FRAME, INCLUDING IN-GAME HUD",font=font(16,False,True),fill=MUTED)
        c.save(a.out/('atlas-%02d.png'%page),optimize=True);pages.append(c)
    pages[0].save(a.out/'Dead-Channel-Interior-Atlas.pdf',save_all=True,append_images=pages[1:],quality=88,resolution=144)
    print(f"Exported selected-room preview, {len(pages)} contact sheets and the 82-frame PDF to {a.out}")

if __name__=='__main__':main()
