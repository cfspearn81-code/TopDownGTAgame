from PIL import Image,ImageDraw,ImageFont
from pathlib import Path
faces=['/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf','/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf']
out=['/* Rasterized glyphs. See font license in the source package. */']
for idx,(face,size,w,h) in enumerate(zip(faces,(15,33),(10,35),(20,40))):
 f=ImageFont.truetype(face,size); im=Image.new('L',(w*95,h)); d=ImageDraw.Draw(im); widths=[]
 for i in range(95):
  ch=chr(i+32);widths.append(max(5,round(f.getlength(ch))+1));d.text((i*w,2),ch,font=f,fill=255)
 out.append(f'static const unsigned char fw{idx}[95]={{'+','.join(map(str,widths))+'};')
 out.append(f'static const unsigned char font{idx}[{w*h*95}]={{')
 for i in range(95):
  for y in range(h):
   out.append(','.join(str(im.getpixel((i*w+x,y))//32) for x in range(w))+',')
 out.append('};')
Path('src/font.inc').write_text('\n'.join(out))
