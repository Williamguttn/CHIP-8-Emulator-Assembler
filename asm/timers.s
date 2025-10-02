; Make ball appear after 3 seconds

CLS

LD V0, 180 ; 3s
LD DT, V0 ; Set delay timer

wait_loop:
LD V1, DT ; V1 = current timer val
SE V1, 0 ; skip next if V1 == 0
JP wait_loop

show_ball:
CLS
LD I, ball_sprite
LD V0, 30
LD V1, 14
DRW V0, V1, 4

loop:
JP loop ; keep window open

ball_sprite:
DB 0xF0, 0xF0, 0xF0, 0xF0