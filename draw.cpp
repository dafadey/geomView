#include "draw.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

#include <GLFW/glfw3.h>

#include "object.h"

static GLuint loadShaders(const std::string& VertexShaderCode, const std::string& FragmentShaderCode, const std::string& GeometryShaderCode = ""){

    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint GeometryShaderID = 0;
    if(!GeometryShaderCode.empty())
      GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);

    GLint Result = GL_FALSE;
    int InfoLogLength;

    std::cout << "building vertex shader...\n";
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
      std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
      glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
      std::cerr << "building vertex shader failed with " << VertexShaderErrorMessage.data() << '\n';
    }

    if(GeometryShaderID) {
      std::cout << "building geometry shader...\n";
      char const* GeometrySourcePointer = GeometryShaderCode.c_str();
      glShaderSource(GeometryShaderID, 1, &GeometrySourcePointer, NULL);
      glCompileShader(GeometryShaderID);

      glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
      glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
      if (InfoLogLength > 0) {
        std::vector<char> GeometryShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(GeometryShaderID, InfoLogLength, NULL, &GeometryShaderErrorMessage[0]);
        std::cerr << "building geometry shader failed with " << GeometryShaderErrorMessage.data() << '\n';
      }
    }

    std::cout << "building fragment shader...\n";
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
      std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
      glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
      std::cerr << "building fragment shader failed with " << FragmentShaderErrorMessage.data() << '\n';
    }

    std::cout << "building shader program...\n";
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    if (GeometryShaderID)
      glAttachShader(ProgramID, GeometryShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
      std::vector<char> ProgramErrorMessage(InfoLogLength+1);
      glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
      std::cerr << "building shader program failed with " << ProgramErrorMessage.data() << '\n';
    }

    glDeleteShader(VertexShaderID);
    glDeleteShader(GeometryShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
  renderer* ren = (renderer*) glfwGetWindowUserPointer(window);
  if (ren->nocallbacks)
    return;

  if(ren->mouse_state == renderer::e_mouse_state::PAN) {
    int win_geo[2];
    glfwGetWindowSize(ren->win, win_geo, &win_geo[1]);
    float winw = static_cast<float>(win_geo[0]);
    float winh = static_cast<float>(win_geo[1]);

    vec3f cam_y = ren->cam_up;
    normalize(cam_y);
    vec3f cam_z = ren->cam_pos - ren->fp_pos;
    float factor = ren->parallel == false ? std::sqrt(cam_z * cam_z) : 1.f / ren->scale;
    normalize(cam_z);
    vec3f cam_x = cross_prod(cam_y, cam_z);
    ren->cam_pos = ren->cam_pos + (cam_y * (ypos-ren->mouse_pos[1]) - cam_x * (xpos - ren->mouse_pos[0])) / winw*2.f * factor;
    ren->fp_pos = ren->fp_pos + (cam_y * (ypos-ren->mouse_pos[1]) - cam_x * (xpos - ren->mouse_pos[0])) / winw*2.f * factor;
  }

  if(ren->mouse_state == renderer::e_mouse_state::ROTATE) {
    vec3f cam_y = ren->cam_up;
    normalize(cam_y);
    vec3f cam_z = ren->cam_pos - ren->fp_pos;
    normalize(cam_z);
    vec3f cam_x = cross_prod(cam_y, cam_z);
    vec3f cam_pos_rel = ren->cam_pos - ren->fp_pos;

    cam_pos_rel = vec3f{cam_pos_rel * cam_x, cam_pos_rel * cam_y, cam_pos_rel * cam_z};
    
    float phi = static_cast<float>(-(xpos-ren->mouse_pos[0])/113.);
    float theta = static_cast<float>((ypos-ren->mouse_pos[1])/113.);

    vec3f cam_up_rel{-std::sin(phi) * std::sin(theta), std::cos(theta), -std::cos(phi) * std::sin(theta)};
    ren->cam_up = cam_up_rel[0] * cam_x + cam_up_rel[1] * cam_y + cam_up_rel[2] * cam_z;
    normalize(ren->cam_up);

    cam_pos_rel = vec3f{cam_pos_rel[2] * std::sin(phi) * std::cos(theta), cam_pos_rel[2] * std::sin(theta), cam_pos_rel[2] * std::cos(phi) * std::cos(theta)};
    
    ren->cam_pos = cam_pos_rel[0] * cam_x + cam_pos_rel[1] * cam_y + cam_pos_rel[2] * cam_z;
    ren->cam_pos = cross_prod(ren->cam_up, cross_prod(ren->cam_pos, ren->cam_up));
    ren->cam_pos = ren->cam_pos + ren->fp_pos;

  }
  
  ren->mouse_pos[0] = xpos;
  ren->mouse_pos[1] = ypos;
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)  {
  renderer* ren = (renderer*) glfwGetWindowUserPointer(window);
  if (ren->nocallbacks)
    return;

  vec3f distance = ren->cam_pos - ren->fp_pos;
  
  GLfloat factor = std::pow(1.1, -yoffset);

  distance = distance * factor;
  
  ren->cam_pos = distance + ren->fp_pos;

}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  renderer* r = (renderer*)glfwGetWindowUserPointer(window);
  if (r->nocallbacks)
    return;
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    r->mouse_state = renderer::e_mouse_state::ROTATE;

  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    r->mouse_state = renderer::e_mouse_state::PAN;

  if (action == GLFW_RELEASE)
    r->mouse_state = renderer::e_mouse_state::HOVER;

}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  renderer* r = (renderer*)glfwGetWindowUserPointer(window);
  glViewport(0, 0, width, height);
}

