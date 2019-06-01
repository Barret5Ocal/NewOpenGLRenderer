void imguisetup()
{
    
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
    
}

void imguistuff()
{
    
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
    
}


void imguirender()
{
#if IMGUI
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif 
}