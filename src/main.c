#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "files.h"
#include "mathematics.h"
#include "meshes.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
// Base Rayleigh coefficient for a reference radius (e.g., 1.0 unit radius)
const float BASE_RAYLEIGH_COEFFICIENT[3] = {0.0025f, 0.0058f, 0.014f};
const float REFERENCE_RADIUS = 686.0f; // Reference radius for base Rayleigh coefficient

// Function to calculate Rayleigh coefficient based on planet radius
void calculateRayleighCoefficient(float planetRadius, float result[3])
{
  float scaleFactor = (REFERENCE_RADIUS / planetRadius) * (REFERENCE_RADIUS / planetRadius);
  result[0] = BASE_RAYLEIGH_COEFFICIENT[0] * scaleFactor;
  result[1] = BASE_RAYLEIGH_COEFFICIENT[1] * scaleFactor;
  result[2] = BASE_RAYLEIGH_COEFFICIENT[2] * scaleFactor;
}

#define MOUSE_SENSITIVITY 0.1f
#define CAMERA_SPEED 0.05f
#define PLANET_SCALE 16.0f

int Width = 1200;
int Height = 1000;

Camera camera = {
    .position = {0.0f, 0.0f, PLANET_SCALE * 1.2f},
    .target = {0.0f, 0.0f, -1.0f},
    .worldUp = {0.0f, 1.0f, 0.0f},
    .up = {0.0f, 1.0f, 0.0f},
    .right = {0.0f, 1.0f, 0.0f},
    .pitch = 0.0f,
    .yaw = -90.0f};
int firstMouse = 1;
float lastX = 500; // Initialize to the center of the window
float lastY = 400;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void setSunAngle(float sunVar[3], double angle)
{
  sunVar[1] = sin(angle);
  sunVar[2] = -cos(angle);
  return;
}

