%fingerprint-spec 1.0
%fprint:FILE
%url:http://web.archive.org/web/20020816190021/http://homer.span.ch/~spaw1088/funge.html
%desc:File I/O functions
%safe:false
%begin-instrs
#I	name	desc
C	fclose	Close a file
G	fgets	Get string from file (like c fgets)
L	ftell	Get current location in file
O	fopen	Open a file (Va = i/o buffer vector)
P	fputs	Put string to file (like c fputs)
R	fread	Read n bytes from file to i/o buffer
S	fseek	Seek to position in file
W	fwrite	Write n bytes from i/o buffer to file
%end
