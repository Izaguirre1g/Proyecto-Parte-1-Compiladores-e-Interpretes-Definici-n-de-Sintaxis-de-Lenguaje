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
restai x2,x2,48 | nop | nop | nop
nop
nop
nop | guardap x2,x1,0 | nop | nop
nop
nop
sumai x4,x0,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,4 | nop | nop
nop
nop
sumai x4,x0,7 | nop | nop | nop
nop
nop
nop | guardap x2,x4,8 | nop | nop
nop
nop
sumai x4,x0,1 | nop | nop | nop
nop
nop
nop | guardap x2,x4,12 | nop | nop
nop
nop
sumai x4,x0,9 | nop | nop | nop
nop
nop
nop | guardap x2,x4,16 | nop | nop
nop
nop
sumai x4,x0,5 | nop | nop | nop
nop
nop
nop | guardap x2,x4,20 | nop | nop
nop
nop
sumai x4,x0,7 | nop | nop | nop
nop
nop
nop | guardap x2,x4,24 | nop | nop
nop
nop
sumai x4,x0,7 | nop | nop | nop
nop
nop
nop | guardap x2,x4,28 | nop | nop
nop
nop
sumai x4,x0,0 | nop | nop | nop
nop
nop
nop | guardap x2,x4,32 | nop | nop
nop
nop
sumai x4,x0,0 | nop | nop | nop
nop
nop
nop | guardap x2,x4,36 | nop | nop
nop
nop
sumai x4,x0,1 | nop | nop | nop
nop
nop
resta x4,x0,x4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,40 | nop | nop
nop
nop
sumai x4,x0,1 | nop | nop | nop
nop
nop
resta x4,x0,x4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,44 | nop | nop
nop
nop
# __gf_main_while_cond_0:
nop | cargai x4,x2,32 | nop | nop
nop
nop
sumai x5,x0,2 | nop | nop | nop
nop
nop
mrq x4,x4,x5 | nop | nop | nop
nop
nop
nop | nop | igualno x4,x0,96 | nop  # -> __rs_cerca_0
nop
nop
nop | nop | sye x0,2256 | nop  # -> __gf_main_while_fin_0
nop
nop
# __rs_cerca_0:
sumai x4,x0,0 | nop | nop | nop
nop
nop
nop | guardap x2,x4,36 | nop | nop
nop
nop
# __gf_main_while_cond_1:
nop | cargai x4,x2,36 | nop | nop
nop
nop
sumai x5,x0,3 | nop | nop | nop
nop
nop
mrq x4,x4,x5 | nop | nop | nop
nop
nop
nop | nop | igualno x4,x0,96 | nop  # -> __rs_cerca_1
nop
nop
nop | nop | sye x0,1680 | nop  # -> __gf_main_while_fin_1
nop
nop
# __rs_cerca_1:
nop | cargai x4,x2,32 | nop | nop
nop
nop
sumai x5,x4,0 | nop | nop | nop
nop
nop
cizqi x4,x4,1 | nop | nop | nop
nop
nop
suma x4,x4,x5 | nop | nop | nop
nop
nop
nop | cargai x5,x2,36 | nop | nop
nop
nop
suma x4,x4,x5 | nop | nop | nop
nop
nop
cizqi x4,x4,2 | nop | nop | nop
nop
nop
suma x4,x4,x2 | nop | nop | nop
nop
nop
nop | cargai x4,x4,4 | nop | nop
nop
nop
nop | cargai x5,x2,28 | nop | nop
nop
nop
xor x4,x4,x5 | nop | nop | nop
nop
nop
mrq x5,x4,x0 | nop | nop | nop
nop
nop
myq x4,x4,x0 | nop | nop | nop
nop
nop
or x4,x4,x5 | nop | nop | nop
nop
nop
xori x4,x4,1 | nop | nop | nop
nop
nop
nop | cargai x5,x2,40 | nop | nop
nop
nop
sumai x6,x0,1 | nop | nop | nop
nop
nop
resta x6,x0,x6 | nop | nop | nop
nop
nop
xor x5,x5,x6 | nop | nop | nop
nop
nop
mrq x6,x5,x0 | nop | nop | nop
nop
nop
myq x5,x5,x0 | nop | nop | nop
nop
nop
or x5,x5,x6 | nop | nop | nop
nop
nop
xori x5,x5,1 | nop | nop | nop
nop
nop
and x4,x4,x5 | nop | nop | nop
nop
nop
nop | nop | igualsi x4,x0,240 | nop  # -> __gf_main_else_2
nop
nop
nop | cargai x4,x2,32 | nop | nop
nop
nop
nop | guardap x2,x4,40 | nop | nop
nop
nop
nop | cargai x4,x2,36 | nop | nop
nop
nop
nop | guardap x2,x4,44 | nop | nop
nop
nop
# __gf_main_else_2:
nop | cargai x4,x2,36 | nop | nop
nop
nop
sumai x5,x0,1 | nop | nop | nop
nop
nop
suma x4,x4,x5 | nop | nop | nop
nop
nop
nop | guardap x2,x4,36 | nop | nop
nop
nop
nop | nop | sye x0,-1824 | nop  # -> __gf_main_while_cond_1
nop
nop
# __gf_main_while_fin_1:
nop | cargai x4,x2,32 | nop | nop
nop
nop
sumai x5,x0,1 | nop | nop | nop
nop
nop
suma x4,x4,x5 | nop | nop | nop
nop
nop
nop | guardap x2,x4,32 | nop | nop
nop
nop
nop | nop | sye x0,-2400 | nop  # -> __gf_main_while_cond_0
nop
nop
# __gf_main_while_fin_0:
nop | cargai x1,x2,0 | nop | nop
nop
nop
sumai x2,x2,48 | nop | nop | nop
nop
nop
nop | nop | syer x0,x1,0 | nop
nop
nop