int main()
{
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window = glfwCreateWindow(Width, Height, "App", NULL, NULL);
  if (!window)
  {
    printf("Window Failed to Create! Terminating\n");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // load GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    printf("Window Failed to Init GLAD! Terminating\n");
    glfwTerminate();
    return -2;
  }
  // GLFW Callbacks
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  // GL Config
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  // shaders
  unsigned int basicShader, atmosphereShader;
  basicShader = create_shader_program("../src/shaders/basic.vs", "../src/shaders/phong.fs");
  atmosphereShader = create_shader_program("../src/shaders/copy.vs", "../src/shaders/copy.fs");

  // variables
  float lightPos[3] = {0.0f, PLANET_SCALE * 4.0f, 0.0f};

  float lightDirection[3] = {0.5f, -1.0f, 0.0f};
  normalize(lightDirection);
  float planetRadius = PLANET_SCALE;
  float atmosphereRadius = planetRadius * 1.1;
  // float rayleighCoefficient[3] = {0.58, 1.35, 3.31};
  float rayleighCoefficient[3];
  calculateRayleighCoefficient(PLANET_SCALE, rayleighCoefficient);
  float mieCoefficient[3] = {0.003, 0.003, 0.003};
  float rayleighColorFactor[3] = {0.65, 0.57, 0.475};
  float mieColorFactor[3] = {0.8, 0.6, 0.3};
  float mieDirectionality = 0.8;

  // meshes
  unsigned int vao,
      vbo, ebo;
  int indexCount;
  setupSphereMesh(PLANET_SCALE, 45, 45, &vao, &vbo, &ebo, &indexCount);
  unsigned int avao, avbo, aebo;
  int aindexCount;
  setupSphereMesh(atmosphereRadius, 45, 45, &avao, &avbo, &aebo, &aindexCount);

  int drawWireframe = 0;
  while (!glfwWindowShouldClose(window))
  {
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
      drawWireframe = !drawWireframe;

    if (drawWireframe)
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    glfwPollEvents();
    processInput(window);

    // rendering
    glClearColor(0.0f, 01.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Uniforms and Matrices
    float projectionMatrix[16];
    create_perspective_matrix(radians(45.0f), (float)Width / (float)Height, 0.1f, 300.0f, projectionMatrix);

    float viewMatrix[16];
    updateCameraVectors(&camera);
    createViewMatrix(viewMatrix, &camera);

    float modelMatrix[16];
    create_identity_matrix(modelMatrix);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // SUN ANGLE ----------------------------------------------------------------------------------
    // setSunAngle(lightDirection, (double)(glfwGetTime() * 0.3));
    lightPos[0] = sin(glfwGetTime() * 0.2) * atmosphereRadius;
    lightPos[1] = 0.0f;
    lightPos[2] = cos(glfwGetTime() * 0.2) * atmosphereRadius;
    lightDirection[0] = lightPos[0];
    lightDirection[2] = lightPos[2];

    // render planet
    glDepthMask(GL_TRUE); // Enable depth writing
    glDepthFunc(GL_LESS); // Default depth test
    glDisable(GL_BLEND);
    glUseProgram(basicShader);
    // set uniforms
    // uniform mat4 model;
    set_matrix_uniform(basicShader, "model", modelMatrix);
    // uniform mat4 view;
    set_matrix_uniform(basicShader, "view", viewMatrix);
    // uniform mat4 projection;
    set_matrix_uniform(basicShader, "projection", projectionMatrix);
    // uniform vec3 lightPos;
    set_vec3fv_uniform(basicShader, "lightPos", lightPos);
    // uniform vec3 viewPos;
    set_vec3fv_uniform(basicShader, "viewPos", camera.position);
    // uniform vec3 surfaceColor;
    set_vec3f_uniform(basicShader, "surfaceColor", 0.1f, 0.3f, 0.4f);

    glBindVertexArray(vao);
    renderSphereMesh(vao, indexCount);

    // // render atmosphere
    glDepthMask(GL_FALSE);                             // Disable depth writing
    glEnable(GL_BLEND);                                // Enable blending for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Standard blending function
    glDepthFunc(GL_LEQUAL);                            // Allow atmosphere to render at equal or greater depth

    glUseProgram(atmosphereShader);
    // scale
    // float sfac = atmosphereRadius / planetRadius;
    // scaleMatrix4x4ColumnMajor(modelMatrix, sfac, sfac, sfac);
    // uniform mat4 model;
    set_matrix_uniform(atmosphereShader, "model", modelMatrix);
    // uniform mat4 view;
    set_matrix_uniform(atmosphereShader, "view", viewMatrix);
    // uniform mat4 projection;
    set_matrix_uniform(atmosphereShader, "projection", projectionMatrix);

    // lighting uniforms

    // set_vec3fv_uniform(atmosphereShader, "ulightDirection", lightDirection);
    // set_float_uniform(atmosphereShader, "planetRadius", planetRadius);
    // set_float_uniform(atmosphereShader, "atmosphereRadius", atmosphereRadius);
    // // set_vec3fv_uniform(atmosphereShader, "rayleighCoefficient", rayleighCoefficient);
    // set_vec3fv_uniform(atmosphereShader, "mieCoefficient", mieCoefficient);
    // // set_vec3fv_uniform(atmosphereShader, "rayleighColorFactor", rayleighColorFactor);
    // set_vec3fv_uniform(atmosphereShader, "mieColorFactor", mieColorFactor);
    // set_float_uniform(atmosphereShader, "mieDirectionality", mieDirectionality);
    // set_int_uniform(atmosphereShader, "numSamples", 32);
    // set_int_uniform(atmosphereShader, "numLightSamples", 16);

    // copy uniforms
    set_vec3fv_uniform(atmosphereShader, "viewPos", camera.position);
    set_vec3fv_uniform(atmosphereShader, "sunPos", lightDirection);
    set_int_uniform(atmosphereShader, "viewSamples", 16);
    set_int_uniform(atmosphereShader, "lightSamples", 8);
    set_float_uniform(atmosphereShader, "sunIntensity", 20.0f);
    set_float_uniform(atmosphereShader, "planetRadius", planetRadius);
    set_float_uniform(atmosphereShader, "atmosphereRadius", atmosphereRadius);
    set_vec3f_uniform(atmosphereShader, "rCoeff", 5.8e-3f, 13.5e-3f, 33.1e-3f);
    set_float_uniform(atmosphereShader, "mCoeff", 21e-3f);
    set_float_uniform(atmosphereShader, "rHeight", 7.994);
    set_float_uniform(atmosphereShader, "mHeight", 1.200);
    set_float_uniform(atmosphereShader, "g", 0.888);
    set_float_uniform(atmosphereShader, "toneMappingFactor", 0.0);

    // renderSphereMesh(avao, aindexCount);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);

    glfwSwapBuffers(window);
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
  Width = width;
  Height = height;
}

void processInput(GLFWwindow *window)
{
  float velocity = CAMERA_SPEED;

  // Orbit Movement
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, 1);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
  {
    camera.position[0] += camera.forward[0] * velocity;
    camera.position[1] += camera.forward[1] * velocity;
    camera.position[2] += camera.forward[2] * velocity;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
  {
    camera.position[0] -= camera.forward[0] * velocity;
    camera.position[1] -= camera.forward[1] * velocity;
    camera.position[2] -= camera.forward[2] * velocity;
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
  {
    float factor[3];
    crossProduct(camera.forward, camera.up, factor);
    normalize(factor);
    factor[0] *= velocity;
    factor[1] *= velocity;
    factor[2] *= velocity;
    camera.position[0] -= factor[0];
    camera.position[1] -= factor[1];
    camera.position[2] -= factor[2];
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
  {
    float factor[3];
    crossProduct(camera.forward, camera.up, factor);
    normalize(factor);
    factor[0] *= velocity;
    factor[1] *= velocity;
    factor[2] *= velocity;
    camera.position[0] += factor[0];
    camera.position[1] += factor[1];
    camera.position[2] += factor[2];
  }
}
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
  if (firstMouse)
  {
    lastX = xpos;
    lastY = ypos;
    firstMouse = 0;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  // Update camera direction based on mouse movement
  camera.yaw -= xoffset * MOUSE_SENSITIVITY;
  camera.pitch += yoffset * MOUSE_SENSITIVITY;

  if (camera.pitch > 89.0f)
    camera.pitch = 89.0f;
  if (camera.pitch < -89.0f)
    camera.pitch = -89.0f;

  updateCameraVectors(&camera);
}