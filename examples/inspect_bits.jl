using UniversalNumbers

# inspect the bit patterns of some numbers
printbits(Posit{16,2}(1.0))
printbits(Posit{16,2}(0.5))
printbits(Takum{32}(3.141592))
printbits(LNS{16, 5}(3.141592))
printbits(CFloat{24, 5}(1.0 ))


# more info on the bit patterns:
about(Posit{16,2}(1.0))
about(Posit{16,2}(0.5))
about(Takum{32}(3.141592))
about(LNS{16, 5}(3.141592))
about(CFloat{24, 5}(1.0 ))
