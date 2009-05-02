%fingerprint-spec 1.3
%fprint:TERM
%url:http://rcfunge98.com/rcsfingers.html
%desc:Terminal control functions
%condition:defined(HAVE_NCURSES)
%safe:true
%begin-instrs
#I	name	desc
C	clear_screen	Clear screen
D	go_down	Move cursor down n lines
G	goto_xy	Goto cursor position x,y (home is 0,0)
H	go_home	Move cursor to home
L	clear_to_eol	Clear from cursor to end of line
S	clear_to_eos	Clear from cursor to end of screen
U	go_up	Move cursor up n lines
%end
