;; CHIP-8 xor test

LD V0, 0x01
LD V1, 0x01
XOR V1, V1 ; V1 = 0
XOR V1, V0 ; V1 = 1