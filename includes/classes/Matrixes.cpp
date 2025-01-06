#include "Matrixes.hpp"

//// Amatrix
/// Constructors & Destructors
Matrix::Matrix() {
	// matrixes are declared like this where V is a vector 3 :
	//     Vx Vy Vz Vw(=1)
	//
	//      a  b  c  d
	//      e  f  g  h
	//      i  j  k  l
	//      m  n  o  p

	mat = mat4 { // Identity matrix in column format
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
}

Matrix::Matrix(const Matrix &matrix) {
	(*this) = matrix;
}

// Create a Matrix class with the matrix passed in argument
// Can be used to get the features of the Matrix class on a matrix
Matrix::Matrix(mat4 matrix) {
	mat = matrix;
}

Matrix & Matrix::operator=(const Matrix &matrix) {
	this->mat = matrix.mat;
	return *this;
}

Matrix & Matrix::operator=(mat4 matrix) {
	this->mat = matrix;
	return *this;
}

Matrix::~Matrix() {
}
/// ---

/// Operators
Matrix Matrix::operator+(const Matrix &rhs) const {
	Matrix result;

	for (int i = 0; i < 16; i++)
		result.mat[i] = mat[i] + rhs.mat[i];

	return result;
}

Matrix Matrix::operator-(const Matrix &rhs) const {
	Matrix result;

	for (int i = 0; i < 16; i++)
		result.mat[i] = mat[i] - rhs.mat[i];

	return result;
}

Matrix Matrix::operator*(const Matrix &rhs) const {
	// Multiplication of two matrixes is done by multiplying each row of the first matrix by each column of the second matrix
	// It looks like this :
	//     a  b  c  d     A  B  C  D     aA+bE+cI+dM  aB+bF+cJ+dN  aC+bG+cK+dO  aD+bH+cL+dP
	//     e  f  g  h  *  E  F  G  H  =  eA+fE+gI+hM  eB+fF+gJ+hN  eC+fG+gK+hO  eD+fH+gL+hP
	//     i  j  k  l     I  J  K  L     iA+jE+kI+lM  iB+jF+kJ+lN  iC+jG+kK+lO  iD+jH+kL+lP
	//     m  n  o  p     M  N  O  P     mA+nE+oI+pM  mB+nF+oJ+pN  mC+nG+oK+pO  mD+nH+oL+pP

	Matrix result;

	for (int i = 0; i < 16; i += 4) {
		for (int j = 0; j < 4; j++) {
			result.mat[i + j] = 0;
			for (int k = 0; k < 4; k++)
				result.mat[i + j] += mat[i + k] * rhs.mat[k * 4 + j];
		}
	}

	return result;
}

vec3 Matrix::operator*(const vec3 &rhs) const {
	// Multiplication of a matrix by a vector 3 is done by multiplying each row of the matrix by the vector
	// It looks like this :
	//     a  b  c  d     x     ax+by+cz+d
	//     e  f  g  h  *  y  =  ex+fy+gz+h
	//     i  j  k  l     z     ix+jy+kz+l

	vec3 result;

	for (int i = 0; i < 3; i++) {
		result[i] = 0;
		for (int j = 0; j < 3; j++)
			result[i] += mat[i * 4 + j] * rhs[j];
		result[i] += mat[i * 4 + 3];
	}

	return result;
}

vec4 Matrix::operator*(const vec4 &rhs) const {
	// Multiplication of a matrix by a vector 4 is done by multiplying each row of the matrix by the vector
	// It looks like this :
	//     a  b  c  d     x     ax+by+cz+dw
	//     e  f  g  h  *  y  =  ex+fy+gz+hw
	//     i  j  k  l     z     ix+jy+kz+lw
	//     m  n  o  p     w     mx+ny+oz+pw

	vec4 result;

	for (int i = 0; i < 4; i++) {
		result[i] = 0;
		for (int j = 0; j < 4; j++)
			result[i] += mat[i * 4 + j] * rhs[j];
	}

	return result;
}

/// Getters
const mat4 &Matrix::getMatrix() const {
	return mat;
}
/// ---
//// ---






//// Translation matrix
/// Constructors & Destructors
TranslationMatrix::TranslationMatrix(float x, float y, float z) {
	// Translation matrixes look like this :
	//     1  0  0  0
	//     0  1  0  0
	//     0  0  1  0
	//     x  y  z  1

	mat[12] = x; // = m
	mat[13] = y; // = n
	mat[14] = z; // = o

	// mat is an identity matrix if no translation is set
}

TranslationMatrix::~TranslationMatrix() {
}
/// ---
//// ---






//// Rotation matrix
/// Constructors & Destructors
RotationMatrix::RotationMatrix(ROTATION axis, float angle) {
	/// With c = cos(angle) and s = sin(angle)
	// Rotation matrixes on pitch axis look like this :
	//     1  0   0  0
	//     0  c  -s  0
	//     0  s   c  0
	//     0  0   0  1

	// Rotation matrixes on yaw axis look like this :
	//     c  0  s  0
	//     0  1  0  0
	//    -s  0  c  0
	//     0  0  0  1

	// Rotation matrixes on roll axis look like this :
	//     c -s  0  0
	//     s  c  0  0
	//     0  0  1  0
	//     0  0  0  1
	/// ---

	if (!angle)
		return ;

	float cosAngle = cos(angle * M_PI / 180.0);
	float sinAngle = sin(angle * M_PI / 180.0);

	switch (axis) {
		case ROTATION::NONE:
			break;
		case ROTATION::PITCH: // x
			mat[5]  = cosAngle;    // = f
			mat[6]  = -sinAngle;   // = g
			mat[9]  = sinAngle;    // = j
			mat[10] = cosAngle;    // = k
			break;
		case ROTATION::YAW: // y
			mat[0]  = cosAngle;    // = a
			mat[2]  = sinAngle;    // = c
			mat[8]  = -sinAngle;   // = i
			mat[10] = cosAngle;    // = k
			break;
		case ROTATION::ROLL: // z
			mat[0]  = cosAngle;    // = a
			mat[1]  = -sinAngle;   // = b
			mat[4]  = sinAngle;    // = e
			mat[5]  = cosAngle;    // = f
			break;
	}

	// mat is an identity matrix if no rotation or angle is set
}

RotationMatrix::RotationMatrix(float pitch, float yaw, float roll) {

	RotationMatrix pitch_rotate(ROTATION::PITCH, pitch);
	RotationMatrix yaw_rotate(ROTATION::YAW, yaw);
	RotationMatrix roll_rotate(ROTATION::ROLL, roll);

	mat = (roll_rotate * yaw_rotate * pitch_rotate).getMatrix();
}

RotationMatrix::~RotationMatrix() {
}
/// ---
//// ---






//// Scaling matrix
/// Constructors & Destructors
ScalingMatrix::ScalingMatrix(float x, float y, float z) {
	// Scaling matrixes look like this :
	//     x 0 0 0
	//     0 y 0 0
	//     0 0 z 0
	//     0 0 0 1

	mat[0] = x;  // = a
	mat[5] = y;  // = f
	mat[10] = z; // = k

	// mat is an identity matrix if no scaling is set
}

ScalingMatrix::~ScalingMatrix() {
}
/// ---
//// ---






//// Projection matrix
/// Constructors & Destructors
ProjectionMatrix::ProjectionMatrix(float fov, float aspectRatio, float near, float far) {
	// Projection matrixes look like this :
	//     f/a  0       0            0
	//      0   f       0            0
	//      0   0  (n+f)/(n-f)  2*n*f/(n-f)
	//      0   0      -1            0

	float f = 1.0f / tan(fov * M_PI / 360.0f);

	mat[0]  = f / aspectRatio;               // = a
	mat[5]  = f;                             // = f
	mat[10] = (far + near) / (near - far);   // = k
	mat[11] = -1;                            // = l
	mat[14] = 2 * far * near / (near - far); // = o

	// mat is an identity matrix if no projection is set
}

ProjectionMatrix::~ProjectionMatrix() {
}
/// ---
//// ---