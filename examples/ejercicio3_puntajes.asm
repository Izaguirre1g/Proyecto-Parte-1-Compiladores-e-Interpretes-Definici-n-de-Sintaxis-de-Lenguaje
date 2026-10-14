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
sumai x13,x0,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,25 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,32 | nop | nop | nop
nop
nop
nop | nop | syer x1,x13,0 | nop
nop
nop
# __arranque_fin:
nop | nop | sye x0,0 | nop  # -> __arranque_fin
nop
nop
# __fn_sumar:
restai x2,x2,12 | nop | nop | nop
nop
nop
nop | guardap x2,x1,0 | nop | nop
nop
nop
nop | guardap x2,x8,4 | nop | nop
nop
nop
nop | guardap x2,x9,8 | nop | nop
nop
nop
nop | cargai x4,x2,4 | nop | nop
nop
nop
nop | cargai x5,x2,8 | nop | nop
nop
nop
suma x4,x4,x5 | nop | nop | nop
nop
nop
sumai x8,x4,0 | nop | nop | nop
nop
nop
nop | cargai x1,x2,0 | nop | nop
nop
nop
sumai x2,x2,12 | nop | nop | nop
nop
nop
nop | nop | syer x0,x1,0 | nop
nop
nop
# __fn_puntaje:
restai x2,x2,8 | nop | nop | nop
nop
nop
nop | guardap x2,x1,0 | nop | nop
nop
nop
nop | guardap x2,x8,4 | nop | nop
nop
nop
nop | cargai x4,x2,4 | nop | nop
nop
nop
sumai x5,x0,3 | nop | nop | nop
nop
nop
mrq x4,x4,x5 | nop | nop | nop
nop
nop
nop | nop | igualsi x4,x0,384 | nop  # -> __gf_puntaje_else_0
nop
nop
nop | cargai x4,x2,4 | nop | nop
nop
nop
sumai x5,x0,1 | nop | nop | nop
nop
nop
suma x4,x4,x5 | nop | nop | nop
nop
nop
sumai x8,x4,0 | nop | nop | nop
nop
nop
nop | cargai x1,x2,0 | nop | nop
nop
nop
sumai x2,x2,8 | nop | nop | nop
nop
nop
nop | nop | syer x0,x1,0 | nop
nop
nop
# __gf_puntaje_else_0:
nop | cargai x4,x2,4 | nop | nop
nop
nop
sumai x5,x0,5 | nop | nop | nop
nop
nop
mrq x4,x4,x5 | nop | nop | nop
nop
nop
nop | nop | igualsi x4,x0,384 | nop  # -> __gf_puntaje_else_1
nop
nop
nop | cargai x4,x2,4 | nop | nop
nop
nop
sumai x5,x0,2 | nop | nop | nop
nop
nop
suma x4,x4,x5 | nop | nop | nop
nop
nop
sumai x8,x4,0 | nop | nop | nop
nop
nop
nop | cargai x1,x2,0 | nop | nop
nop
nop
sumai x2,x2,8 | nop | nop | nop
nop
nop
nop | nop | syer x0,x1,0 | nop
nop
nop
# __gf_puntaje_else_1:
nop | cargai x4,x2,4 | nop | nop
nop
nop
sumai x5,x0,3 | nop | nop | nop
nop
nop
suma x4,x4,x5 | nop | nop | nop
nop
nop
sumai x8,x4,0 | nop | nop | nop
nop
nop
nop | cargai x1,x2,0 | nop | nop
nop
nop
sumai x2,x2,8 | nop | nop | nop
nop
nop
nop | nop | syer x0,x1,0 | nop
nop
nop
# __fn_acumular:
restai x2,x2,20 | nop | nop | nop
nop
nop
nop | guardap x2,x1,0 | nop | nop
nop
nop
nop | guardap x2,x8,4 | nop | nop
nop
nop
sumai x4,x0,1 | nop | nop | nop
nop
nop
nop | guardap x2,x4,8 | nop | nop
nop
nop
sumai x4,x0,0 | nop | nop | nop
nop
nop
nop | guardap x2,x4,12 | nop | nop
nop
nop
# __gf_acumular_while_cond_0:
nop | cargai x4,x2,8 | nop | nop
nop
nop
nop | cargai x5,x2,4 | nop | nop
nop
nop
sumai x6,x0,1 | nop | nop | nop
nop
nop
suma x5,x5,x6 | nop | nop | nop
nop
nop
mrq x4,x4,x5 | nop | nop | nop
nop
nop
nop | nop | igualno x4,x0,96 | nop  # -> __rs_cerca_0
nop
nop
nop | nop | sye x0,2400 | nop  # -> __gf_acumular_while_fin_0
nop
nop
# __rs_cerca_0:
nop | cargai x4,x2,8 | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | cargai x4,x2,0 | nop | nop
nop
nop
sumai x13,x0,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,6 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,96 | nop | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | cargai x8,x2,0 | nop | nop
nop
nop
nop | nop | syer x1,x13,0 | nop
nop
nop
sumai x2,x2,4 | nop | nop | nop
nop
nop
sumai x2,x2,4 | nop | nop | nop
nop
nop
sumai x4,x8,0 | nop | nop | nop
nop
nop
nop | guardap x2,x4,16 | nop | nop
nop
nop
nop | cargai x4,x2,12 | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | cargai x4,x2,20 | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | cargai x4,x2,4 | nop | nop
nop
nop
nop | cargai x5,x2,0 | nop | nop
nop
nop
sumai x13,x0,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,4 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,80 | nop | nop | nop
nop
nop
restai x2,x2,8 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | guardap x2,x5,4 | nop | nop
nop
nop
nop | cargai x8,x2,0 | nop | nop
nop
nop
nop | cargai x9,x2,4 | nop | nop
nop
nop
nop | nop | syer x1,x13,0 | nop
nop
nop
sumai x2,x2,8 | nop | nop | nop
nop
nop
sumai x2,x2,8 | nop | nop | nop
nop
nop
sumai x4,x8,0 | nop | nop | nop
nop
nop
nop | guardap x2,x4,12 | nop | nop
nop
nop
nop | cargai x4,x2,8 | nop | nop
nop
nop
sumai x5,x0,1 | nop | nop | nop
nop
nop
suma x4,x4,x5 | nop | nop | nop
nop
nop
nop | guardap x2,x4,8 | nop | nop
nop
nop
nop | nop | sye x0,-2640 | nop  # -> __gf_acumular_while_cond_0
nop
nop
# __gf_acumular_while_fin_0:
nop | cargai x4,x2,12 | nop | nop
nop
nop
sumai x8,x4,0 | nop | nop | nop
nop
nop
nop | cargai x1,x2,0 | nop | nop
nop
nop
sumai x2,x2,20 | nop | nop | nop
nop
nop
nop | nop | syer x0,x1,0 | nop
nop
nop
# __fn_main:
restai x2,x2,20 | nop | nop | nop
nop
nop
nop | guardap x2,x1,0 | nop | nop
nop
nop
sumai x4,x0,5 | nop | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | cargai x4,x2,0 | nop | nop
nop
nop
sumai x13,x0,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,12 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,96 | nop | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | cargai x8,x2,0 | nop | nop
nop
nop
nop | nop | syer x1,x13,0 | nop
nop
nop
sumai x2,x2,4 | nop | nop | nop
nop
nop
sumai x2,x2,4 | nop | nop | nop
nop
nop
sumai x4,x8,0 | nop | nop | nop
nop
nop
nop | guardap x2,x4,4 | nop | nop
nop
nop
sumai x4,x0,0 | nop | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | cargai x4,x2,0 | nop | nop
nop
nop
sumai x13,x0,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,12 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,96 | nop | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | cargai x8,x2,0 | nop | nop
nop
nop
nop | nop | syer x1,x13,0 | nop
nop
nop
sumai x2,x2,4 | nop | nop | nop
nop
nop
sumai x2,x2,4 | nop | nop | nop
nop
nop
sumai x4,x8,0 | nop | nop | nop
nop
nop
nop | guardap x2,x4,8 | nop | nop
nop
nop
sumai x4,x0,3 | nop | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | cargai x4,x2,0 | nop | nop
nop
nop
sumai x13,x0,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,12 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,96 | nop | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | cargai x8,x2,0 | nop | nop
nop
nop
nop | nop | syer x1,x13,0 | nop
nop
nop
sumai x2,x2,4 | nop | nop | nop
nop
nop
sumai x2,x2,4 | nop | nop | nop
nop
nop
sumai x4,x8,0 | nop | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
sumai x4,x0,1 | nop | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
sumai x4,x0,2 | nop | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | cargai x4,x2,4 | nop | nop
nop
nop
nop | cargai x5,x2,0 | nop | nop
nop
nop
sumai x13,x0,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,4 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,80 | nop | nop | nop
nop
nop
restai x2,x2,8 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | guardap x2,x5,4 | nop | nop
nop
nop
nop | cargai x8,x2,0 | nop | nop
nop
nop
nop | cargai x9,x2,4 | nop | nop
nop
nop
nop | nop | syer x1,x13,0 | nop
nop
nop
sumai x2,x2,8 | nop | nop | nop
nop
nop
sumai x2,x2,8 | nop | nop | nop
nop
nop
sumai x4,x8,0 | nop | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | cargai x4,x2,4 | nop | nop
nop
nop
nop | cargai x5,x2,0 | nop | nop
nop
nop
sumai x13,x0,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,4 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,80 | nop | nop | nop
nop
nop
restai x2,x2,8 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | guardap x2,x5,4 | nop | nop
nop
nop
nop | cargai x8,x2,0 | nop | nop
nop
nop
nop | cargai x9,x2,4 | nop | nop
nop
nop
nop | nop | syer x1,x13,0 | nop
nop
nop
sumai x2,x2,8 | nop | nop | nop
nop
nop
sumai x2,x2,8 | nop | nop | nop
nop
nop
sumai x4,x8,0 | nop | nop | nop
nop
nop
nop | guardap x2,x4,12 | nop | nop
nop
nop
nop | cargai x4,x2,4 | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | cargai x4,x2,16 | nop | nop
nop
nop
restai x2,x2,4 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | cargai x4,x2,4 | nop | nop
nop
nop
nop | cargai x5,x2,0 | nop | nop
nop
nop
sumai x13,x0,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,0 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,4 | nop | nop | nop
nop
nop
cizqi x13,x13,8 | nop | nop | nop
nop
nop
ori x13,x13,80 | nop | nop | nop
nop
nop
restai x2,x2,8 | nop | nop | nop
nop
nop
nop | guardap x2,x4,0 | nop | nop
nop
nop
nop | guardap x2,x5,4 | nop | nop
nop
nop
nop | cargai x8,x2,0 | nop | nop
nop
nop
nop | cargai x9,x2,4 | nop | nop
nop
nop
nop | nop | syer x1,x13,0 | nop
nop
nop
sumai x2,x2,8 | nop | nop | nop
nop
nop
sumai x2,x2,8 | nop | nop | nop
nop
nop
sumai x4,x8,0 | nop | nop | nop
nop
nop
nop | guardap x2,x4,16 | nop | nop
nop
nop
nop | cargai x1,x2,0 | nop | nop
nop
nop
sumai x2,x2,20 | nop | nop | nop
nop
nop
nop | nop | syer x0,x1,0 | nop
nop
nop
