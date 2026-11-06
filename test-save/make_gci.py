#!/usr/bin/env python3
"""Builds a Twilight Princess (GZ2E) memory card save (.gci) from a 0xA8C-byte quest log.

Usage: python3 -I make_gci.py <quest_log.bin> <out.gci> [comment]
Fills all 3 file slots (named Link, Linkle, Link) with the given quest log, using the layout in
Dusklight's m_Do_MemCardRWmng.cpp: 0x4000 header (no banner art), data at 0x4000, backup at 0x6000.
"""
import struct, sys, time

SAVEDATA_SIZE = 0xA94
src = open(sys.argv[1], 'rb').read()
assert len(src) == SAVEDATA_SIZE - 8, hex(len(src))

def slot(name):
    d = bytearray(src)
    for off, text in ((0x1B4, name), (0x1C5, 'Epona')):
        d[off:off + 17] = text.encode().ljust(17, b'\0')
    hi = sum(d) & 0xFFFFFFFF
    lo = sum(~b for b in d) & 0xFFFFFFFF
    return bytes(d) + struct.pack('>II', hi, lo)

saves = b''.join(slot(n) for n in ('Link', 'Linkle', 'Link'))
block = bytearray(0x2000)
struct.pack_into('>ii', block, 0, 0, 6)  # unk_0x0, SAVEDATA_VERSION
block[8:8 + len(saves)] = saves
words = struct.unpack('>%dH' % ((0x2000 - 4) // 2), block[:0x2000 - 4])
high = sum(words) & 0xFFFF
low = sum((~w) & 0xFFFF for w in words) & 0xFFFF
struct.pack_into('>I', block, 0x2000 - 4, high << 16 | low)

header = bytearray(0x4000)
header[0x2400:0x2420] = b'Zelda: Twilight Princess'.ljust(0x20, b'\0')
header[0x2420:0x2440] = (sys.argv[3] if len(sys.argv) > 3 else 'Test save').encode()[:31].ljust(0x20, b'\0')
data = bytes(header) + bytes(block) + bytes(block)
assert len(data) == 0x8000

mtime = int(time.time()) - 946684800  # seconds since 2000-01-01
gci = bytearray(0x40)
gci[0:4] = b'GZ2E'; gci[4:6] = b'01'; gci[6] = 0xFF
gci[7] = 1 | 4                      # banner C8, anim bounce
gci[8:40] = b'gczelda2'.ljust(32, b'\0')
struct.pack_into('>IIHHBBHHHI', gci, 0x28, mtime, 0, 0x155, 0x155, 0x04, 0, 0, len(data) // 0x2000, 0xFFFF, 0x2400)
open(sys.argv[2], 'wb').write(bytes(gci) + data)
