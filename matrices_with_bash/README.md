The bash file "matrix" has five functionalities that accept one or two matrices.
Matrix files are tab delimited, and can be entered through the command line or
through stdin.

./matrix dims [matrix 1]
    returns the dimensions of the matrix

./matrix tranpose [matrix 1]
    Turns an MxN matrix into an NxM matrix and prints to stdout

./matrix mean [matrix 1]
    Takes an MxN matrix and returns a 1xN row vector of columns averages to stdout

./matrix add [matrix 1][matrix 2]
    Adds two MxN matrices and returns the result to stdout

./matrix multiply [matrix 1][matrix 2]
    Multiplies a MxN and NxP matrix and returns a MxP matrix to stdout

If files return an error, windows machines can try $dos2unix filename
Make sure that the matrix file has execution permission using chmod +x matrix

The file "p1gradingscipt" can be run as a test suite
