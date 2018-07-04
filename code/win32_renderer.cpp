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

#include <stdio.h>

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
                    // NOTE(casey): File read successfully
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
                                     "OpenGl Pratice",
                                     WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                     CW_USEDEFAULT,
                                     CW_USEDEFAULT,
                                     ScreenWidth,
                                     ScreenHeight,
                                     0,
                                     0,
                                     Instance,
                                     0);
        
        
        
        //Result = stbi_write_png("Test.png", 512/4, 512/4, 4, temp_bitmap, 512);
        
        
        Win32InitOpenGL(Window);
        GLuint QuadVAO;
        GLuint VBO;
#if 0
        0 {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        1 {{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        2 {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        3 {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
#endif 
        
        vertex Vertices[] = { 
            // Pos      // Tex
            {-0.5f, 0.5f, 0.0f, 0.0f, 0.0f},
            {0.5f, -0.5f, 0.0f, 1.0f, 1.0f},
            {-0.5f, -0.5f, 0.0f, 1.0f, 0.0f}, 
            
            {-0.5f, 0.5f, 0.0f, 0.0f, 0.0f},
            {0.5f, 0.5f, 0.0f, 1.0f, 0.0f},
            {0.5f, -0.5f, 0.0f, 1.0f, 1.0f}
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
        
        read_results VertexShaderCode = Win32GetFileContents("..\\project\\code\\vertex_shader.glsl");
        read_results FragShaderCode = Win32GetFileContents("..\\project\\code\\frag_shader.glsl"); 
        GLuint ShaderProgram = glCreateProgram();
        GLuint VertexShaderObj = glCreateShader(GL_VERTEX_SHADER);
        GLuint FragShaderObj = glCreateShader(GL_FRAGMENT_SHADER);
        
        glShaderSource(VertexShaderObj, 1, &VertexShaderCode.Memory, &VertexShaderCode.Size);
        glShaderSource(FragShaderObj, 1, &FragShaderCode.Memory, &FragShaderCode.Size);
        
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
        
        glUseProgram(ShaderProgram);
        
        //GLuint ScaleLocation = glGetUniformLocation(ShaderProgram, "Scale");
        
        GLuint WorldLocation = glGetUniformLocation(ShaderProgram, "World");
        
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
            glClear(GL_COLOR_BUFFER_BIT);
            
            static float Scale = 0.0f;
            Scale += 0.001f;
            
            m4 World; 
            gb_mat4_identity(&World);
            //World.x.x = gb_sin(Scale);
            //World.y.y = gb_sin(Scale);
            //World.z.z = gb_sin(Scale);
            //World.w.w = gb_cos(Scale);
            
            //gb_mat4_identity(&World);
            //gb_mat4_rotate(&World, {0.0f, 0.0f, 1.0f}, Scale);
            
            glUniformMatrix4fv(WorldLocation, 1, GL_FALSE, &World.e[0]);
            
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            Win32RenderFrame(Window, ScreenWidth, ScreenHeight);
        }
    }
    return 0; 
}

