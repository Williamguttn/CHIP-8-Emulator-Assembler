start:
LD I, var ; I = address of var
LD V0, [I] ; Load var into V0
ADD V0, 5 ; V0 = V0 + 5
LD [I], V0 ; Store V0 back into memory at address I

LD V1, V0
JP end

var:
DB 10

end: