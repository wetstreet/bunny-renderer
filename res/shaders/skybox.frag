#version 330 core
out vec4 FragColor;

in vec3 texCoords;

uniform samplerCube skybox;

void main()
{
    vec3 envColor = textureLod(skybox, texCoords, 1.2).rgb; 
    
    FragColor = vec4(envColor, 1);
}