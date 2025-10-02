LD I, ball_sprite
LD V0, 30
LD V1, 14
DRW V0, V1, 4

loop:
CLS
ADD V0, 1
DRW V0, V1, 4
JP loop

ball_sprite:
DB 0xF0, 0xF0, 0xF0, 0xF0

tempend: