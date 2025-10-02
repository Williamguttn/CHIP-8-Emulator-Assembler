LD V0, 1
LD V1, 2

JP jmp1

jmp2:
LD V4, 5
LD V5, 6
JP end

nowdefinebytes:
DB 0xDE, 0xAD, 0xBE, 0xEF
DB 0xCA, 0xFE, 0xBA, 0xBE


jmp1:
LD V2, 3
LD V3, 4
JP jmp2

end: