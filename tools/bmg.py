"""Readers for Yaz0, RARC and BMG (from vendor-price-mod/tools/price_messages.py)."""
import struct

def yaz0(d):
    assert d[:4]==b'Yaz0'
    size=struct.unpack('>I',d[4:8])[0]; out=bytearray(); i=16
    while len(out)<size:
        code=d[i]; i+=1
        for b in range(8):
            if len(out)>=size: break
            if code&(0x80>>b): out.append(d[i]); i+=1
            else:
                b1,b2=d[i],d[i+1]; i+=2
                dist=((b1&0xF)<<8|b2)+1; n=b1>>4
                if n==0: n=d[i]+0x12; i+=1
                else: n+=2
                for _ in range(n): out.append(out[-dist])
    return bytes(out)
def rarc(d):
    hdr=d[:0x40]
    fdo=struct.unpack('>I',d[0xC:0x10])[0]+0x20
    info=0x20
    nodes_n,nodes_off,ent_n,ent_off,str_sz,str_off=struct.unpack('>IIIIII',d[info:info+24])
    nodes_off+=0x20; ent_off+=0x20; str_off+=0x20
    files={}
    for e in range(ent_n):
        o=ent_off+e*20
        idx,h,typ,noff,doff,sz=struct.unpack('>HHHHII',d[o:o+16])
        name=d[str_off+noff:d.index(b'\0',str_off+noff)].decode()
        if (typ>>8)&1 and name not in('.','..'):
            files[name]=d[fdo+doff:fdo+doff+sz]
    return files
def blocks(d):
    n=struct.unpack('>I',d[0xC:0x10])[0]; o=0x20; out={}
    for _ in range(n):
        tag=d[o:o+4].decode(); sz=struct.unpack('>I',d[o+4:o+8])[0]; out[tag]=o; o+=sz
    return out
def messages(d):
    b = blocks(d); inf = b['INF1']; dat = b['DAT1'] + 8
    cnt, esz = struct.unpack('>HH', d[inf + 8:inf + 12])
    res = []
    for k in range(cnt):
        e = inf + 0x10 + k * esz
        off = struct.unpack('>I', d[e:e + 4])[0]; mid = struct.unpack('>H', d[e + 4:e + 6])[0]
        j = dat + off  # text ends at a 0 byte outside a tag
        while d[j] != 0:
            j += d[j + 1] if d[j] == 0x1A else 1
        res.append((mid, d[dat + off:j]))
    return res
