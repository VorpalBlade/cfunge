%fingerprint-spec 1.4
%fprint:BASE
%url:http://rcfunge98.com/rcsfingers.html#BASE
%desc:I/O for numbers in other bases
%condition:!defined(CFUN_NO_FLOATS)
%safe:true
%begin-instrs
#I	name	desc
B	output_binary	Output in binary
H	output_hex	Output in hex
I	input_base	Input in base
N	output_base	Output in base
O	output_octal	Output in octal
%end
