const char* vertexSource = R"glsl(
#version 150 core

in vec3 position;
in vec4 colour;

out vec4 Colour;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

void main()
{
	Colour = colour;
    gl_Position = proj * view * model * vec4(position, 1.0);
})glsl";

const char* fragmentSource = R"glsl(#version 150 core

in vec4 Colour;

out vec4 outColor;

void main()
{
    outColor = Colour;
}
)glsl";