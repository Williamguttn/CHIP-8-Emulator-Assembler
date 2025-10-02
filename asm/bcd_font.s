; Because LD Vx, [I] is quirky and loads V0 through Vx, we will use the higher registers for general variables.

CLS
LD V7, 156
LD I, digits
LD B, V7 ; Store 1, 5, 6 into digits[0..2]

; Set up positions & loop counter
LD V5, 10 ; x
LD V6, 10 ; y
LD V4, 0 ; Digit index

draw_loop:
LD I, digits
ADD I, V4 ; digits[V4]. I is now address to the current digit
LD V0, [I] ; Curr digit in V0

; Now draw the sprite
LD I, V0 ; Font address
DRW V5, V6, 5

ADD V5, 6 ; space for next digit

; Deal with loop counter
ADD V4, 1
SE V4, 3 ; Done?
JP draw_loop

halt:
JP halt

digits:
DB 0, 0, 0