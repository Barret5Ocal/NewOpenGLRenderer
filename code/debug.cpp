struct debug_controls
{
    int BoxToggle;
    int PointToggle; 
    int LineToggle; 
};

debug_controls Controls = {1, 1,1};

enum debug_draw_type 
{
    DEBUG_POINT,
    DEBUG_BOX,
    DEBUG_LINE,
};

struct debug_draw_entry
{
    int Type;
    float X;
    float Y;
    float DimX;
    float DimY;
    int Color;
};

int DebugIndex = 0;
debug_draw_entry DebugEntries[50];

#define DebugEntry(Type, X, Y, DimX, DimY, Color) DebugEntries[DebugIndex++] = {Type, X, Y, DimX, DimY, Color}


int BoxDebug = 6; 

v4 DebugColors[] = 
{
    {1.0f, 0.0f, 0.0f, 1.0f}, //0
    {0.0f, 1.0f, 0.0f, 1.0f}, //1
    {0.0f, 0.0f, 1.0f, 1.0f}, //2
    {1.0f, 1.0f, 0.0f, 1.0f}, //3
    {1.0f, 0.0f, 1.0f, 1.0f}, //4
    {0.0f, 1.0f, 1.0f, 1.0f}, //5
    
    {1.0f, 0.0f, 0.0f, 0.5f}, //6 
    {0.0f, 1.0f, 0.0f, 0.5f}, //7
    {0.0f, 0.0f, 1.0f, 0.5f}, //8
    {1.0f, 1.0f, 0.0f, 0.5f}, //9
    {1.0f, 0.0f, 1.0f, 0.5f}, //10
    {0.0f, 1.0f, 1.0f, 0.5f}, //11
    
};


// TODO(Barret5Ocal): This is broken. Figure out how to do lines 
void DrawDebugLine(GLuint DebugShader, debug_draw_entry Entry)
{
    glUseProgram(DebugShader);
    
    GLuint QuadVAO;
    GLuint VBO;
    
    vertex Vertices[] = {
        
        {-1.0f, 1.0f, 0.0f},
        {1.0f, -1.0f, 0.0f}
    };
    
    
    glGenVertexArrays(1, &QuadVAO);
    glGenBuffers(1, &VBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    
    glBindVertexArray(QuadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);  
    glBindVertexArray(QuadVAO);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    
    GLuint WorldLocation = glGetUniformLocation(DebugShader, "World");
    GLuint ProjectionLocation = glGetUniformLocation(DebugShader, "Projection");
    GLuint ColorLocation = glGetUniformLocation(DebugShader, "InColor");
    
    m4 Ortho;
    gb_mat4_ortho3d(&Ortho, 0, ScreenWidth, ScreenHeight, 0, -1, 1);
    
    m4 World; 
    gb_mat4_identity(&World);
    gb_mat4_translate(&World, {Entry.X, Entry.Y, 0.0f});
    m4 ScaleM;
    gb_mat4_scale(&ScaleM, {Entry.DimX, Entry.DimY, 0.0f});
    World = World * ScaleM;
    
    glUniformMatrix4fv(WorldLocation, 1, GL_FALSE, &World.e[0]);
    glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, &Ortho.e[0]);
    glUniform4f(ColorLocation, DebugColors[Entry.Color].x, DebugColors[Entry.Color].y, DebugColors[Entry.Color].z,DebugColors[Entry.Color].w);
    
    //glDrawArrays(GL_LINES, 0, 6);
    
    glDrawArrays(GL_LINES, 0, 2);
    
    glDeleteVertexArrays(1, &QuadVAO);
    glDeleteBuffers(1, &VBO);
    
}

void DrawDebugBox(GLuint DebugShader, debug_draw_entry Entry)
{
    glUseProgram(DebugShader);
    
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
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    
    GLuint WorldLocation = glGetUniformLocation(DebugShader, "World");
    GLuint ProjectionLocation = glGetUniformLocation(DebugShader, "Projection");
    GLuint ColorLocation = glGetUniformLocation(DebugShader, "InColor");
    
    m4 Ortho;
    gb_mat4_ortho3d(&Ortho, 0, ScreenWidth, ScreenHeight, 0, -1, 1);
    
    m4 World; 
    gb_mat4_identity(&World);
    gb_mat4_translate(&World, {Entry.X, Entry.Y, 0.0f});
    m4 ScaleM;
    gb_mat4_scale(&ScaleM, {Entry.DimX, Entry.DimY, 0.0f});
    World = World * ScaleM;
    
    glUniformMatrix4fv(WorldLocation, 1, GL_FALSE, &World.e[0]);
    glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, &Ortho.e[0]);
    glUniform4f(ColorLocation, DebugColors[Entry.Color].x, DebugColors[Entry.Color].y, DebugColors[Entry.Color].z,DebugColors[Entry.Color].w);
    
    //glDrawArrays(GL_LINES, 0, 6);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDeleteVertexArrays(1, &QuadVAO);
    glDeleteBuffers(1, &VBO);
    
}

void DrawDebugGraphics(GLuint DebugShader)
{
    
    for(int32 Index = 0;
        Index <= DebugIndex;
        ++Index)
    {
        debug_draw_entry Entry = DebugEntries[Index];
        
        if((Entry.Type == DEBUG_POINT && Controls.PointToggle) || (Entry.Type == DEBUG_BOX && Controls.BoxToggle) )
        {
            DrawDebugBox(DebugShader, Entry);
        }
        else if((Entry.Type == DEBUG_LINE && Controls.LineToggle))
        {
            DrawDebugLine(DebugShader, Entry);
        }
    }
    
    
}


