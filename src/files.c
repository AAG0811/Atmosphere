#include "files.h"

// Function to read shader source code from a file
char *read_shader_file(const char *filepath)
{
  FILE *file = fopen(filepath, "r");
  if (!file)
  {
    fprintf(stderr, "Unable to open shader file: %s\n", filepath);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  // fseek(file, 0, SEEK_SET);
  rewind(file);

  char *code = malloc(length + 1);
  if (!code)
  {
    fprintf(stderr, "Unable to allocate memory for shader code\n");
    fclose(file);
    return NULL;
  }

  fread(code, 1, length, file);
  code[length] = '\0'; // Null-terminate the string
  fclose(file);

  return code;
}

// Function to compile a shader
unsigned int compile_shader(GLenum type, const char *source)
{
  unsigned int shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  // Check for compilation errors
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    char *shaderType;
    if (type == GL_FRAGMENT_SHADER)
    {
      shaderType = "Fragment";
    }
    else
    {
      shaderType = "Vertex";
    }

    char infoLog[1024];
    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
    fprintf(stderr, "%s Shader compilation error: %s\n", shaderType, infoLog);
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

// Function to create a shader program
unsigned int create_shader_program(const char *vertexPath, const char *fragmentPath)
{
  char *vertexCode = read_shader_file(vertexPath);
  char *fragmentCode = read_shader_file(fragmentPath);

  if (!vertexCode || !fragmentCode)
  {
    free(vertexCode);
    free(fragmentCode);
    return 0;
  }

  unsigned int vertexShader = compile_shader(GL_VERTEX_SHADER, vertexCode);
  unsigned int fragmentShader = compile_shader(GL_FRAGMENT_SHADER, fragmentCode);

  free(vertexCode);
  free(fragmentCode);

  if (!vertexShader || !fragmentShader)
  {
    return 0;
  }

  unsigned int program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  // Check for linking errors
  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success)
  {
    char infoLog[512];
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    fprintf(stderr, "Shader program linking error: %s\n", infoLog);
    glDeleteProgram(program);
    return 0;
  }

  // Clean up shaders as they are no longer needed
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  return program;
}
GLuint loadPNGTexture(const char *filename)
{
  FILE *fp = fopen(filename, "rb");
  if (!fp)
  {
    fprintf(stderr, "Failed to open file: %s\n", filename);
    return 0;
  }

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png)
  {
    fclose(fp);
    return 0;
  }

  png_infop info = png_create_info_struct(png);
  if (!info)
  {
    png_destroy_read_struct(&png, NULL, NULL);
    fclose(fp);
    return 0;
  }

  if (setjmp(png_jmpbuf(png)))
  {
    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);
    return 0;
  }

  png_init_io(png, fp);
  png_read_info(png, info);

  int width = png_get_image_width(png, info);
  int height = png_get_image_height(png, info);
  png_byte bit_depth = png_get_bit_depth(png, info);
  png_byte color_type = png_get_color_type(png, info);

  // Convert to RGBA
  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png);
  if (png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);
  if (bit_depth == 16)
    png_set_strip_16(png);
  if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGBA)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

  png_read_update_info(png, info);

  png_bytep row = (png_bytep)malloc(png_get_rowbytes(png, info));
  png_bytep *image_data = (png_bytep *)malloc(sizeof(png_bytep) * height);
  for (int y = 0; y < height; y++)
  {
    image_data[y] = (png_bytep)malloc(png_get_rowbytes(png, info));
    png_read_row(png, image_data[y], NULL);
  }

  // Create OpenGL texture
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  // Set the texture parameters for sRGB
  glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data[0]);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Generate mipmaps
  glGenerateMipmap(GL_TEXTURE_2D);

  // Cleanup
  for (int y = 0; y < height; y++)
  {
    free(image_data[y]);
  }
  free(image_data);
  png_destroy_read_struct(&png, &info, NULL);
  fclose(fp);

  return texture;
}

png_bytep loadPNGImage(const char *filename, int *width, int *height)
{
  FILE *fp = fopen(filename, "rb");
  if (!fp)
  {
    fprintf(stderr, "Failed to open file: %s\n", filename);
    return 0;
  }

  png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png)
  {
    fclose(fp);
    return 0;
  }

  png_infop info = png_create_info_struct(png);
  if (!info)
  {
    png_destroy_read_struct(&png, NULL, NULL);
    fclose(fp);
    return 0;
  }

  if (setjmp(png_jmpbuf(png)))
  {
    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);
    return 0;
  }

  png_init_io(png, fp);
  png_read_info(png, info);

  *width = png_get_image_width(png, info);
  *height = png_get_image_height(png, info);
  png_byte bit_depth = png_get_bit_depth(png, info);
  png_byte color_type = png_get_color_type(png, info);

  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png);
  if (png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);
  if (bit_depth == 16)
    png_set_strip_16(png);
  if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGBA)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

  png_read_update_info(png, info);

  png_bytep row = (png_bytep)malloc(png_get_rowbytes(png, info));
  png_bytep image_data = (png_bytep)malloc(*height * png_get_rowbytes(png, info));
  for (int y = 0; y < *height; y++)
  {
    png_read_row(png, image_data + y * png_get_rowbytes(png, info), NULL);
  }

  fclose(fp);
  png_destroy_read_struct(&png, &info, NULL);

  return image_data;
}
GLuint loadCubemapTexture(const char *faces[6])
{
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  for (int i = 0; i < 6; i++)
  {
    int width, height;
    png_bytep data = loadPNGImage(faces[i], &width, &height);
    if (!data)
    {
      fprintf(stderr, "Failed to load texture: %s\n", faces[i]);
      glDeleteTextures(1, &textureID);
      return 0;
    }

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    free(data);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // Unbind the texture

  return textureID;
}