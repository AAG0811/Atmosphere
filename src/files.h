#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <libpng16/png.h>

// shaders

//  this function is used in the create_shader_program function
void createShader(unsigned int *shaderProg, const char *vertexPath, const char *fragmentPath);
// use this function to create shaders
unsigned int create_shader_program(const char *vertexPath, const char *fragmentPath);

// images
GLuint loadPNGTexture(const char *filename);
png_bytep loadPNGImage(const char *filename, int *width, int *height);
GLuint loadCubemapTexture(const char *faces[6]);