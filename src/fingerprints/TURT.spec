%fingerprint-spec 1.4
%fprint:TURT
%url:http://catseye.tc/projects/funge98/library/TURT.html
%desc:Simple Turtle Graphics Library
%condition:!defined(CFUN_NO_FLOATS) && !defined(CFUN_NO_TURT)
%safe:true
%begin-instrs
#I	name	desc
A	query_heading	Query Position (x, y coordinates)
B	back	Back (distance in pixles)
C	pen_colour	Pen Colour (24-bit RGB)
D	show_display	Show Display (0 = no, 1 = yes)
E	query_pen	Query Pen (0 = up, 1 = down)
F	forward	Forward (distance in pixels)
H	set_heading	Set Heading (angle in degrees, relative to 0deg, east)
I	print_drawing	Print current Drawing (if possible)
L	turn_left	Turn Left (angle in degrees)
N	clear_paper	Clear Paper with Colour (24-bit RGB)
P	pen_position	Pen Position (0 = up, 1 = down)
Q	query_position	Query Position (x, y coordinates)
R	turn_right	Turn Right (angle in degrees)
T	teleport	Teleport (x, y coords relative to origin; 00T = home)
U	query_bounds	Query Bounds (two pairs of x, y coordinates)
%end
