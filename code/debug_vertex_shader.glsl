#version 330

layout (location = 0) in vec3 Position;

uniform mat4 World;
uniform mat4 Projection;
uniform vec4 InColor; 

out vec4 Color; 

void main()
{
    gl_Position = Projection * World * vec4(Position, 1.0);
    Color = InColor; 
    
}