bool renderer::init(GLFWwindow* win_, object* obj_) {
  obj = obj_;
  GLint GLM{};
  GLint GLm{};
  glGetIntegerv(GL_MAJOR_VERSION, &GLM);
  glGetIntegerv(GL_MINOR_VERSION, &GLm);
  std::cout << "GL:" << GLM << '.' << GLm << '\n';

  win = win_;

  auto glew_err = glewInit();
  if (glew_err != GLEW_OK) {
    std::cerr << "renderer::init: ERROR: failed init glew with error " << glewGetErrorString(glew_err) << '\n';
    return false;
  }

  //make identity matrices
  for(int i=0; i<4; i++) {
    for(int j=0; j<4; j++) {
      view_matrix[i * 4 + j] = i == j ? 1.f : 0.f;
      proj_matrix[i * 4 + j] = i == j ? 1.f : 0.f;
    }
  }

  return true;
}

void renderer::set_callbacks(GLFWwindow* window) {
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}

static void iterate_collect_items(std::vector<OGLitem*>& items, const object* obj) {
  if(!obj->visible)
    return;
  for (auto& child : obj->children)
    iterate_collect_items(items, child);
  if(obj->item && obj->item->visible)
    items.push_back(obj->item);
}

std::vector<OGLitem*> renderer::get_items() {
  std::vector<OGLitem*> items;
  if(!obj)
    return items;
  iterate_collect_items(items, obj);
  return items;
}

