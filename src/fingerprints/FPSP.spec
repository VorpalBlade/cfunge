%fingerprint-spec 1.4
%fprint:FPSP
%url:http://rcfunge98.com/rcsfingers.html#FPSP
%desc:Single precision floating point
%condition:!defined(CFUN_NO_FLOATS)
%safe:true
%begin-instrs
#I	name	desc
A	add	add
B	sin	sin
C	cos	cos
D	div	divide
E	asin	arcsin
F	fromint	create float from integer
G	atan	arctan
H	acos	arcsin
I	toint	create integer from float
K	ln	natural logarithm
L	log10	10 logarithm
M	mul	multiply
N	neg	negate
P	print	output number
Q	sqrt	square root
R	fromascii	create float from ascii
S	sub	substract
T	tan	tan
V	abs	absolute value
X	exp	exp
Y	pow	pow
%end