%fingerprint-spec 1.2
%fprint:SOCK
%url:http://rcfunge98.com/rcsfingers.html#SOCK
%desc:TCP/IP socket extension
%safe:false
%begin-instrs
#I	name	desc
A	accept	Accept a connection
B	bind	Bind a socket
C	open	Open a connection
I	fromascii	Convert an ASCII IP address to a 32 bit address
K	kill	Kill a connection
L	listen	Set a socket to listening mode (n=backlog size)
O	setopt	Set socket option
R	receive	Receive from a socket
S	create	Create a socket
W	write	Write to a socket
%end
