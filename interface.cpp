#include "interface.h"
#include <iostream>
#include <array>

static void glfw_error_callback(int error, const char* description)
{
    std::cerr << "Glfw Error " << error << ": " << description << '\n' << std::flush;
}

mainwin_config mainwin_conf;

void* desk_Data_ReadOpen(ImGuiContext* ctx, ImGuiSettingsHandler* handler, const char* name) {
  return (void*) &mainwin_conf;
}

static std::array<std::string, 2> tokenize(const std::string& s) 
{
  std::array<std::string, 2> tokens;
  size_t pos = s.find("=");
  if(pos != std::string::npos) {
    tokens[0] = s.substr(0,pos);
    tokens[1] = s.substr(pos+1);
  }
  return tokens;
}

void desk_Data_ReadLine(ImGuiContext* ctx, ImGuiSettingsHandler* handler, void* entry, const char* line) {
  auto dc = (mainwin_config*)entry;
  auto tokens = tokenize(std::string(line));
  if(tokens[0] == "width")
    dc->width = atoi(tokens[1].c_str());
  if(tokens[0] == "height")
    dc->height = atoi(tokens[1].c_str());
  if(tokens[0] == "posx")
    dc->posx = atoi(tokens[1].c_str());
  if(tokens[0] == "posy")
    dc->posy = atoi(tokens[1].c_str());
  if(tokens[0] == "file")
    dc->recent_files.push_back(tokens[1]);
}

void desk_Data_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* out_buf) {
  auto window = *static_cast<GLFWwindow**>(handler->UserData);
  out_buf->append("[MainSize][desk]\n");
  int w,h;
  glfwGetWindowSize(window, &w, &h);
  mainwin_conf.width = w;
  mainwin_conf.height = h;
  int x,y;
  glfwGetWindowPos(window, &x, &y);
  mainwin_conf.posx = x;
  mainwin_conf.posy = y;
  out_buf->append(("width="+std::to_string(mainwin_conf.width)+'\n').c_str());
  out_buf->append(("height="+std::to_string(mainwin_conf.height)+'\n').c_str());
  out_buf->append(("posx="+std::to_string(mainwin_conf.posx)+'\n').c_str());
  out_buf->append(("posy="+std::to_string(mainwin_conf.posy)+'\n').c_str());
  for(const auto& fl : mainwin_conf.recent_files)
    out_buf->append(("file="+fl+'\n').c_str());
}

bool mainwin_config::valid() const {
  return width > 0 && height > 0;
}

static bool validIntervalsOverlap(int a0, int a1, int b0, int b1) {
  return a0 < b1 && b0 < a1;
}

static bool validRectsOverlap(int posx0, int posy0, int width0, int height0, int posx1, int posy1, int width1, int height1) {
  return validIntervalsOverlap(posx0, posx0+width0, posx1, posx1+width1) && validIntervalsOverlap(posy0, posy0+height0, posy1, posy1+height1);
}

static void checkSetWindowGeo(mainwin_config& conf) {
  int count;
  GLFWmonitor** monitors = glfwGetMonitors(&count);
  bool placementIsOK = false;
  if(conf.valid()) {
    //for eachmonitor check if current window rectangle stored in conf overlaps with monitor work area
    //if not then place window to first monitor
    for(int i=0;i<count;i++) {
      int posx, posy, width, height;
      glfwGetMonitorWorkarea(monitors[i], &posx, &posy, &width, &height);
      bool OK = validRectsOverlap(posx, posy, width, height, conf.posx, conf.posy, conf.width, conf.height);
      //it is also weird if in any direction window exceeds monitor workarea
      OK &= !((conf.posx < posx && conf.posx + conf.width > posx + width) || (conf.posy < posy && conf.posy + conf.height > posy + height));
      placementIsOK |= OK;
    }
  }
  
  if(!placementIsOK) {
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if(primary) {
      glfwGetMonitorWorkarea(primary, &conf.posx, &conf.posy, &conf.width, &conf.height);
      conf.posx += 11;
      conf.posy += 33;
      conf.width -= 22;
      conf.height -= 44;
    }
  }
}

bool imgui_interface::init() {

  glfwSetErrorCallback(glfw_error_callback);
   
  if (!glfwInit()) {
    std::cerr << "interface::init: ERROR: failed init glfw\n";
    return false;
  }
      
  // GL 3.2 + GLSL 150
  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac

  IMGUI_CHECKVERSION();
  auto imgui_ctx = ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  #ifndef NOIMPLOT
  ImPlot::CreateContext();
  #endif
  
  ImGuiSettingsHandler desk_ini_handler;
  desk_ini_handler.TypeName = "MainSize";
  desk_ini_handler.TypeHash = ImHashStr("MainSize");
  desk_ini_handler.ReadOpenFn = desk_Data_ReadOpen;
  desk_ini_handler.ReadLineFn = desk_Data_ReadLine;
  desk_ini_handler.WriteAllFn = desk_Data_WriteAll;
  desk_ini_handler.UserData = static_cast<void*>(&window);
  
  imgui_ctx->SettingsHandlers.push_back(desk_ini_handler);
  ImGui::LoadIniSettingsFromDisk(imgui_ctx->IO.IniFilename); // it seems to be perfectly fine to call this after context is created and before ImGui::NewFrame or any other ImGui drawing command
  // Create window with graphics context

  checkSetWindowGeo(mainwin_conf);

  window = glfwCreateWindow(mainwin_conf.width, mainwin_conf.height, "3D geometry viewer", NULL, NULL);
  if (window == NULL) {
    std::cerr << "interface::init: ERROR: failed to create glfw window\n";
    return false;
  }
  glfwSetWindowPos(window, mainwin_conf.posx, mainwin_conf.posy);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();
  //ImGui::StyleColorsLight();
  //ImGui::GetStyle().FrameBorderSize=1;
  //ImGui::GetStyle().FrameRounding=3;

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  return true;
}

void imgui_interface::close() {
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  #ifndef NOIMPLOT
  ImPlot::DestroyContext();
  #endif
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
}
