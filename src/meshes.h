#include <stdlib.h>
#include <math.h>
#include <glad/glad.h>

// Vertex structure
typedef struct
{
  float position[3];
  float normal[3];
  float texCoord[2];
} Vertex;

// Function to generate a sphere mesh
void generateSphereMesh(float radius, int stacks, int slices, Vertex **outVertices, unsigned int **outIndices, int *outVertexCount, int *outIndexCount);

void setupSphereMesh(float radius, int stacks, int slices, unsigned int *vao, unsigned int *vbo, unsigned int *ebo, int *indexCountReturn);

void renderSphereMesh(unsigned int vao, int indexCount);