; Test 8XY6 - SHR Vx (shift right)
; Load test value into V1
LD V1, 0x0B    ; V1 = 11 (binary: 00001011)
SHR V1         ; Shift right: V1 = 5 (binary: 00000101), VF = 1 (LSB was 1)

; Test with even number
LD V2, 0x08    ; V2 = 8 (binary: 00001000)
SHR V2         ; Shift right: V2 = 4 (binary: 00000100), VF = 0 (LSB was 0)

; Test 8XYE - SHL Vx (shift left)
; Load test value into V3
LD V3, 0x85    ; V3 = 133 (binary: 10000101)
SHL V3         ; Shift left: V3 = 10 (binary: 00001010, wraps), VF = 1 (MSB was 1)

; Test with MSB = 0
LD V4, 0x3F    ; V4 = 63 (binary: 00111111)
SHL V4         ; Shift left: V4 = 126 (binary: 01111110), VF = 0 (MSB was 0)

; Test 8XY7 - SUBN Vx, Vy (Vx = Vy - Vx)
; Test with no underflow (VY >= VX)
LD V5, 0x0A    ; V5 = 10
LD V6, 0x1E    ; V6 = 30
SUBN V5, V6    ; V5 = 30 - 10 = 20, VF = 1 (no underflow)

; Test with underflow (VY < VX)
LD V7, 0x32    ; V7 = 50
LD V8, 0x14    ; V8 = 20
SUBN V7, V8    ; V7 = 20 - 50 = -30 = 226 (wraps), VF = 0 (underflow)

; Test edge case: equal values
LD V9, 0x0F    ; V9 = 15
LD VA, 0x0F    ; VA = 15
SUBN V9, VA    ; V9 = 15 - 15 = 0, VF = 1 (no underflow, VY >= VX)

loop:
JP loop