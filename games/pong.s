;;        CHIP-8 PONG           ;;
;; 50 opcodes/frame recommended ;;
; chip8 --asm pong.s -n 50 --run

start:
    JP loop

draw_score:
    LD I, score
    LD V0, [I]
    LD V7, V0
    LD I, digits
    LD B, V7 ; store BCD of V7 at I, I+1, I+2

    ; Positions & loop counter
    LD V4, 23 ; x
    LD V5, 3 ; y
    LD V6, 0 ; digit index

draw_score_loop:
    LD I, digits
    ADD I, V6 ; digits[V6]. I is now address to the current digit
    LD V0, [I] ; Curr digit in V0

    ; Now draw the sprite
    LD I, V0 ; Font address
    DRW V4, V5, 5

    ADD V4, 6 ; space for next digit

    ; Deal with loop counter
    ADD V6, 1
    SE V6, 3 ; Done?
    JP draw_score_loop

    RET

draw:
    CLS
    CALL load_plr_pos ; pos to a and b

    LD I, paddle_sprite
    DRW VA, VB, 9

    ; draw ball
    CALL load_ball_pos ; pos to c and d

    LD I, ball_sprite
    DRW VC, VD, 3

    JP draw_score ; handles RET

plr_move_up:
    LD I, plr_y
    LD V0, [I]

    LD V1, 0
    SNE V0, V1
    JP skip_up ; clamp at 0

    LD V1, 1
    SUB V0, V1 ; SUB V0, 1 not allowed
    LD [I], V0
skip_up:
    RET

plr_move_down:
    LD I, plr_y
    LD V0, [I]

    LD V1, 23
    SNE V0, V1
    JP skip_down ; clamp at 23 (bottom of screen)

    ADD V0, 1
    LD [I], V0
skip_down:
    RET

check_paddle_input:
    LD V9, 1 ; KEY_ONE (up)
    SKNP V9
    JP plr_move_up ; Will return back to loop in plr_move_up

    LD V9, 4 ; KEY_Q (down)
    SKNP V9
    JP plr_move_down

    RET

check_ball_collision:
    ; Check X
    LD V6, 1 ; xor mask
    LD V5, 0 ; Collision flag

    ; x == 0
    LD V7, 0
    SE VC, V7
    JP no_left_x
    LD VC, 0
    CALL save_ball_pos
    JP x_flip
no_left_x:

    ; x == 61
    LD V7, 61
    SE VC, V7
    JP check_ball_collision_y
    LD VC, 61
    CALL save_ball_pos
    JP x_flip

    JP check_ball_collision_y
x_flip:
    ; Flip sign
    XOR V8, V6
    LD V5, 1 ; Collision flag
    JP check_ball_collision_y
check_ball_collision_y:
    ; y == 0
    LD V7, 0
    SE VD, V7
    JP no_top_y
    LD VD, 0
    CALL save_ball_pos
    JP y_flip
no_top_y:

    ; y == 29
    LD V7, 29
    SE VD, V7
    JP save_signs
    LD VD, 29
    CALL save_ball_pos
    JP y_flip

    JP save_signs
y_flip:
    XOR V9, V6
    LD V5, 1 ; Collision flag
save_signs:
    LD V0, V8
    LD V1, V9
    LD I, ball_sign_x
    LD [I], V1 ; [I] = ball_sign_x, [I+1] = ball_sign_y
    RET

v0_leq_v1:
    LD V2, V1
    SUB V2, V0
    LD V2, VF
    RET

v0_geq_v1:
    LD V2, V0
    SUB V2, V1
    LD V2, VF
    RET

check_ball_paddle_collision:
    ; collision true if (ball_x <= plr_x + width) && (ball_y >= plr_y) && (ball_y <= plr_y + height)
    CALL load_plr_pos ; VA = plr_x, VB = plr_y

    LD V3, VA ; V3 = original plr_x
    LD V4, VB ; V4 = original plr_y

    ; VA = plr_x + width
    ; VB = plr_y + height
    ADD VA, 1 ; width = 2px
    ADD VB, 8 ; height = 9px

; 1. (ball_x <= plr_x + width) (VC <= VA)
    LD V0, VC
    LD V1, VA
    CALL v0_leq_v1
    ; V2 = 1 => true
    SE V2, V6
    RET         ; No collision

; 2. (ball_y >= plr_y) (VD >= V4)
    LD V0, VD
    ADD V0, 2
    LD V1, V4
    CALL v0_geq_v1
    ; V2 = 1 => true
    SE V2, V6
    RET         ; No collision

; 3. (ball_y <= plr_y + height) (VD <= VB)
    LD V0, VD
    LD V1, VB
    CALL v0_leq_v1
    ; V2 = 1 => true
    SE V2, V6
    RET         ; No collision

    LD V0, VC
    ADD V0, 2
    LD V1, V3
    CALL v0_geq_v1
    ; V2 = 1 => true
    SE V2, V6
    RET

    ; Collision
    LD V5, 1
    LD V0, VA
    ADD V0, 1
    LD VC, V0
    CALL save_ball_pos
    XOR V8, V6 ; Flip ball x-sign
    CALL increase_score
    JP save_signs

ball_move:
    CALL load_ball_pos ; X = VC, Y = VD, X_sign = V8, Y_sign = V9
    LD V6, 1
    LD V5, 0
    CALL check_ball_paddle_collision
    SE V5, V6
    CALL check_ball_collision

    ;LD V6, 1 ; DX/DY USING XOR MASK

    ; Check x-sign
    SNE V8, 0
    JP ball_move_x_pos
    JP ball_move_x_neg

ball_move_x_pos:
    ADD VC, V6
    JP ball_move_y
ball_move_x_neg:
    SUB VC, V6
ball_move_y:

    ; Check y-sign
    SNE V9, 0
    JP ball_move_y_pos
    JP ball_move_y_neg

ball_move_y_pos:
    ADD VD, V6
    JP ball_move_done
ball_move_y_neg:
    SUB VD, V6
ball_move_done:
; Save ball positions (signs are done in collision check)
    LD V0, VC
    LD V1, VD ; X = VC, Y = VD
    LD I, ball_x
    LD [I], V1 ; [I] = ball_x, [I+1] = ball_y
    RET

loop:
    CALL check_paddle_input
    CALL ball_move
    CALL draw

    JP loop

; Player positions to VA (x) and VB (y)
load_plr_pos:
    LD I, plr_x
    LD V1, [I] ; V0 = plr_h, V1 = plr_y
    LD VA, V0
    LD VB, V1
    RET

; Ball positions to VC (x) and VD (y). X sign to V8, Y sign to V9
load_ball_pos:
    LD I, ball_x
    LD V3, [I] ; Load V[0..3] = ball_x, ball_y, ball_sign_x, ball_sign_y
    LD VC, V0
    LD VD, V1
    LD V8, V2
    LD V9, V3
    RET

save_ball_pos:
    LD V0, VC
    LD V1, VD
    LD I, ball_x
    LD [I], V1
    RET

increase_score:
    LD I, score
    LD V0, [I]
    ADD V0, 1
    LD [I], V0
    RET

reset_score:
    LD I, score
    LD V0, 0
    LD [I], V0
    RET

plr_x:
DB 0

plr_y:
DB 10

paddle_sprite:
DB 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0



ball_x:
DB 14

ball_y:
DB 13

ball_sign_x:
DB 0 ; 0 = positive, 1 = negative

ball_sign_y:
DB 0 ; 0 = positive, 1 = negative

ball_sprite:
DB 0xE0, 0xE0, 0xE0

score:
DB 0

digits:
DB 0, 0, 0 ; For drawing score