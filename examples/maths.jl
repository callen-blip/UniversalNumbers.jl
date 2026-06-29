using UniversalNumbers

# Create two Posit{16,1} numbers (any Real works as input)
a = Takum{16}(pi)
b = Posit{16,1}(0.5)
# Maths 
sin(a)
cos(a)
tan(a)
tan(b)

sinh(a)
cosh(a)
tanh(a)

asin(b)
acos(b)
atan(b)

log2(a)
log10(a)
log1p(a)

exp(a)
exp2(a)
exp10(a)
expm1(a)
