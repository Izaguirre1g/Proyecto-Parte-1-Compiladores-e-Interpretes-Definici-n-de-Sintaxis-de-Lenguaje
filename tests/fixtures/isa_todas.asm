# Cubre las 34 instrucciones, pseudoinstrucciones, alias ABI, etiquetas y
# operandos nombrados. isa_todas.mem e isa_todas.bin se generaron con
# tools/assembler.py del repositorio de Arquitectura (commit 818e1de).
inicio: suma x5, x6, x7 | nop | nop | nop
resta x8, x9, x10 | nop | nop | nop
cader x11, x12, x13 | nop | nop | nop
cizq x14, x15, x16 | nop | nop | nop
and x17, x18, x19 | nop | nop | nop
or x20, x21, x22 | nop | nop | nop
xor x23, x24, x25 | nop | nop | nop
cder x26, x27, x28 | nop | nop | nop
mrq x29, x30, x31 | nop | nop | nop
myq x4, x6, x7 | nop | nop | nop
sumai x5, x6, -1024 | nop | nop | nop
restai x8, x9, 1023 | nop | nop | nop
cizqi x10, x6, 2047 | nop | nop | nop
cderi x11, x6, 31 | nop | nop | nop
caderi x12, x6, 0b11 | nop | nop | nop
xori x13, x6, -1 | nop | nop | nop
andi x14, x6, 0xff | nop | nop | nop
ori x15, x6, +0x7f | nop | nop | nop
mov t0, s0 | nop | nop | nop
not s15, gp | nop | nop | nop
nop | guardap sp, x16, -4 | nop | nop
nop | guardab gp, t1, 1023 | nop | nop
nop | cargai x17, sp, 8 | nop | nop
nop | cargabai x18, gp, -1024 | nop | nop
nop | nop | igualsi x16, x19, inicio | nop
nop | nop | igualno x16, x19, fin | nop
nop | nop | menora x20, x21, -32 | nop
nop | nop | mayoroigual x22, x23, 16 | nop
nop | nop | sye x0, fin | nop
nop | nop | syer x1, x24, 0 | nop
nop | nop | ret | nop
nop | nop | nop | fsl x6, x2, 0, 1
nop | nop | nop | fsli x8, x4, 3, 2
nop | nop | nop | ell 2, 0, x4, x10
nop | nop | nop | vcr 0x2000
nop | nop | nop | camcon 16, 7
nop | nop | nop | setpwd x25
nop | nop | nop | fsl rd=12, r1=14, LK=1, RK=3
nop
nop
fin:
sumai x26, x0, 7 | cargai x27, x2, 0 | igualsi x28, x29, inicio | nop   ; comentario
nop // otro comentario
nop | nop | sye ra, inicio | nop # llamada
