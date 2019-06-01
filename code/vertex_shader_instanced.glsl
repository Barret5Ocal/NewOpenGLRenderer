#version 130

// Per instance
in vec2 GlyphPosition;
in vec2 GlyphSize;
in vec2 GlyphTextureCoordinates;
in vec2 GlyphTextureSize;
in int VSColor;

out vec2 FSTextureCoordinates;
out vec4 FSColor;

uniform sampler1D FontColor;

void main()
{
    vec2 VSPosition = vec2(gl_VertexID >> 1, gl_VertexID & 1);
    FSTextureCoordinates = GlyphTextureCoordinates + GlyphTextureSize * VSPosition;
    FSColor = texelFetch(FontColor, VSColor, 0);
    
    gl_Position = vec4(GlyphPosition + GlyphSize * VSPosition, 0.0, 1.0);
}
