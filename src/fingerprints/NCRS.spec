%fingerprint-spec 1.3
%fprint:NCRS
%url:http://www.imaginaryrobots.net/projects/funge/myexts.txt
%desc:ncurses extension
%condition:defined(HAVE_NCURSES)
%safe:true
%begin-instrs
#I  name            desc
B   beep            Beep
C   clear           Clear all or part of the screen
E   toggle_echo     Set echo mode
G   get             Get character
I   init            Initialise and end curses mode
K   toggle_keypad   Set keypad mode
M   goto_xy         Move cursor to x,y
N   toggle_input    Toggle input mode
P   put             Put the character at cursor
R   refresh         Refresh window
S   write           Write string at cursor
U   unget           Unget character
%end
