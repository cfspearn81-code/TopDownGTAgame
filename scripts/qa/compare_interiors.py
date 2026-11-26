"""Usage: python compare_interiors.py BASELINE_FRAMES CANDIDATE_FRAMES
Exploratory QA measurements, not art-direction pass/fail thresholds. Compare
matching addresses; ignore the menus, headings and HUD. Never substitute these
metrics for inspecting representative native images and playing each room.
"""
import csv
import statistics
import sys
from pathlib import Path
from PIL import Image, ImageChops, ImageOps, ImageFilter

def edges(frame):
    with Image.open(frame) as im:
        # In-room floor and furnishings only; 2x decimation suppresses text speckle.
        a=im.crop((158,146,802,407)).resize((322,131),Image.Resampling.BILINEAR).convert('L')
    a=a.filter(ImageFilter.GaussianBlur(.8)).filter(ImageFilter.FIND_EDGES).crop((2,2,320,129))
    hist=a.histogram();n=a.width*a.height;acc=0
    for threshold in range(255,0,-1):
        acc+=hist[threshold]
        if acc>=n*.12:break
    return a.point(lambda v:int(v>=threshold)*255)

def count(im):return im.histogram()[255]
def match(a,b):
    # Soft structural edge disagreement: allow 2 native pixels of tolerance
    # for a matching contour. Contrast normalisation makes simple recolouring
    # less influential, but it cannot distinguish all paint from geometry.
    n=count(a)+count(b)
    av=count(ImageChops.multiply(a,b.filter(ImageFilter.MaxFilter(3))))
    bv=count(ImageChops.multiply(b,a.filter(ImageFilter.MaxFilter(3))))
    return 100*(1-(av+bv)/max(n,1))
def footprint_similarity(a,b):
    # Same-origin occupancy intersection over union of *walkable* pixels.
    common=count(ImageChops.multiply(a,b));union=count(ImageChops.lighter(a,b))
    return 100*common/max(union,1)
def frames(folder):
    rooms={}
    with (folder/'rooms.tsv').open() as f:
        for group,num,leaf,name,desc in csv.reader(f,delimiter='\t'):
            path=folder/leaf;stem=path.stem
            with Image.open(folder/(stem+'-walk.pgm')) as m:mask=m.copy()
            rooms[(group,int(num))]=(name,edges(path),mask)
    return rooms
if __name__=='__main__':
    a=frames(Path(sys.argv[1]));b=frames(Path(sys.argv[2]))
    if a.keys()!=b.keys():raise SystemExit("Cannot compare: inventories differ")
    rows=[]
    for k in sorted(a):
        old,new=a[k],b[k];rows.append((k[0],k[1],new[0],match(old[1],new[1]),100-footprint_similarity(old[2],new[2])))
    print('Room count, median and minimum changed structural-edge share; count with changed collision footprints')
    for g in ['venue','rear','address']:
        rr=[r for r in rows if r[0]==g]
        print(f'{g}: {len(rr)}   median {statistics.median(r[3] for r in rr):.1f}%   min {min(r[3] for r in rr):.1f}%   footprints changed: {sum(r[4]>0 for r in rr)}/{len(rr)}')
    print('Least-changed art (building IDs):', ', '.join(f'{r[2]} #{r[1]} ({r[3]:.1f}%)' for r in sorted(rows,key=lambda x:x[3])[:6]))
    print('Closest candidate street-address collision footprints (including mirror-equivalent plans):')
    shops=[(k,v) for k,v in b.items() if k[0]=='address'];pairs=[]
    for i,(ka,va) in enumerate(shops):
        for kb,vb in shops[:i]:
            sim=max(footprint_similarity(va[2],vb[2]),footprint_similarity(va[2],ImageOps.mirror(vb[2])))
            pairs.append((sim,ka[1],va[0],kb[1],vb[0]))
    for sim,i,na,j,nb in sorted(pairs,reverse=True)[:3]:print(f'  {sim:.1f}% {na} #{i} / {nb} #{j}')
