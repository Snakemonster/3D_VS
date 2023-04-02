#version 450 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normals;
layout(location = 2) in vec2 texCoord;

out vec3 FragPos;
out vec3 Normal;
out mat4 View;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 matrixNormals;


void main(){
    TexCoords = texCoord;
    FragPos = vec3(view * model * vec4(position, 1.0));
    Normal = matrixNormals * normals;
    View = view;
    gl_Position = projection * view * model * vec4(position, 1.0);
}