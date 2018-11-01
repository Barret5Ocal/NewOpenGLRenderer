
struct character_asset
{
    char Codepoint;
    int32 Width;
    int32 Height;
    int32 XOffset; 
    int32 YOffset; 
    int32 x1;
    int32 x2;
    int32 y1; 
    int32 y2; 
    int32 advance;
    int32 lsb;
    uint8 *Data;
};

struct font_asset
{
    int Count;
    character_asset *Character;
    float scale;
    int ascent; 
    int descent; 
    int lineGap;
    int baseline;
};

void GetFont(memory_arena *Arena, font_asset *FontAsset) 
{
    read_results Read = Win32GetFileContents("c:/windows/fonts/arial.ttf");
    
    //stbtt_fontinfo Font; 
    stbtt_InitFont(&FontInfo, (unsigned char *)Read.Memory, stbtt_GetFontOffsetForIndex((unsigned char *)Read.Memory, 0));
    
    int ascent; int descent; int lineGap;
    stbtt_GetFontVMetrics(&FontInfo, &ascent, &descent, &lineGap);
    float scale = stbtt_ScaleForPixelHeight(&FontInfo, 64.0f);
    int baseline = (int) (ascent*scale);
    FontAsset->ascent = ascent;
    FontAsset->descent = descent;
    FontAsset->lineGap = lineGap;
    FontAsset->baseline = baseline;
    FontAsset->scale = scale; 
    
    int x, y, x1, y1;
    stbtt_GetFontBoundingBox(&FontInfo, &x, &y, &x1, &y1);
    
    FontAsset->Character = (character_asset *)PushArray(Arena, '~' - '!' + 1, character_asset);
    for(int Index = '!'; 
        Index <= '~';
        ++Index)
    {
        FontAsset->Count += 1; 
        
        int32 XOffset, YOffset; 
        int32 Width, Height;
        uint8 *Character = stbtt_GetCodepointBitmap(&FontInfo, 0, scale , Index, &Width, &Height, &XOffset, &YOffset);
        
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(&FontInfo, Index, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
        
        int advance,lsb;
        stbtt_GetCodepointHMetrics(&FontInfo, Index, &advance, &lsb);
        
        
        uint8 *Data = (uint8 *)PushArray(Arena, Width * Height * 4, uint8); 
        
        FontAsset->Character[Index - '!'] = {(char)Index, Width, Height, XOffset, YOffset, c_x1, c_x2, c_y1, c_y2, advance, lsb, Data};
        
        uint8 *Source = Character; 
        uint8 *DestRow = Data;
        for(int32 Y = 0;
            Y < Height;
            ++Y)
        {
            uint32 *Dest = (uint32 *)DestRow; 
            for(int32 X = 0;
                X < Width;
                ++X)
            {
                uint8 Alpha = *Source++;
                *Dest++ = ((Alpha << 24)|
                           (Alpha << 16)|
                           (Alpha << 8)|
                           (Alpha << 0));
            }
            DestRow += (Width * 4); 
        }
        stbtt_FreeBitmap(Character, 0);
    }
}

inline character_asset GetCharacter(font_asset *Font, char Character)
{
    return Font->Character[Character - '!']; //33
}

void DrawCharacter(GLuint ShaderProgram, v2 Position, v2 Scale)
{
    DebugEntry(DEBUG_BOX, Position.x, Position.y, Scale.x, Scale.y, BoxDebug++);
    
    GLuint WorldLocation = glGetUniformLocation(ShaderProgram, "World");
    GLuint ProjectionLocation = glGetUniformLocation(ShaderProgram, "Projection");
    
    m4 Ortho;
    gb_mat4_ortho3d(&Ortho, 0, ScreenWidth, ScreenHeight, 0, -1, 1);
    
    m4 World; 
    gb_mat4_identity(&World);
    gb_mat4_translate(&World, {Position.x, Position.y, 0.0f});
    m4 ScaleM;
    gb_mat4_scale(&ScaleM, {Scale.x, Scale.y, 0.0f});
    World = World * ScaleM;
    
    glUniformMatrix4fv(WorldLocation, 1, GL_FALSE, &World.e[0]);
    glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, &Ortho.e[0]);
    
    //glLineWidth(3.0f);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    
}

