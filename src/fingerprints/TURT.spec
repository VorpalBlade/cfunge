%fingerprint-spec 1.2
%fprint:TURT
%url:http://catseye.tc/projects/funge98/library/TURT.html
%desc:Simple Turtle Graphics Library
%safe:true
%begin-instrs
#I	name	desc
A	queryHeading	Query Position (x, y coordinates)
B	back	Back (distance in pixles)
C	penColour	Pen Colour (24-bit RGB)
D	showDisplay	Show Display (0 = no, 1 = yes)
E	queryPen	Query Pen (0 = up, 1 = down)
F	forward	Forward (distance in pixels)
H	setHeading	Set Heading (angle in degrees, relative to 0deg, east)
I	printDrawing	Print current Drawing (if possible)
L	turnLeft	Turn Left (angle in degrees)
N	clearPaper	Clear Paper with Colour (24-bit RGB)
P	penPosition	Pen Position (0 = up, 1 = down)
Q	queryPosition	Query Position (x, y coordinates)
R	turnRight	Turn Right (angle in degrees)
T	teleport	Teleport (x, y coords relative to origin; 00T = home)
U	queryBounds	Query Bounds (two pairs of x, y coordinates)
%end
