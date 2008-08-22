%fingerprint-spec 1.2
%fprint:3DSP
%url:http://www.rcfunge98.com/rcfunge_manual.html#3DSP
%desc:3D space manipulation extension
%safe:true
%begin-instrs
#I	name	desc
A	add	Add two 3d vectors
B	sub	Subtract two 3d vectors
C	cross	Cross porduct of two vectors
D	dot	Dot product of two vector
L	length	Length of vector
M	mul	Multiply two 3d vectors
N	normalise	Normalize vector (sets length to 1)
P	matrixCopy	Copy a matrix
R	matrixRotate	Generate a rotation matrix
S	matrixScale	Generate a scale matrix
T	matrixTranslate	Generate a translation matrix
U	duplicate	Duplicate vector on top of stack
V	map	Map 3d point to 2d view
X	transform	Transform a vector using transformation matrix
Y	matrixMul	Multiply two matrices
Z	scale	Scale a vector
%end