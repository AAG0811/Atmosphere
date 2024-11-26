#include "meshes.h"

// Function to generate a sphere mesh
void generateSphereMesh(float radius, int stacks, int slices, Vertex **outVertices, unsigned int **outIndices, int *outVertexCount, int *outIndexCount)
{
  int vertexCount = (stacks + 1) * (slices + 1);
  int indexCount = stacks * slices * 6;

  // Allocate memory for vertices and indices
  *outVertices = (Vertex *)malloc(vertexCount * sizeof(Vertex));
  *outIndices = (unsigned int *)malloc(indexCount * sizeof(unsigned int));

  Vertex *vertices = *outVertices;
  unsigned int *indices = *outIndices;

  int vertexIndex = 0;
  int indexIndex = 0;

  // Generate vertices
  for (int i = 0; i <= stacks; ++i)
  {
    float stackAngle = M_PI / 2 - i * M_PI / stacks; // angle from top to bottom
    float xy = radius * cosf(stackAngle);            // radius of the current stack
    float z = radius * sinf(stackAngle);             // z position of the current stack

    for (int j = 0; j <= slices; ++j)
    {
      float sliceAngle = j * 2 * M_PI / slices; // angle around the sphere

      float x = xy * cosf(sliceAngle);
      float y = xy * sinf(sliceAngle);

      // Position
      vertices[vertexIndex].position[0] = x;
      vertices[vertexIndex].position[1] = y;
      vertices[vertexIndex].position[2] = z;

      // Normal
      float length = sqrtf(x * x + y * y + z * z);
      vertices[vertexIndex].normal[0] = x / length;
      vertices[vertexIndex].normal[1] = y / length;
      vertices[vertexIndex].normal[2] = z / length;

      // Texture coordinates
      vertices[vertexIndex].texCoord[0] = (float)j / slices;
      vertices[vertexIndex].texCoord[1] = (float)i / stacks;

      vertexIndex++;
    }
  }

  // Generate indices
  for (int i = 0; i < stacks; ++i)
  {
    for (int j = 0; j < slices; ++j)
    {
      int first = i * (slices + 1) + j;
      int second = first + slices + 1;

      // First triangle of the quad
      indices[indexIndex++] = first;
      indices[indexIndex++] = second;
      indices[indexIndex++] = first + 1;

      // Second triangle of the quad
      indices[indexIndex++] = second;
      indices[indexIndex++] = second + 1;
      indices[indexIndex++] = first + 1;
    }
  }

  *outVertexCount = vertexCount;
  *outIndexCount = indexCount;
}

void setupSphereMesh(float radius, int stacks, int slices, unsigned int *vao, unsigned int *vbo, unsigned int *ebo, int *indexCountReturn)
{
  Vertex *vertices;
  unsigned int *indices;
  int vertexCount, indexCount;

  // Generate the sphere mesh
  generateSphereMesh(radius, stacks, slices, &vertices, &indices, &vertexCount, &indexCount);

  glGenVertexArrays(1, &*vao);
  glGenBuffers(1, &*vbo);
  glGenBuffers(1, &*ebo);

  glBindVertexArray(*vao);

  // Vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, *vbo);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

  // Element buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

  // Position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
  glEnableVertexAttribArray(0);

  // Normal attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Texture coordinate attribute
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);

  // Free the mesh data (now stored in OpenGL buffers)
  free(vertices);
  free(indices);
  *indexCountReturn = indexCount;
}

void renderSphereMesh(unsigned int vao, int indexCount)
{
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}