LD V0, 0

loop:
CALL incr
SNE V0, 10
JP end
JP loop

incr:
ADD V0, 1
RET

end:
LD V1, 1