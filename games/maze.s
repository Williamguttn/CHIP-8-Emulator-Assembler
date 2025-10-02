; CHIP-8 Program Disassembly

MAIN:
    LD I, SPRITE_DATA_1      ; 0x0200: A2 1E - Load I with sprite data at 0x21E
    RND V2, 0x01             ; 0x0202: C2 01 - Set V2 to random number AND 0x01
    SE V2, 0x01              ; 0x0204: 32 01 - Skip next if V2 == 0x01
    LD I, SPRITE_DATA_2      ; 0x0206: A2 1A - Load I with sprite data at 0x21A
    
DRAW_LOOP:
    DRW V0, V1, 0x04         ; 0x0208: D0 14 - Draw 4-byte sprite at (V0, V1)
    ADD V0, 0x04             ; 0x020A: 70 04 - Add 4 to V0
    SE V0, 0x40              ; 0x020C: 30 40 - Skip next if V0 == 0x40
    JP MAIN                  ; 0x020E: 12 00 - Jump to MAIN
    
    LD V0, 0x00              ; 0x0210: 60 00 - Set V0 to 0
    ADD V1, 0x04             ; 0x0212: 71 04 - Add 4 to V1
    SE V1, 0x20              ; 0x0214: 31 20 - Skip next if V1 == 0x20
    JP MAIN                  ; 0x0216: 12 00 - Jump to MAIN
    
INFINITE_LOOP:
    JP INFINITE_LOOP         ; 0x0218: 12 18 - Infinite loop


SPRITE_DATA_2:
    DB 0x80, 0x40            ; 0x021A: 80 40 - Sprite pattern 1
    DB 0x20, 0x10            ; 0x021C: 20 10 - Sprite pattern 2

SPRITE_DATA_1:
    DB 0x20, 0x40            ; 0x021E: 20 40 - Sprite pattern 3
    DB 0x80, 0x10            ; 0x0220: 80 10 - Sprite pattern 4
