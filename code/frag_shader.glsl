#version 330

in vec4 Color; 
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D Texture;

void main()
{
    vec4 TexColor = texture(Texture, TexCoord);
    
    FragColor = TexColor;
}