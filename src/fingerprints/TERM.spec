%fingerprint-spec 1.2
%fprint:TERM
%url:http://www.elf-emulation.com/funge/rcfunge_manual.html
%desc:Terminal control functions
%safe:true
%begin-instrs
#I	name	desc
C	clearScreen	Clear screen
D	goDown	Move cursor down n lines
G	gotoXY	Goto cursor position x,y (home is 0,0)
H	goHome	Move cursor to home
L	clearToEOL	Clear from cursor to end of line
S	clearToEOS	Clear from cursor to end of screen
U	goUp	Move cursor up n lines
%end
