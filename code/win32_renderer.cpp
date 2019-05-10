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
#define STBI_MSC_SECURE_CRT
#include "include\stb_image_write.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "include\stb_sprintf.h"


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

//#include "win32_opengl.h"

#define FGL_IMPLEMENTATION
#include "include\final_dynamic_opengl.h"

#if IMGUI
#include "include\imgui.cpp"
#include "include\imgui.h"
#include "include\imgui_demo.cpp"
#include "include\imgui_draw.cpp"
#include "include\imgui_internal.h"
#include "include\imconfig.h"
#include "include\imgui_impl_opengl3.cpp"
#else 
#define STB_TRUETYPE_IMPLEMENTATION
#include "include\stb_truetype.h"

#endif

int LeftMouse = 0; 

struct win32_windowdim 
{
    int Width, Height; 
    int x, y;
    //int DisplayWidth, DisplayHeight; 
};


win32_windowdim Win32GetWindowDim(HWND Window)
{
    win32_windowdim Dim = {};
    
    RECT Rect = {};
    //GetClientRect(Window, &Rect);
    GetWindowRect(Window, &Rect);
    Dim.x = Rect.left;
    Dim.y = Rect.top;
    Dim.Width = Rect.right - Rect.left;
    Dim.Height = Rect.bottom - Rect.top;
    return Dim; 
}


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
            uint32 FileSize32 = (uint32)FileSize.QuadPart;
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


stbtt_fontinfo FontInfo; 

#include "debug.cpp"

