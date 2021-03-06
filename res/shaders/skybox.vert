#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 texCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    texCoords = vec3(aPos.x, aPos.y, -aPos.z);

    mat4 rotView = mat4(mat3(view)); // remove translation from the view matrix
    vec4 pos = projection * rotView * vec4(aPos, 1.0f);

    gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
}