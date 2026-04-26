#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;

out vec3 fragNormal;
out vec4 fragColor;

void main()
{
    // normal in "world" space
    fragNormal = normalize(vec3(matModel * vec4(vertexNormal, 0.0)));
    fragColor = vertexColor;
    // final pos
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}