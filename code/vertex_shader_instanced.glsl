#version 130
// Per instance

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoordIn;

layout (location = 2) in vec2 GlyphPosition;
layout (location = 3) in vec2 GlyphSize;
layout (location = 4) in vec2 GlyphTextureCoordinates;
layout (location = 5) in vec2 GlyphTextureSize;

out vec2 FSTextureCoordinates;

void main()
{
    vec2 VSPosition = vec2(gl_VertexID >> 1, gl_VertexID & 1);
    FSTextureCoordinates = GlyphTextureCoordinates + GlyphTextureSize * VSPosition;
    
    gl_Position = vec4(GlyphPosition + GlyphSize * VSPosition, 0.0, 1.0);
}