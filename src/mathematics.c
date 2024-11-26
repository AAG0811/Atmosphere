#include "mathematics.h"

void printMatrix(const float matrix[16])
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      printf("%.2f ", matrix[i * 4 + j]);
    }
    printf("\n");
  }
}
void printVec3(const float vector[3])
{
  printf("%.2f ", vector[0]);
  printf("%.2f ", vector[1]);
  printf("%.2f ", vector[2]);
  printf("\n");
}
float radians(float degrees)
{
  return degrees * (M_PI / 180.0f);
}
void normalize(float *v)
{
  float length = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  if (length > 0)
  {
    v[0] /= length;
    v[1] /= length;
    v[2] /= length;
  }
}
void crossProduct(float *A, float *B, float *Result)
{
  Result[0] = A[1] * B[2] - A[2] * B[1]; // Result.x
  Result[1] = A[2] * B[0] - A[0] * B[2]; // Result.y
  Result[2] = A[0] * B[1] - A[1] * B[0]; // Result.z
}
void multiplyMatrices4x4(const float A[16], const float B[16], float result[16])
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      result[i * 4 + j] = 0; // Initialize result element
      for (int k = 0; k < 4; k++)
      {
        result[i * 4 + j] += A[i * 4 + k] * B[k * 4 + j]; // Perform multiplication and sum
      }
    }
  }
}
void addVectors(const float A[3], const float B[3], float result[3])
{
  result[0] = A[0] + B[0];
  result[1] = A[1] + B[1];
  result[2] = A[2] + B[2];
}
void subtractVectors(const float A[3], const float B[3], float result[3])
{
  result[0] = A[0] - B[0];
  result[1] = A[1] - B[1];
  result[2] = A[2] - B[2];
}
// basic matrix functions
void create_identity_matrix(float *matrix)
{
  matrix[0] = 1; // Row 0, Column 0
  matrix[1] = 0; // Row 0, Column 1
  matrix[2] = 0; // Row 0, Column 2
  matrix[3] = 0; // Row 0, Column 3

  matrix[4] = 0; // Row 1, Column 0
  matrix[5] = 1; // Row 1, Column 1
  matrix[6] = 0; // Row 1, Column 2
  matrix[7] = 0; // Row 1, Column 3

  matrix[8] = 0;  // Row 2, Column 0
  matrix[9] = 0;  // Row 2, Column 1
  matrix[10] = 1; // Row 2, Column 2
  matrix[11] = 0; // Row 2, Column 3

  matrix[12] = 0; // Row 3, Column 0
  matrix[13] = 0; // Row 3, Column 1
  matrix[14] = 0; // Row 3, Column 2
  matrix[15] = 1; // Row 3, Column 3
}
void translateMatrix(float *matrix, float tx, float ty, float tz)
{
  matrix[12] += tx; // Translate in x (4th column, 1st row)
  matrix[13] += ty; // Translate in y (4th column, 2nd row)
  matrix[14] += tz; // Translate in z (4th column, 3rd row)
}
void scaleMatrix4x4ColumnMajor(float *matrix, float sx, float sy, float sz)
{
  // Set the diagonal values for scaling
  matrix[0] = sx;    // Scale x-axis
  matrix[5] = sy;    // Scale y-axis
  matrix[10] = sz;   // Scale z-axis
  matrix[15] = 1.0f; // Homogeneous coordinate
}

// perspective matrix
void create_perspective_matrix(float fov, float aspect, float near, float far, float *matrix)
{
  float f = 1.0f / tanf(fov / 2.0f); // Calculate the cotangent of the angle
  float nf = 1.0f / (near - far);

  matrix[0] = f / aspect;
  matrix[1] = 0.0f;
  matrix[2] = 0.0f;
  matrix[3] = 0.0f;

  matrix[4] = 0.0f;
  matrix[5] = f;
  matrix[6] = 0.0f;
  matrix[7] = 0.0f;

  matrix[8] = 0.0f;
  matrix[9] = 0.0f;
  matrix[10] = (far + near) * nf;
  matrix[11] = -1.0f;

  matrix[12] = 0.0f;
  matrix[13] = 0.0f;
  matrix[14] = (2 * far * near) * nf;
  matrix[15] = 0.0f; // Row 3, Column 3
}

