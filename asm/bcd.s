
LD I, digits
LD V0, 156
LD B, V0 ; FX33: store BCD of V0 at I, I+1, I+2
JP end

digits:
DB 0, 0, 0

end: