#include <windows.h>

static int Running = 1; 

#include "b50_timing.h"

#include <float.h>

#define GB_MATH_IMPLEMENTATION
#include "include\gb_math.h"
typedef gbVec2 v2;
typedef gbVec3 v3;
typedef gbVec4 v4;
typedef gbMat4 m4;
typedef gbQuat quaternion;

#define STB_IMAGE_IMPLEMENTATION
#include "include\stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include\stb_image_write.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "include\stb_sprintf.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "include\stb_truetype.h"

#include <stdint.h>
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

#define global_variable static 
#define local_persist static 
#define internal static 

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#define InvalidCodePath Assert(false)
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Kilobyte(Value) 1024 * Value
#define Megabyte(Value) 1024 * Kilobyte(Value)
#define Gigabyte(Value) 1024 * Megabyte(Value)
#define Terabyte(Value) 1024 * Gigabyte(Value)

#define ScreenWidth 1280
#define ScreenHeight 720

#include "win32_opengl.h"

#include "memory.cpp"

struct vertex
{
    v3 Pos;
    v2 UV;
};


struct read_results
{
    char *Memory; 
    int32 Size; 
};

read_results Win32GetFileContents(char *Filename)
{
    read_results Result = {}; 
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            uint32 FileSize32 = FileSize.QuadPart;
            Result.Memory = (char *)VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Result.Memory)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Memory, FileSize32, &BytesRead, 0) &&
                   (FileSize32 == BytesRead))
                {
                    Result.Size = FileSize32;
                }
                else 
                {
                    VirtualFree(Result.Memory, 0, MEM_RELEASE);
                    
                }
            }
        }
        CloseHandle(FileHandle);
    }
    return Result; 
}

