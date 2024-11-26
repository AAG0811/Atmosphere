#version 330 core

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform vec3 surfaceColor;
uniform vec3 lightPos;

out vec4 FragColor;

void main() {
  vec3 lightColor = vec3(1.0, 1.0, 1.0);

  vec3 ambient = 0.2 * lightColor;

  vec3 norm = normalize(Normal);
  vec3 lightDir = normalize(lightPos - FragPos);  
  vec3 viewDir = normalize(viewPos - FragPos);

  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = diff * lightColor;

  // vec3 reflectDir = reflect(-lightDir, norm); // replace this line for blinn phong
  // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0); // last value is material shininess
  vec3 specular = vec3(0.3) * spec;

  vec3 col =  (ambient + diffuse + specular) * surfaceColor;
  FragColor = vec4(col, 1.0);
}