#include "text.cpp"

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
        InvalidCodePath; 
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
        case WM_KEYDOWN:
        {
            if(WParam == VK_F5)
                Controls.BoxToggle = !Controls.BoxToggle; 
            if(WParam == VK_F6)
                Controls.PointToggle = !Controls.PointToggle; 
            
        }break;
        
        case WM_LBUTTONDOWN: 
        {
            LeftMouse = 1;
        }break;
        case WM_LBUTTONUP: 
        {
            LeftMouse = 0;
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
    WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    
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
        
#if 1
        HDC WindowDCGL = GetDC(Window);
        
        // Load opengl library without loading all the functions - functions are loaded separately later
        if (fglLoadOpenGL(false)) {
            
            // Fill out window handle (This is platform dependent!)
            fglOpenGLContextCreationParameters contextCreationParams = {};
#if defined(FGL_PLATFORM_WIN32)
            contextCreationParams.windowHandle.win32.deviceContext = WindowDCGL;// ... pass your current device context here
            // or
            contextCreationParams.windowHandle.win32.windowHandle = Window;// ... pass your current window handle here
#endif
            
            
            // Create context and load opengl functions
            fglOpenGLContext glContext = {};
            if (fglCreateOpenGLContext(&contextCreationParams, &glContext)) {
                fglLoadOpenGLFunctions();
                
                // ... load shader, whatever you want to do
                
                //fglDestroyOpenGLContext(&glContext);
            }
            //fglUnloadOpenGL();
        }
        ReleaseDC(Window, WindowDCGL);
#else 
        
        Win32InitOpenGL(Window);
#endif 
        win32_windowdim Dim = Win32GetWindowDim(Window);
        
#if IMGUI
        ImGui::CreateeContext();
        
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        
        io.DisplaySize = {(float)ScreenWidth, (float)ScreenHeight};
        float FrameRate = 60;
        float dt = 1 / FrameRate; 
        io.DeltaTime = dt;
        
        
        
        ImGui_ImplOpenGL3_Init(0);
        ImGui::StyleColorsDark();
        bool show_demo_window = true;
#endif 
        
        font_asset Font;
        GetFont(&FontArena, &Font);
        
        read_results VertexShaderCode = Win32GetFileContents("..\\project\\code\\vertex_shader.glsl");//"..\\project\\code\\vertex_shader.glsl");
        read_results FragShaderCode = Win32GetFileContents("..\\project\\code\\frag_shader.glsl");//"..\\project\\code\\frag_shader.glsl"); 
        GLuint ShaderProgram = CreateShaderProgram(VertexShaderCode.Memory, VertexShaderCode.Size, FragShaderCode.Memory, FragShaderCode.Size);
        
        read_results DebugVertexShaderCode = Win32GetFileContents("..\\project\\code\\debug_vertex_shader.glsl");//"..\\project\\code\\vertex_shader.glsl");
        read_results DebugFragShaderCode = Win32GetFileContents("..\\project\\code\\debug_frag_shader.glsl");//"..\\project\\code\\frag_shader.glsl"); 
        GLuint DebugShaderProgram = CreateShaderProgram(DebugVertexShaderCode.Memory, DebugVertexShaderCode.Size, DebugFragShaderCode.Memory, DebugFragShaderCode.Size);
        
        
        
        bool TextEditWindow = 1;
        
        
        int DebugGraphicsToggle = 0;
        
        time_info TimeInfo = {};
        while(RunLoop(&TimeInfo, 60))
        {
            MSG Message;
            while(PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }
            
            Dim = Win32GetWindowDim(Window);
            
#if IMGUI
            POINT Point = {}; 
            GetCursorPos(&Point);
            
            io.MousePos = {(float)Point.x - Dim.x - 5, (float)Point.y - Dim.y + 5};
            
            //io.MouseDown[0] = 0;
            io.MouseDown[0] = LeftMouse;
            
            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();
            
            //ImGui::PushAllowKeyboardFocus(true);
            io.WantCaptureKeyboard = true;
            io.WantTextInput = true;
            
            
            // NOTE(Barret5Ocal): GUI stuff
            if(TextEditWindow)
            {
                
                //ImGui::SetNextWindowSize({0.0f, 0.0f} );
                ImGui::Begin("Text Edit", &TextEditWindow, ImGuiWindowFlags_None);
                
                //ImGui::InputText("string", DEdit.Text, ArrayCount(DEdit.Text));
                //ImGui::Text("Scale Edit");
                ImGui::SliderFloat("Character Scale", &DEdit.Scale, -3.0f, 3.0f);
                ImGui::SliderFloat("Advance Scale", &DEdit.AdvanceScale, -3.0f, 3.0f);
                //ImGui::SliderFloat("Add to AtX", &DEdit.AtXAdd, -3.0f, 3.0f);
                
                
                if (ImGui::Button("Reset Values"))
                {
                    DEdit.Scale = 1.0f;
                    DEdit.AdvanceScale = 1.0f;
                    DEdit.AddXOffset = 0; 
                    DEdit.Addx1 = 0;
                    DEdit.Addx2 = 0;
                    DEdit.Addlsb = 0;
                }
                
                ImGui::BeginGroup();
                if(ImGui::Button("Toggle XOffset")){DEdit.AddXOffset = DEdit.AddXOffset ? 0 : 1;}
                if(DEdit.AddXOffset){ImGui::Text("On");}else {ImGui::Text("Off");}
                ImGui::EndGroup();
                ImGui::SameLine();
                ImGui::BeginGroup();
                if(ImGui::Button("Toggle x1")){DEdit.Addx1 = DEdit.Addx1 ? 0 : 1;}
                if(DEdit.Addx1){ImGui::Text("On");}else {ImGui::Text("Off");}
                ImGui::EndGroup();
                ImGui::SameLine();
                ImGui::BeginGroup();
                if(ImGui::Button("Toggle x2")){DEdit.Addx2 = DEdit.Addx2 ? 0 : 1;}
                if(DEdit.Addx2){ImGui::Text("On");}else {ImGui::Text("Off");}
                ImGui::EndGroup();
                ImGui::SameLine();
                ImGui::BeginGroup();
                if(ImGui::Button("Toggle lsb")){DEdit.Addlsb = DEdit.Addlsb ? 0 : 1;}
                if(DEdit.Addlsb){ImGui::Text("On");}else {ImGui::Text("Off");}
                ImGui::EndGroup();
                ImGui::BeginGroup();
                if(ImGui::Button("Toggle Debug Graphics")){DebugGraphicsToggle = DebugGraphicsToggle ? 0 : 1;}
                if(DebugGraphicsToggle){ImGui::Text("On");}else {ImGui::Text("Off");}
                ImGui::EndGroup();
                
                
                
                ImGui::End();
                
                
                ImGui::SetNextWindowSize({0.0f, 0.0f} );
                ImGui::Begin("Text Info", &TextEditWindow, ImGuiWindowFlags_None);
                //ImGui::BeginGroup();  
                
                ImGui::BeginGroup();
                ImGui::Text("Scale: %f", Font.scale);
                ImGui::Text("Ascent: %d", Font.ascent);
                ImGui::Text("Descent: %d", Font.descent);
                ImGui::Text("Line Gap: %d", Font.lineGap);
                ImGui::Text("Baseline: %d", Font.baseline);
                ImGui::EndGroup();
                ImGui::SameLine(0.0f, 30.0f);
                
                char *Char = DEdit.Text;
                while (*Char)
                {
                    if(*Char != ' ')
                    {
                        ImGui::BeginGroup();                                                   
                        character_asset CharData = GetCharacter(&Font, *Char);
                        ImGui::Text(&CharData.Codepoint);
                        ImGui::Text("Width: %d", CharData.Width);
                        ImGui::Text("Height: %d", CharData.Height);
                        ImGui::Text("XOffset: %d", CharData.XOffset);
                        ImGui::Text("YOffset: %d", CharData.YOffset);
                        ImGui::Text("x1: %d", CharData.x1);
                        ImGui::Text("x2: %d", CharData.x2);
                        ImGui::Text("y1: %d", CharData.y1);
                        ImGui::Text("y2: %d", CharData.y2);
                        ImGui::Text("advance: %f", CharData.advance * Font.scale);
                        ImGui::Text("lsb: %f", CharData.lsb * Font.scale);
                        ImGui::EndGroup();
                        
                        ImGui::SameLine();
                        //ImGui::EndGroup();
                    }
                    Char++;
                    
                }
                
                ImGui::End();
                
            }
            
            ImGui::Render();
#endif 
            
            glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
            //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            DrawString(ShaderProgram, &Font, "Helgo Dark\nstuff\nthings", {400.0f, 400.0f}, 0.5f);
            //DrawString(ShaderProgram, &Font, "AAAAA", {400.0f, 400.0f + 32.937062616749998f}, 0.5f, &DEdit);
            
            if(DebugGraphicsToggle)
                DrawDebugGraphics(DebugShaderProgram);
            
            DebugIndex = 0;
#if IMGUI
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif 
            HDC WindowDC = GetDC(Window);
            
            glViewport(0, 0, ScreenWidth, ScreenHeight);
            SwapBuffers(WindowDC);
            
            ReleaseDC(Window, WindowDC);
            //Win32RenderFrame(Window, ScreenWidth, ScreenHeight);
            
        }
    }
    return 0; 
}

