#include <glad/glad.h>
#include <math.h>
#include <stdio.h>

typedef struct
{
  float position[3];
  float target[3];
  float up[3];
  float worldUp[3];
  float forward[3];
  float right[3];
  float pitch;
  float yaw;
} Camera;

// general utility functions
void printMatrix(const float matrix[16]);
void printVec3(const float vector[3]);
float radians(float degrees);
void normalize(float *v);
void crossProduct(float *A, float *B, float *Result);
void multiplyMatrices4x4(const float A[16], const float B[16], float result[16]);
void addVectors(const float A[3], const float B[3], float result[3]);
void subtractVectors(const float A[3], const float B[3], float result[3]);

// basic matrix functions
void create_identity_matrix(float *matrix);
void translateMatrix(float *matrix, float x_translation, float y_translation, float z_translation);
void scaleMatrix4x4ColumnMajor(float *matrix, float sx, float sy, float sz);

// perspective matrix
void create_perspective_matrix(float fov, float aspect, float near, float far, float *matrix);

// view matrix
// may need to change these functions
void createViewMatrix(float *viewMatrix, Camera *camera); // AKA LookAt(position, target, up) to rotate modify target
void getRotationMatrix(float *originalViewMatrix, float *rotationMatrix);
void updateCameraVectors(Camera *camera);

// set uniforms in shader program
void set_matrix_uniform(GLuint program, const char *uniformName, float *matrix);
void set_int_uniform(GLuint program, const char *uniformName, int value);
void set_float_uniform(GLuint program, const char *uniformName, float value);
void set_vec3f_uniform(GLuint program, const char *uniformName, float x, float y, float z);
void set_vec3fv_uniform(GLuint program, const char *uniformName, const float vector3[3]);