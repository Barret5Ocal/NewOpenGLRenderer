
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
    uint32 DataOffset; 
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
    
    uint8 *Atlas;
    uint32 Size; 
};

void GetFont(memory_arena *Arena, font_asset *FontAsset) 
{
    read_results Read = Win32GetFileContents("c:/windows/fonts/arial.ttf");
    
    //stbtt_fontinfo Font; 
    stbtt_InitFont(&FontInfo, (unsigned char *)Read.Memory, stbtt_GetFontOffsetForIndex((unsigned char *)Read.Memory, 0));
    
    int ascent; int descent; int lineGap;
    stbtt_GetFontVMetrics(&FontInfo, &ascent, &descent, &lineGap);
    float scale = stbtt_ScaleForPixelHeight(&FontInfo, 100.0f);
    int baseline = (int) (ascent*scale);
    FontAsset->ascent = ascent;
    FontAsset->descent = descent;
    FontAsset->lineGap = lineGap;
    FontAsset->baseline = baseline;
    FontAsset->scale = scale; 
    
    int x, y, x1, y1;
    stbtt_GetFontBoundingBox(&FontInfo, &x, &y, &x1, &y1);
    
    FontAsset->Character = (character_asset *)PushArray(Arena, '~' - '!' + 1, character_asset);
    
    FontAsset->Atlas = Arena->Memory + Arena->Used; 
    
    int TotalWidth = 0;
    int TotalHeight = 0;
    int TotalSurface = 0;
    int Count = 0;
    
    uint32 NextOffset = 0; 
    for(int Index = '!'; 
        Index <= '~';
        ++Index)
    {
        FontAsset->Count += 1; 
        
        int32 XOffset, YOffset; 
        int32 Width, Height;
        uint8 *Character = stbtt_GetCodepointBitmap(&FontInfo, 0, scale , Index, &Width, &Height, &XOffset, &YOffset);
        
        TotalWidth += Width;
        TotalHeight += Height; 
        Count++;
        TotalSurface += (Width * Height); 
        
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(&FontInfo, Index, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
        
        int advance,lsb;
        stbtt_GetCodepointHMetrics(&FontInfo, Index, &advance, &lsb);
        
        
        uint8 *Data = (uint8 *)PushArray(Arena, Width * Height * 4, uint8); 
        
        FontAsset->Character[Index - '!'] = {(char)Index, Width, Height, XOffset, YOffset, c_x1, c_x2, c_y1, c_y2, advance, lsb, Data, NextOffset};
        
        NextOffset += (Width * Height * 4) * sizeof(uint8);
        
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
    
    FontAsset->Size = NextOffset;
    
#if 0
    const int surface_sqrt = (int)gb_sqrt((float)TotalSurface) + 1;
    int TexWidth = (surface_sqrt >= 4096*0.7f) ? 4096 : (surface_sqrt >= 2048*0.7f) ? 2048 : (surface_sqrt >= 1024*0.7f) ? 1024 : 512;
    const int TEX_HEIGHT_MAX = 1024 * 32;
    
    stbtt_pack_context spc = {};
    stbtt_PackBegin(&spc, NULL, TexWidth, TEX_HEIGHT_MAX, 0, 1, NULL);
    
    int i =0; 
    //ImFontAtlasBuildPackCustomRects(atlas, spc.pack_info);
    
    stbrp_context* pack_context = (stbrp_context*)spc.pack_info;
    
    //ImVector<ImFontAtlas::CustomRect>& user_rects = atlas->CustomRects;
    
    stbrp_rect pack_rects[94];
    pack_rects.resize(user_rects.Size);
    memset(pack_rects.Data, 0, (size_t)pack_rects.size_in_bytes());
    for (int i = 0; i < user_rects.Size; i++)
    {
        pack_rects[i].w = user_rects[i].Width;
        pack_rects[i].h = user_rects[i].Height;
    }
    stbrp_pack_rects(pack_context, &pack_rects[0], pack_rects.Size);
    for (int i = 0; i < pack_rects.Size; i++)
        if (pack_rects[i].was_packed)
    {
        user_rects[i].X = pack_rects[i].x;
        user_rects[i].Y = pack_rects[i].y;
        IM_ASSERT(pack_rects[i].w == user_rects[i].Width && pack_rects[i].h == user_rects[i].Height);
        atlas->TexHeight = ImMax(atlas->TexHeight, pack_rects[i].y + pack_rects[i].h);
    }
    
#endif 
    
    
}

inline character_asset GetCharacter(font_asset *Font, char Character)
{
    return Font->Character[Character - '!']; //33
}

void DrawCharacter(GLuint ShaderProgram, v2 Position, v2 Scale)
{
    TIMED_BLOCK();
    
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
    
    //glDrawArrays(GL_TRIANGLES, 0, 6);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
}

// NOTE(Barret5Ocal): The error was caused by the DEBUG code. Make sure that your debug code is error resistent otherwise it will cause problems in the future. (I still don't know why it worked on my own computer)
void DrawString(GLuint ShaderProgram, font_asset *Font, char *Text, v2 Baseline, float Pixels)
{
    TIMED_BLOCK();
    Assert(Font && Font->Character);
    
    real32 AtX = Baseline.x;//67.0f;
    real32 AtY = Baseline.y;//128.0f;
    
    float FontScale =  (Pixels / (Font->ascent - Font->descent)) / Font->scale;
    
    glUseProgram(ShaderProgram);
    
    GLuint Texture;
    
    GLuint QuadVAO;
    GLuint VBO;
    GLuint EBO;
    
    vertex Vertices[] = { 
        // Pos      // Tex
        {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // Top Left      0
        {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // Bottom Right  1
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // Bottom Left   2
        
        //{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},  //              0
        {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // Top Right     3
        //{{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}   //              1
    };
    
    uint32 Indices[] = 
    {
        0, 1, 2,
        0, 3, 1
    };
    
    
    glGenVertexArrays(1, &QuadVAO);
    glBindVertexArray(QuadVAO); // NOTE(Barret5Ocal): Make sure you bind the VAO before you start gen-ing and and adding data to the other stuff 
    
    
    glGenBuffers(1, &VBO);
    
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    
    
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW); 
    
    glBindVertexArray(QuadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
    
    glGenTextures(1, &Texture);
    glActiveTexture(GL_TEXTURE0+0);
    glBindTexture(GL_TEXTURE_2D, Texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    
    int baseline = (int) (Font->ascent*Font->scale);
    
    for(char *Char = Text;
        *Char;
        Char++)
    {
        TIMED_BLOCK();
        
        if(*Char >= '!' && *Char <= '~')
        {
            character_asset CharData = GetCharacter(Font, *Char);
            
            Assert(CharData.Data);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CharData.Width, CharData.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, CharData.Data);
            
            glGenerateMipmap(GL_TEXTURE_2D);
            
            
            v2 Scale = {(float)CharData.Width * FontScale,(float)CharData.Height * FontScale};
            
            
            
            v2 Position = v2{AtX, AtY + baseline + (CharData.y1 * FontScale)};// + CharData.y2};
            DrawCharacter(ShaderProgram, Position, Scale);
            
            
            float ScaleAdvance = CharData.Width * FontScale; //CharData.advance * Font->scale * DEdit->AdvanceScale;
            AtX += ScaleAdvance;
            float KernAdvance = 0;
            char Char1 = *Char; 
            char Char2 = *(Char + 1);
            if(Char + 1 || *(Char + 1) == ' ')
                KernAdvance = (float)stbtt_GetCodepointKernAdvance(&FontInfo, Char1, Char2);
            
            float ExtraAdvance = Font->scale * KernAdvance * FontScale; 
            AtX += ExtraAdvance; 
        }
        else if(*Char == ' ')
        {
            AtX += 33 * FontScale;
        }
        else if(*Char == '\n')
        {
            AtX = Baseline.x;//67.0f; 
            AtY += (Font->ascent - Font->descent + Font->lineGap) * Font->scale * FontScale;
        }
        else if(*Char == '\t')
        {
            AtX += 33 * FontScale * 5;
        }
    }
    
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &QuadVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    
    BoxDebug = 6; 
    
}

void DrawString_Instanced(GLuint ShaderProgram, font_asset *Font, char *Text, v2 Baseline, float Pixels, v4  Color)
{
    TIMED_BLOCK();
    
    real32 AtX = Baseline.x;//67.0f;
    real32 AtY = Baseline.y;//128.0f;
    
    float FontScale =  (Pixels / (Font->ascent - Font->descent)) / Font->scale;
    
    int baseline = (int) (Font->ascent*Font->scale);
    
    m4 PositionArray[100] = {};
    u32 TextureOffsetArray[100] = {};
    
    m4 Ortho;
    gb_mat4_ortho3d(&Ortho, 0, ScreenWidth, ScreenHeight, 0, -1, 1);
    
    i32 Index = 0;
    
    for(char *Char = Text;
        *Char;
        Char++)
    {
        
        if(*Char >= '!' && *Char <= '~')
        {
            character_asset CharData = GetCharacter(Font, *Char);
            
            v2 Scale = {(float)CharData.Width * FontScale,(float)CharData.Height * FontScale};
            
            v2 Position = v2{AtX, AtY + baseline + (CharData.y1 * FontScale)};// + CharData.y2};
            
            m4 World; 
            gb_mat4_identity(&World);
            gb_mat4_translate(&World, {Position.x, Position.y, 0.0f});
            m4 ScaleM;
            gb_mat4_scale(&ScaleM, {Scale.x, Scale.y, 0.0f});
            World = World * ScaleM;
            
            PositionArray[Index] = World;
            TextureOffsetArray[Index] = CharData.DataOffset;
            
            Index++;
            
            float ScaleAdvance = CharData.Width * FontScale; 
            AtX += ScaleAdvance;
            float KernAdvance = 0;
            char Char1 = *Char; 
            char Char2 = *(Char + 1);
            if(Char + 1 || *(Char + 1) == ' ')
                KernAdvance = (float)stbtt_GetCodepointKernAdvance(&FontInfo, Char1, Char2);
            
            float ExtraAdvance = Font->scale * KernAdvance * FontScale; 
            AtX += ExtraAdvance; 
        }
        else if(*Char == ' ')
        {
            AtX += 33 * FontScale;
        }
        else if(*Char == '\n')
        {
            AtX = Baseline.x;//67.0f; 
            AtY += (Font->ascent - Font->descent + Font->lineGap) * Font->scale * FontScale;
        }
        else if(*Char == '\t')
        {
            AtX += 33 * FontScale * 5;
        }
    }
    
    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m4) * 100, &PositionArray[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    unsigned int instanceTex;
    glGenBuffers(1, &instanceTex);
    glBindBuffer(GL_ARRAY_BUFFER, instanceTex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(u32), &TextureOffsetArray[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    
    vertex Vertices[] = { 
        // Pos      // Tex
        {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // Top Left      0
        {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // Bottom Right  1
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // Bottom Left   2
        
        //{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},  //              0
        {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}, // Top Right     3
        //{{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}   //              1
    };
    
    uint32 Indices[] = 
    {
        0, 1, 2,
        0, 3, 1
    };
    
    
    GLuint QuadVAO;
    GLuint VBO;
    GLuint EBO;
    
    glGenVertexArrays(1, &QuadVAO);
    glBindVertexArray(QuadVAO); // NOTE(Barret5Ocal): Make sure you bind the VAO before you start gen-ing and and adding data to the other stuff 
    
    glGenBuffers(1, &VBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW); 
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(v4), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.
    
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer
    glVertexAttribPointer(2, 2, GL_INT, GL_FALSE, sizeof(u32), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(3, 1); // tell OpenGL this is an instanced vertex attribute.
    
    GLuint Texture;
    
    glGenTextures(1, &Texture);
    glActiveTexture(GL_TEXTURE0+0);
    glBindTexture(GL_TEXTURE_2D, Texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CharData.Width, CharData.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, CharData.Data);
    
    glGenerateMipmap(GL_TEXTURE_2D);
    
    
    
}
