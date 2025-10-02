LD V0, 0x05
LD V1, 0x03

SNE V0, V1

infloop:
JP infloop

skip:
LD V0, 0x00
LD V1, 0x01