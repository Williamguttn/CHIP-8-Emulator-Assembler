LD V0, 0x10
LD V1, 0x10

SE V0, V1

infloop:
JP infloop

skip:
LD V0, 0x00
LD V1, 0x01