// view matrix
void createViewMatrix(float *viewMatrix, Camera *camera)
{
  // float forward[3], right[3], up[3];

  // Calculate the forward vector
  // camera->forward[0] = camera->target[0] - camera->position[0];
  // camera->forward[1] = camera->target[1] - camera->position[1];
  // camera->forward[2] = camera->target[2] - camera->position[2];
  normalize(camera->forward);

  // Calculate the camera->right vector
  camera->right[0] = camera->up[1] * camera->forward[2] - camera->up[2] * camera->forward[1];
  camera->right[1] = camera->up[2] * camera->forward[0] - camera->up[0] * camera->forward[2];
  camera->right[2] = camera->up[0] * camera->forward[1] - camera->up[1] * camera->forward[0];
  normalize(camera->right);

  // Recalculate the up vector
  camera->up[0] = camera->forward[1] * camera->right[2] - camera->forward[2] * camera->right[1];
  camera->up[1] = camera->forward[2] * camera->right[0] - camera->forward[0] * camera->right[2];
  camera->up[2] = camera->forward[0] * camera->right[1] - camera->forward[1] * camera->right[0];

  // Build the view matrix
  viewMatrix[0] = camera->right[0];
  viewMatrix[1] = camera->up[0];
  viewMatrix[2] = -camera->forward[0];
  viewMatrix[3] = 0.0f;

  viewMatrix[4] = camera->right[1];
  viewMatrix[5] = camera->up[1];
  viewMatrix[6] = -camera->forward[1];
  viewMatrix[7] = 0.0f;

  viewMatrix[8] = camera->right[2];
  viewMatrix[9] = camera->up[2];
  viewMatrix[10] = -camera->forward[2];
  viewMatrix[11] = 0.0f;

  viewMatrix[12] = -(camera->right[0] * camera->position[0] + camera->right[1] * camera->position[1] + camera->right[2] * camera->position[2]);
  viewMatrix[13] = -(camera->up[0] * camera->position[0] + camera->up[1] * camera->position[1] + camera->up[2] * camera->position[2]);
  viewMatrix[14] = camera->forward[0] * camera->position[0] + camera->forward[1] * camera->position[1] + camera->forward[2] * camera->position[2];
  viewMatrix[15] = 1.0f;
}
void getRotationMatrix(float *originalViewMatrix, float *rotationMatrix)
{
  // Copy the rotation part (upper 3x3) of the original view matrix
  rotationMatrix[0] = originalViewMatrix[0]; // m00
  rotationMatrix[1] = originalViewMatrix[1]; // m01
  rotationMatrix[2] = originalViewMatrix[2]; // m02
  rotationMatrix[3] = 0.0f;                  // m03

  rotationMatrix[4] = originalViewMatrix[4]; // m10
  rotationMatrix[5] = originalViewMatrix[5]; // m11
  rotationMatrix[6] = originalViewMatrix[6]; // m12
  rotationMatrix[7] = 0.0f;                  // m13

  rotationMatrix[8] = originalViewMatrix[8];   // m20
  rotationMatrix[9] = originalViewMatrix[9];   // m21
  rotationMatrix[10] = originalViewMatrix[10]; // m22
  rotationMatrix[11] = 0.0f;                   // m23

  rotationMatrix[12] = 0.0f; // m30
  rotationMatrix[13] = 0.0f; // m31
  rotationMatrix[14] = 0.0f; // m32
  rotationMatrix[15] = 1.0f; // m33
}
void updateCameraVectors(Camera *camera)
{
  float front[3];
  front[0] = cos(radians(camera->pitch)) * cos(radians(camera->yaw));
  front[1] = sin(radians(camera->pitch));
  front[2] = cos(radians(camera->pitch)) * sin(radians(camera->yaw));
  normalize(front);
  camera->forward[0] = front[0];
  camera->forward[1] = front[1];
  camera->forward[2] = front[2];
  // recalute up and right vectors
  crossProduct(camera->forward, camera->worldUp, camera->right);
  normalize(camera->right);
  crossProduct(camera->right, camera->forward, camera->up);
  normalize(camera->up);
  addVectors(camera->position, camera->forward, camera->target);
}

// set uniforms in shader program
void set_matrix_uniform(GLuint program, const char *uniformName, float *matrix)
{
  // Get the location of the uniform
  GLint location = glGetUniformLocation(program, uniformName);
  if (location == -1)
  {
    fprintf(stderr, "Could not find uniform %s\n", uniformName);
    return;
  }

  // Set the matrix uniform
  glUniformMatrix4fv(location, 1, GL_FALSE, matrix);
}
void set_int_uniform(GLuint program, const char *uniformName, int value)
{
  // Get Location
  GLint location = glGetUniformLocation(program, uniformName);
  if (location == -1)
  {
    fprintf(stderr, "Could not find uniform %s\n", uniformName);
    return;
  }
  // Set Int Unifrom
  glUniform1i(location, value);
}
void set_float_uniform(GLuint program, const char *uniformName, float value)
{
  // Get Location
  GLint location = glGetUniformLocation(program, uniformName);
  if (location == -1)
  {
    fprintf(stderr, "Could not find uniform %s\n", uniformName);
    return;
  }
  // Set Int Unifrom
  glUniform1f(location, value);
}
void set_vec3f_uniform(GLuint program, const char *uniformName, float x, float y, float z)
{
  // Get Location
  GLint location = glGetUniformLocation(program, uniformName);
  if (location == -1)
  {
    fprintf(stderr, "Could not find uniform %s\n", uniformName);
    return;
  }
  // Set Int Unifrom
  glUniform3f(location, x, y, z);
}
void set_vec3fv_uniform(GLuint program, const char *uniformName, const float vector3[3])
{
  // Get Location
  GLint location = glGetUniformLocation(program, uniformName);
  if (location == -1)
  {
    fprintf(stderr, "Could not find uniform %s\n", uniformName);
    return;
  }
  // Set Int Unifrom
  glUniform3fv(location, 1, vector3);
}