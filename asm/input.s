; Draw square when "q" is pressed
CLS
LD V3, 4 ; key index for "q" key

check_loop:
SKNP V3 ; skip if NOT pressed
JP draw_square
JP check_loop

draw_square:
CLS
LD I, ball_sprite
LD V0, 30
LD V1, 14
DRW V0, V1, 4

loop:
JP loop ; keep window open

ball_sprite:
DB 0xF0, 0xF0, 0xF0, 0xF0