#version 330

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoordIn;

uniform mat4 World;
uniform mat4 Projection;

out vec4 Color; 
out vec2 TexCoord;

void main()
{
    gl_Position = Projection * World * vec4(Position, 1.0);
    Color = vec4(clamp(Position, 0.0, 1.0), 1.0);
    TexCoord = TexCoordIn;
}