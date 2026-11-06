import sys,struct,glob,os,shutil
sys.path.insert(0,'.')
from pm import yaz0,rarc,messages,blocks
def arc(p):
    d=open(p,'rb').read()
    return rarc(yaz0(d) if d[:4]==b'Yaz0' else d)
def patch(bmg,new):  # new: {mid: raw bytes}
    d=bytearray(bmg); b=blocks(d); inf=b['INF1']; dat=b['DAT1']
    assert list(b)[-1]=='DAT1' or True
    cnt,esz=struct.unpack('>HH',d[inf+8:inf+12])
    datsz=struct.unpack('>I',d[dat+4:dat+8])[0]
    # rebuild: put DAT1 last? check order
    body=bytearray(d[dat+8:dat+datsz])
    for k in range(cnt):
        e=inf+0x10+k*esz
        mid=struct.unpack('>H',d[e+4:e+6])[0]
        if mid in new:
            off=len(body); body+=new[mid]+b'\0'
            struct.pack_into('>I',d,e,off)
    while (8+len(body))%32: body.append(0)
    newdat=b'DAT1'+struct.pack('>I',8+len(body))+body
    out=d[:dat]+newdat+d[dat+datsz:]
    delta=len(newdat)-datsz
    struct.pack_into('>I',out,8,struct.unpack('>I',d[8:12])[0]+delta)
    return bytes(out)
H='/mnt/project-files/uploads/hearth/'
L='/mnt/user-data/uploads/mods/.claude_tmp/overlay/res/Msgus/'
VAN={2:glob.glob(H+'b4b189ac*')[0],4:glob.glob(H+'2830b646*')[0]}
for edition in ('linkle','vanilla'):
    root=f'out/{edition}'; shutil.rmtree(root,ignore_errors=True)
    for n in (2,4):
        name=f'zel_0{n}.bmg'
        v=dict(messages(arc(VAN[n])[name])); t=dict(messages(arc(f'tf/overlay/res/Msgus/bmgres{n}.arc')[name]))
        fixes={k:t[k] for k in v if v[k]!=t[k]}
        base=open(L+f'bmgres{n}/bmgres{n}/{name}','rb').read() if edition=='linkle' else arc(VAN[n])[name]
        out=patch(base,fixes)
        # verify
        om=dict(messages(out)); bm=dict(messages(base))
        assert all(om[k]==(fixes[k] if k in fixes else bm[k]) for k in bm) and len(om)==len(bm)
        p=f'{root}/overlay/res/Msgus/bmgres{n}/bmgres{n}'; os.makedirs(p)
        open(f'{p}/{name}','wb').write(out)
        print(edition,n,'fixed',sorted(fixes),'size',len(base),'->',len(out))