void renderer::render() {
  int win_geo[2];
  glfwGetWindowSize(win, win_geo, &win_geo[1]);

  vec3f cam_y = cam_up;
  normalize(cam_y);
  vec3f cam_z = cam_pos - fp_pos;
  normalize(cam_z);
  vec3f cam_x = cross_prod(cam_y, cam_z);

  //x_new = (p - cam_pos) * cam_x;
  //y_new = (p - cam_pos) * cam_y;
  //z_new = (p - cam_pos) * cam_z;
  
  vec3f shift = vec3f{cam_pos * cam_x, cam_pos * cam_y, cam_pos * cam_z};
  
  for(int i=0;i<3;i++) {
    view_matrix[i*4] = cam_x[i];
    view_matrix[i*4+1] = cam_y[i];
    view_matrix[i*4+2] = cam_z[i];
  }
  for(int i=0;i<3;i++)
    view_matrix[3*4+i] = - shift[i];

  light_dir = cam_z + cam_x * 0.23 + cam_y * 0.33;
  normalize(light_dir);
 
  GLfloat f = FLT_MAX;
  GLfloat n = -FLT_MAX;
  
  if(!obj)
    return;
  auto items = get_items();


  for(auto& item : items) {
    auto item_bounds = item->get_bounds();
    vec3f box = item_bounds[1] - item_bounds[0];
    item_bounds[0] = item_bounds[0] - box * .01f;
    item_bounds[1] = item_bounds[1] + box * .01f;
    for(int i=0; i<8; i++) {
      vec3f outline_vertex;
      for(int c=0; c<3; c++)
        outline_vertex[c] = item_bounds[(i>>c) & 1][c];
      GLfloat proj = static_cast<GLfloat>((outline_vertex - cam_pos) * cam_z);
      f = f < proj ? f : proj;
      n = n > proj ? n : proj;
    }
  }

  if(parallel)
    scale = 4.f * std::tan(56.f / 2.f * 3.14f / 180.f) / std::sqrt((cam_pos - fp_pos)*(cam_pos - fp_pos));
  else
    scale = 1.f/std::tan(56.f / 2.f * 3.14f / 180.f); //https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix

  
  /* 
   *  0  1  2  3
   *  4  5  6  7
   *  8  9 10 11
   * 12 13 14 15
   * they multiply vector by COLUMN of matrix! yes, they do!
   */
  
  if(parallel) {
    proj_matrix[0] = scale;
    proj_matrix[5] = scale/static_cast<GLfloat>(win_geo[1]) * static_cast<GLfloat>(win_geo[0]);
    proj_matrix[10] = 2/(f-n);
    proj_matrix[14] = -(f+n)/(f-n);
    proj_matrix[11] = .0f;
    proj_matrix[15] = 1.f;
  } else { // perspective
    proj_matrix[0] = scale;
    proj_matrix[5] = scale/static_cast<GLfloat>(win_geo[1]) * static_cast<GLfloat>(win_geo[0]);
    proj_matrix[10] = -f/(f-n);
    proj_matrix[11] = -1.f;
    proj_matrix[14] = f*n/(f-n);
    proj_matrix[15] = 0.f;
  }

  GLfloat light[3]{static_cast<GLfloat>(light_dir[0]), static_cast<GLfloat>(light_dir[1]), static_cast<GLfloat>(light_dir[2])};

  for(auto& item : items)
    item->draw(view_matrix.data(), proj_matrix.data(), light);
}

GLuint renderer::getShader(const std::string& vs_name, const std::string& fs_name, const std::string& gs_name) {
  GLuint shader;
  auto it = shaders.find(std::array<std::string,3>{vs_name, fs_name, gs_name});
  if(it == shaders.end()) {
    std::stringstream vs;
    std::stringstream gs;
    std::stringstream fs;
    std::ifstream ivs(vs_name.c_str());
    vs << ivs.rdbuf();
    ivs.close();
    if(!gs_name.empty()) {
      std::ifstream igs(gs_name.c_str());
      gs << igs.rdbuf();
      igs.close();
    }
    std::ifstream ifs(fs_name);
    fs << ifs.rdbuf();
    ifs.close();

    shader = loadShaders(vs.str(), fs.str(), gs.str());
    if(!shader) {
      std::cerr << "load shaders failed\n";
      return false;
    }
    shaders[std::array<std::string,3>{vs_name, fs_name, gs_name}] = shader;
    return shader;
  } else
    return it->second;
}


vec3f renderer::center_camera() {
  vec3f geo_d{ .0, .0, .0 };
  fp_pos = vec3f{ .0, .0, .0 };
  auto items = get_items();

  for (auto& item : items) {
    if (!item->visible)
      continue;
    auto item_bounds = item->get_bounds();

    for (int c = 0; c < 3; c++) {
      double d = item_bounds[1][c] - item_bounds[0][c];
      geo_d[c] = geo_d[c] > d ? geo_d[c] : d;
    }
    fp_pos = fp_pos + (item_bounds[1] + item_bounds[0]) * .5;
  }
  fp_pos = fp_pos / static_cast<double>(items.size());
  return geo_d;
}

void renderer::reset_camera() {
  vec3f geo_d = center_camera();

  cam_pos = fp_pos + vec3f{0.f, 0.f, 3*geo_d[2]};

  cam_up = vec3f{0.f,1.f,0.f};

}
