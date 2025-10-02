;;; Written by Claude ;;;

; CHIP-8 Font Display Program
; Draws several characters from the built-in fontset to the screen

; Starting at memory location 0x200 (default CHIP-8 program start)

START:
    ; Clear the screen
    CLS
    
    ; Draw the digit "0" at position (5, 5)
    LD V0, 0x0     ; Character 0
    LD V1, 0x5     ; X position
    LD V2, 0x5     ; Y position
    LD I, V0       ; Load font address for character in V0
    DRW V1, V2, 0x5 ; Draw 5-byte sprite
    
    ; Draw the digit "1" at position (12, 5)
    LD V0, 0x1     ; Character 1
    LD V1, 0xC     ; X position
    LD V2, 0x5     ; Y position
    LD I, V0       ; Load font address
    DRW V1, V2, 0x5 ; Draw sprite
    
    ; Draw the digit "2" at position (19, 5)
    LD V0, 0x2     ; Character 2
    LD V1, 0x13    ; X position
    LD V2, 0x5     ; Y position
    LD I, V0       ; Load font address
    DRW V1, V2, 0x5 ; Draw sprite
    
    ; Draw the digit "3" at position (26, 5)
    LD V0, 0x3     ; Character 3
    LD V1, 0x1A    ; X position
    LD V2, 0x5     ; Y position
    LD I, V0       ; Load font address
    DRW V1, V2, 0x5 ; Draw sprite
    
    ; Draw the letter "A" at position (5, 15)
    LD V0, 0xA     ; Character A (hex A)
    LD V1, 0x5     ; X position
    LD V2, 0xF     ; Y position
    LD I, V0       ; Load font address
    DRW V1, V2, 0x5 ; Draw sprite
    
    ; Draw the letter "B" at position (12, 15)
    LD V0, 0xB     ; Character B (hex B)
    LD V1, 0xC     ; X position
    LD V2, 0xF     ; Y position
    LD I, V0       ; Load font address
    DRW V1, V2, 0x5 ; Draw sprite
    
    ; Draw the letter "C" at position (19, 15)
    LD V0, 0xC     ; Character C (hex C)
    LD V1, 0x13    ; X position
    LD V2, 0xF     ; Y position
    LD I, V0       ; Load font address
    DRW V1, V2, 0x5 ; Draw sprite
    
    ; Draw the letter "F" at position (26, 15)
    LD V0, 0xF     ; Character F (hex F)
    LD V1, 0x1A    ; X position
    LD V2, 0xF     ; Y position
    LD I, V0       ; Load font address
    DRW V1, V2, 0x5 ; Draw sprite

LOOP:
    ; Infinite loop to keep the display showing
    JP LOOP