# __arranque:
sumai x3,x0,0 | nop | nop | nop
nop
nop
cizqi x3,x3,8 | nop | nop | nop
nop
nop
ori x3,x3,0 | nop | nop | nop
nop
nop
cizqi x3,x3,8 | nop | nop | nop
nop
nop
ori x3,x3,64 | nop | nop | nop
nop
nop
cizqi x3,x3,8 | nop | nop | nop
nop
nop
ori x3,x3,0 | nop | nop | nop
nop
nop
sumai x2,x0,0 | nop | nop | nop
nop
nop
cizqi x2,x2,8 | nop | nop | nop
nop
nop
ori x2,x2,1 | nop | nop | nop
nop
nop
cizqi x2,x2,8 | nop | nop | nop
nop
nop
ori x2,x2,0 | nop | nop | nop
nop
nop
cizqi x2,x2,8 | nop | nop | nop
nop
nop
ori x2,x2,0 | nop | nop | nop
nop
nop
sumai x13,x0,816 | nop | nop | nop
nop
nop
nop | nop | syer x1,x13,0 | nop
nop
nop
# __arranque_fin:
nop | nop | sye x0,0 | nop  # -> __arranque_fin
nop
nop
# __fn_main:
restai x2,x2,16 | nop | nop | nop
nop
nop
nop | guardap x2,x1,0 | nop | nop
nop
nop
sumai x4,x0,5 | nop | nop | nop
nop
nop
nop | guardap x2,x4,4 | nop | nop
nop
nop
sumai x4,x0,1 | nop | nop | nop
nop
nop
nop | guardap x2,x4,8 | nop | nop
nop
nop
sumai x4,x0,2 | nop | nop | nop
nop
nop
nop | guardap x2,x4,12 | nop | nop
nop
nop
nop | cargai x4,x2,4 | nop | nop
nop
nop
sumai x5,x0,1 | nop | nop | nop
nop
nop
myq x4,x4,x5 | nop | nop | nop
nop
nop
xori x4,x4,1 | nop | nop | nop
nop
nop
nop | nop | igualsi x4,x0,192 | nop  # -> __gf_main_else_0
nop
nop
sumai x4,x0,1 | nop | nop | nop
nop
nop
nop | guardap x2,x4,8 | nop | nop
nop
nop
nop | nop | sye x0,1344 | nop  # -> __gf_main_fin_0
nop
nop
# __gf_main_else_0:
# __gf_main_while_cond_1:
nop | cargai x4,x2,12 | nop | nop
nop
nop
nop | cargai x5,x2,4 | nop | nop
nop
nop
myq x4,x4,x5 | nop | nop | nop
nop
nop
xori x4,x4,1 | nop | nop | nop
nop
nop
nop | nop | igualno x4,x0,96 | nop  # -> __rs_cerca_0
nop
nop
nop | nop | sye x0,1056 | nop  # -> __gf_main_while_fin_1
nop
nop
# __rs_cerca_0:
nop | cargai x4,x2,8 | nop | nop
nop
nop
nop | cargai x5,x2,12 | nop | nop
nop
nop
sumai x12,x4,0 | nop | nop | nop
nop
nop
sumai x13,x5,0 | nop | nop | nop
nop
nop
sumai x14,x0,0 | nop | nop | nop
nop
nop
sumai x5,x0,32 | nop | nop | nop
nop
nop
# __gb_mul_1_paso:
andi x15,x13,1 | nop | nop | nop
nop
nop
resta x15,x0,x15 | nop | nop | nop
nop
nop
and x15,x12,x15 | nop | nop | nop
nop
nop
suma x14,x14,x15 | nop | nop | nop
nop
nop
cizqi x12,x12,1 | nop | nop | nop
nop
nop
cderi x13,x13,1 | nop | nop | nop
nop
nop
restai x5,x5,1 | nop | nop | nop
nop
nop
nop | nop | igualno x5,x0,-336 | nop  # -> __gb_mul_1_paso
nop
nop
sumai x4,x14,0 | nop | nop | nop
nop
nop
nop | guardap x2,x4,8 | nop | nop
nop
nop
nop | cargai x4,x2,12 | nop | nop
nop
nop
sumai x5,x0,1 | nop | nop | nop
nop
nop
suma x4,x4,x5 | nop | nop | nop
nop
nop
nop | guardap x2,x4,12 | nop | nop
nop
nop
nop | nop | sye x0,-1248 | nop  # -> __gf_main_while_cond_1
nop
nop
# __gf_main_while_fin_1:
# __gf_main_fin_0:
nop | cargai x1,x2,0 | nop | nop
nop
nop
sumai x2,x2,16 | nop | nop | nop
nop
nop
nop | nop | syer x0,x1,0 | nop
nop
nop
