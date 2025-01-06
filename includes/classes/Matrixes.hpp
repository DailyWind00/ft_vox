#pragma once

/// System includes
# include <iostream>
# include <array>
# include <cmath>

/// Global variables
enum class ROTATION {
	NONE,
	PITCH,      // x
	YAW,        // y
	ROLL        // z
};

typedef std::array<float, 2> vec2;
typedef std::array<float, 3> vec3;
typedef std::array<float, 4> vec4;
typedef std::array<float, 16> mat4; // using column format

// Default matrix class, generate an identity matrix at creation
class Matrix {
	protected:
		mat4	mat;

	public :
		Matrix();
		Matrix(const Matrix &matrix);
		Matrix(mat4 mat);
		virtual	~Matrix();

		// Operator overloads

		Matrix & operator=(const Matrix &matrix);
		Matrix & operator=(mat4 mat);

		Matrix operator+(const Matrix &rhs) const;
		Matrix operator-(const Matrix &rhs) const;

		Matrix operator*(const Matrix &rhs) const;
		vec3 operator*(const vec3 &rhs) const;
		vec4 operator*(const vec4 &rhs) const;


		// Getter

		const mat4 &getMatrix() const;
};

// Translation matrix class used for moving object, take 3 arguments at creation :
//     float x = x position multiplier      (default = 0)
//     float y = y position multiplier      (default = 0)
//     float z = z position multiplier      (default = 0)
class TranslationMatrix : public Matrix {
	public :
		TranslationMatrix(float x = 0, float y = 0, float z = 0);
		~TranslationMatrix();
};

// Scaling matrix class, the first constructor take 2 arguments at creation :
//     ROTATION axis = rotation axis        (default = NONE) # see ROTATION enum in config.hpp
//     float angle   = angle in degrees     (default = 0)
//
// The second constructor take 3 arguments at creation :
//     float pitch   = pitch rotation       (default = 0)
//     float yaw     = yaw   rotation       (default = 0)
//     float roll    = roll  rotation       (default = 0)
class RotationMatrix : public Matrix {
	public :
		RotationMatrix(ROTATION axis = ROTATION::NONE, float angle = 0);
		RotationMatrix(float pitch = 0, float yaw = 0, float roll = 0);
		~RotationMatrix();
};

// Scaling matrix class used for scaling object, take 3 arguments at creation :
//     float x = x axis size multiplier     (default = 1)
//     float y = y axis size multiplier     (default = 1)
//     float z = z axis size multiplier     (default = 1)
class ScalingMatrix : public Matrix {
	public :
		ScalingMatrix(float x = 1, float y = 1, float z = 1);
		~ScalingMatrix();
};

// Projection matrix class used for camera, take 4 arguments at creation :
//     float fov         = field of view in degrees (default =  45)
//     float aspectRatio = aspect ratio             (default =  1 )
//     float near        = near clipping plane      (default = 0.1)
//     float far         = far clipping plane       (default = 100)
class ProjectionMatrix : public Matrix {
	public :
		ProjectionMatrix(float fov = 45, float aspectRatio = 1, float near = 0.1, float far = 100);
		~ProjectionMatrix();
};