void DrawString(GLuint ShaderProgram, font_asset *Font, char *Text, v2 Baseline, debug_edit *DEdit)
{
    glUseProgram(ShaderProgram);
    
    real32 AtX = 67.0f;
    real32 AtY = 128.0f;
    
    //real32 AtX = 0.0f;
    //real32 AtY = 0.0f;
    
    GLuint Texture;
    
    GLuint QuadVAO;
    GLuint VBO;
    
    vertex Vertices[] = { 
        // Pos      // Tex
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, 
        
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}
    };
    
    glGenVertexArrays(1, &QuadVAO);
    glGenBuffers(1, &VBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    
    glBindVertexArray(QuadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);  
    glBindVertexArray(QuadVAO);
    
    
    glGenTextures(1, &Texture);
    glBindTexture(GL_TEXTURE_2D, Texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    
    int baseline = (int) (Font->ascent*Font->scale);
    
    DebugEntry(DEBUG_BOX, AtX, AtY + baseline - (Font->descent * Font->scale) + 15, 41, 2, BoxDebug++);
    // NOTE(Barret5Ocal):It starts at the AtX and spans past the 'H' box.
    // The AtX of the next letter 'e' should start where this box ends but it does not. it starts little bit before that
    // this might be a problem with the projection. 
    
    for(char *Char = Text;
        *Char;
        Char++)
    {
        character_asset CharData = GetCharacter(Font, *Char);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CharData.Width, CharData.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, CharData.Data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        v2 Scale = {(float)CharData.Width,(float)CharData.Height};
        Scale *= DEdit->Scale; 
        
        float ScaleLSB = CharData.lsb  * Font->scale;
        
        
        v2 Position = v2{AtX + ScaleLSB, AtY + baseline + CharData.y1 + CharData.y2};
        DrawCharacter(ShaderProgram, Position, Scale);
        
        
        // NOTE(Barret5Ocal): Debug Graphics (Also, remember that debug graphics sometimes lies to you)
        {
            DebugEntry(DEBUG_POINT, AtX, AtY, 2, 2, 0); //RED Dot
            
            DebugEntry(DEBUG_POINT, Position.x, Position.y, 2, 2, 1); // GREEN Dot
            
            v2 TopLeft = {Position.x - CharData.Width, Position.y - CharData.Height}; 
            DebugEntry(DEBUG_POINT, TopLeft.x, TopLeft.y, 2, 2, 2); // blue Dot
            
            v2 Origin = {TopLeft.x - ScaleLSB, TopLeft.y - CharData.y1 * 2}; 
            DebugEntry(DEBUG_POINT, Origin.x, Origin.y, 2, 2, 3); // yellow Dot
        }
        
        {// NOTE(Barret5Ocal): Debug stuff
            AtX += ScaleLSB * DEdit->Addlsb; 
            AtX += CharData.x1 * DEdit->Addx1; 
            AtX += CharData.x2 * DEdit->Addx2; 
            AtX += CharData.XOffset * DEdit->AddXOffset; 
            
        }
        
        //AtX += CharData.Width/2; 
        int ScaleAdvance =  CharData.advance * Font->scale * DEdit->AdvanceScale;
        AtX += ScaleAdvance;
        //AtX += (ScaleAdvance/2);
        //AtX += c_x1 * 5; 
        //AtX += ScaleLSB;
        float KernAdvance = 0;
        char Char1 = *Char; 
        char Char2 = *(Char + 1);
        if(Char + 1)
            KernAdvance =stbtt_GetCodepointKernAdvance(&FontInfo, Char1, Char2);
        
        float ExtraAdvance = Font->scale * KernAdvance; 
        AtX += ExtraAdvance; 
        
        
    }
    
    float TestX = 67;
    DebugEntry(DEBUG_POINT, TestX, 200, 2, 2, 4); //RED Dot
    TestX += 41;
    DebugEntry(DEBUG_POINT, TestX, 200, 2, 2, 4); //RED Dot
    TestX += 31;
    DebugEntry(DEBUG_POINT, TestX, 200, 2, 2, 4); //RED Dot
    TestX += 15;
    DebugEntry(DEBUG_POINT, TestX, 200, 2, 2, 4); //RED Dot
    TestX += 34;
    DebugEntry(DEBUG_POINT, TestX, 200, 2, 2, 4); //RED Dot
    
    
    
    
    
    
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &QuadVAO);
    glDeleteBuffers(1, &VBO);
    
    BoxDebug = 6; 
}
