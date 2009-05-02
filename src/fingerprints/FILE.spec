%fingerprint-spec 1.2
%fprint:FILE
%url:http://rcfunge98.com/rcsfingers.html
%desc:File I/O functions
%safe:false
%begin-instrs
#I	name	desc
C	fclose	Close a file
D	delete	Delete specified file
G	fgets	Get string from file (like c fgets)
L	ftell	Get current location in file
O	fopen	Open a file (Va = i/o buffer vector)
P	fputs	Put string to file (like c fputs)
R	fread	Read n bytes from file to i/o buffer
S	fseek	Seek to position in file
W	fwrite	Write n bytes from i/o buffer to file
%end