struct character_asset
{
    char Codepoint;
    int32 Width;
    int32 Height;
    int32 XOffset; 
    int32 YOffset; 
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


stbtt_fontinfo FontInfo; 

void GetFont(memory_arena *Arena, font_asset *FontAsset) 
{
    read_results Read = Win32GetFileContents("c:/windows/fonts/arialbd.ttf");
    
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
        
        
        uint8 *Data = (uint8 *)PushArray(Arena, Width * Height * 4, uint8); 
        
        FontAsset->Character[Index - '!'] = {(char)Index, Width, Height, XOffset, YOffset, Data};
        
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

#include "debug.cpp"

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

void DrawString(GLuint ShaderProgram, font_asset *Font, char *Text, v2 Baseline)
{
    glUseProgram(ShaderProgram);
    
    real32 AtX = 67.0f;
    real32 AtY = 128.0f;
    
    //real32 AtX = 0.0f;
    //real32 AtY = 0.0f;
    
    GLuint Texture;
    
    GLuint QuadVAO;
    GLuint VBO;
    
#if 1
    vertex Vertices[] = { 
        // Pos      // Tex
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, 
        
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}
    };
#else 
    vertex Vertices[] = { 
        // Pos      // Tex
        {{0.0f, 2.0f, 0.0f}, {0.0f, 1.0f}},
        {{2.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, 
        
        {{0.0f, 2.0f, 0.0f}, {0.0f, 1.0f}},
        {{2.0f, 2.0f, 0.0f}, {1.0f, 1.0f}},
        {{2.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}
    };
#endif 
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
    
    for(char *Char = Text;
        *Char;
        Char++)
    {
        character_asset CharData = GetCharacter(Font, *Char);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CharData.Width, CharData.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, CharData.Data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        v2 Scale = {(float)CharData.Width,(float)CharData.Height};
        Scale /= 2; 
        
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(&FontInfo, CharData.Codepoint, Font->scale, Font->scale, &c_x1, &c_y1, &c_x2, &c_y2);
        
        int advance,lsb;
        stbtt_GetCodepointHMetrics(&FontInfo, CharData.Codepoint, &advance, &lsb);
        
        //AtX -= (c_x1);
        //AtX += 18;
        v2 Position = v2{AtX, AtY + baseline + c_y1};
        DebugEntry(DEBUG_POINT, AtX, AtY, 2, 2, 0); //RED Dot
        
        DebugEntry(DEBUG_POINT, Position.x, Position.y, 2, 2, 1); // GREEN Dot
        
        v2 TopLeft = {Position.x - CharData.Width/2, Position.y - CharData.Height/2}; // NOTE(Barret5Ocal): CharData.Width and CharData.Height might be 2 times larger than it should be 
        DebugEntry(DEBUG_POINT, TopLeft.x, TopLeft.y, 2, 2, 2); // GREEN Dot
        
        float ScaleLSB = lsb  * Font->scale;
        
        v2 Origin = {TopLeft.x - ScaleLSB, TopLeft.y - c_y1}; // NOTE(Barret5Ocal): The scale of the characters might be to big 
        DebugEntry(DEBUG_POINT, Origin.x, Origin.y, 2, 2, 3); // GREEN Dot
        
        
        DrawCharacter(ShaderProgram, Position, Scale);
        
        
        int ScaleAdvance =  advance * Font->scale;
        AtX += ScaleAdvance;
        AtX += ScaleLSB;
        float KernAdvance = 0;
        char Char1 = *Char; 
        char Char2 = *(Char + 1);
        if(Char + 1)
            KernAdvance =stbtt_GetCodepointKernAdvance(&FontInfo, Char1, Char2);
        
        float ExtraAdvance = Font->scale * KernAdvance; 
        AtX += ExtraAdvance; 
        
    }
    
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &QuadVAO);
    glDeleteBuffers(1, &VBO);
    
    BoxDebug = 6; 
}


GLuint CreateShaderProgram(char *VertCode, int VertSize, char *FragCode, int FragSize)
{
    GLuint ShaderProgram = glCreateProgram();
    GLuint VertexShaderObj = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragShaderObj = glCreateShader(GL_FRAGMENT_SHADER);
    
    glShaderSource(VertexShaderObj, 1, &VertCode, &VertSize);
    glShaderSource(FragShaderObj, 1, &FragCode, &FragSize);
    
    glCompileShader(VertexShaderObj);
    glCompileShader(FragShaderObj);
    
    GLint success;
    glGetShaderiv(VertexShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        char Buffer[1024];
        GLchar InfoLog[1024];
        glGetShaderInfoLog(VertexShaderObj, sizeof(InfoLog), NULL, InfoLog);
        stbsp_sprintf(Buffer , "Error compiling shader type %d: '%s'\n", GL_VERTEX_SHADER, InfoLog);
        OutputDebugStringA(Buffer);
    }
    
    glGetShaderiv(FragShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        char Buffer[1024];
        GLchar InfoLog[1024];
        glGetShaderInfoLog(FragShaderObj, sizeof(InfoLog), NULL, InfoLog);
        stbsp_sprintf(Buffer , "Error compiling shader type %d: '%s'\n", GL_FRAGMENT_SHADER, InfoLog);
        OutputDebugStringA(Buffer);
        InvalidCodePath; 
    }
    
    glAttachShader(ShaderProgram, VertexShaderObj);
    glAttachShader(ShaderProgram, FragShaderObj);
    
    glLinkProgram(ShaderProgram);
    
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &success);
    if (success == 0) {
        char Buffer[1024];
        GLchar ErrorLog[1024];
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        stbsp_sprintf(Buffer, "Error linking shader program: '%s'\n", ErrorLog);
        OutputDebugStringA(Buffer);
        InvalidCodePath; 
    }
    
    return ShaderProgram; 
}

LRESULT CALLBACK
MainWindowProc(HWND Window,
               UINT Message,
               WPARAM WParam,
               LPARAM LParam)
{
    LRESULT Result = {};
    
    switch(Message)
    {
        case WM_DESTROY:
        {
            Running = 0;
        }break;
        case WM_CLOSE:
        {
            Running = 0;
        }break;
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        }break;
    }
    
    return Result; 
}

int WinMain(HINSTANCE Instance, 
            HINSTANCE PrevInstance,
            LPSTR CmdLine,
            int ShowCode)
{
    WNDCLASS WindowClass = {};
    WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = MainWindowProc;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "OpenGlPratice";
    
    if(RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowEx(0, 
                                     WindowClass.lpszClassName,
                                     "OpenGL Pratice",
                                     WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     ScreenWidth,
                                     ScreenHeight,
                                     0,
                                     0,
                                     Instance,
                                     0);
        
        memory_arena WorldArena; 
        InitMemoryArena(&WorldArena, Gigabyte(1));
        memory_arena FontArena = PushArena(&WorldArena, Megabyte(16));
        
        Win32InitOpenGL(Window);
        
        font_asset Font;
        GetFont(&FontArena, &Font);
        
        read_results VertexShaderCode = Win32GetFileContents("..\\code\\vertex_shader.glsl");//"..\\project\\code\\vertex_shader.glsl");
        read_results FragShaderCode = Win32GetFileContents("..\\code\\frag_shader.glsl");//"..\\project\\code\\frag_shader.glsl"); 
        GLuint ShaderProgram = CreateShaderProgram(VertexShaderCode.Memory, VertexShaderCode.Size, FragShaderCode.Memory, FragShaderCode.Size);
        
        read_results DebugVertexShaderCode = Win32GetFileContents("..\\code\\debug_vertex_shader.glsl");//"..\\project\\code\\vertex_shader.glsl");
        read_results DebugFragShaderCode = Win32GetFileContents("..\\code\\debug_frag_shader.glsl");//"..\\project\\code\\frag_shader.glsl"); 
        GLuint DebugShaderProgram = CreateShaderProgram(DebugVertexShaderCode.Memory, DebugVertexShaderCode.Size, DebugFragShaderCode.Memory, DebugFragShaderCode.Size);
        
        
        
        time_info TimeInfo = {};
        while(RunLoop(&TimeInfo, Running, 60))
        {
            MSG Message;
            while(PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }
            
            glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
            //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            DrawString(ShaderProgram, &Font, "Telgo", {400.0f, 400.0f});
            DrawDebugGraphics(DebugShaderProgram);
            DebugIndex = 0;
            Win32RenderFrame(Window, ScreenWidth, ScreenHeight);
        }
    }
    return 0; 
}

