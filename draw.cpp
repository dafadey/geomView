#include "draw.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <algorithm>

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

static void getCamCoords(vec3f& cam_x, vec3f& cam_y, vec3f& cam_z, float& factor, const renderer* ren) {
  cam_y = ren->cam_up;
  normalize(cam_y);
  cam_z = ren->cam_pos - ren->fp_pos;
  factor = ren->parallel == false ? std::sqrt(cam_z * cam_z) : 1.f / ren->scale;
  normalize(cam_z);
  cam_x = cross_prod(cam_y, cam_z);
}

static void getWinGeo(float& winw, float& winh, const renderer* ren) // pixels
{
  int win_geo[2];
  glfwGetWindowSize(ren->win, win_geo, &win_geo[1]);
  winw = static_cast<float>(win_geo[0]);
  winh = static_cast<float>(win_geo[1]);
} 

vec3f getD(GLFWwindow* window, const renderer* r) {

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    float winw, winh;
    getWinGeo(winw, winh, r);
    vec3f cam_x, cam_y, cam_z;
    float factor;
    getCamCoords(cam_x, cam_y, cam_z, factor, r);
    
    return cam_x * (xpos - r->xPressPos) / winw * 2.f * factor - cam_y * (ypos - r->yPressPos) / winw * 2.f * factor;
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
  renderer* ren = (renderer*) glfwGetWindowUserPointer(window);
  if (ren->nocallbacks)
    return;

  if(ren->mouse_state == renderer::e_mouse_state::PAN) {
    float winw, winh;
    getWinGeo(winw, winh, ren);
    vec3f cam_x, cam_y, cam_z;
    float factor;
    getCamCoords(cam_x, cam_y, cam_z, factor, ren);

    ren->cam_pos = ren->cam_pos + (cam_y * (ypos-ren->mouse_pos[1]) - cam_x * (xpos - ren->mouse_pos[0])) / winw * 2.f * factor;
    ren->fp_pos = ren->fp_pos + (cam_y * (ypos-ren->mouse_pos[1]) - cam_x * (xpos - ren->mouse_pos[0])) / winw * 2.f * factor;
  }

  if(ren->mouse_state == renderer::e_mouse_state::ROTATE) {
    vec3f cam_x, cam_y, cam_z;
    float factor;
    getCamCoords(cam_x, cam_y, cam_z, factor, ren);

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

  if(ren->mouse_state == renderer::e_mouse_state::SELECT_DRAG) {
    vec3f d = getD(window, ren);
    const object* obj = std::get<0>(ren->selectionResults.back());
    size_t item_id = std::get<2>(ren->selectionResults.back());
    for(int i=0; i<3; i++)
      obj->item->VBOdata[item_id * obj->item->VBOstride() + i] = ren->selectedPointPos0[i] + d[i];
    obj->item->VBOdata[item_id * obj->item->VBOstride() + 3] = ren->selectedPointRadius * 1.3f;
    obj->item->vboCopied=false;
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

static void select_object(const object* obj, const renderer* ren, std::vector<std::tuple<const object*, GLfloat, size_t>>& res, bool onlyControlPoints) {
  int win_geo[2];
  glfwGetWindowSize(ren->win, win_geo, &win_geo[1]);
  vec2f pos{(GLfloat) ren->mouse_pos[0] / (GLfloat) win_geo[0], -(GLfloat) ren->mouse_pos[1] / (GLfloat) win_geo[1]};
  pos = 2.f * (pos - vec2f{.5f, -.5f});
  //std::cout << "pos=(" << pos[0] << ", " << pos[1] << ")\n";
  for(auto& co : obj->children()) {
    if(co->visible)
      select_object(co, ren, res, onlyControlPoints);
  }
  if(!obj->item)
    return;
  if(onlyControlPoints && !obj->item->isControlPoints())
    return;
  GLfloat minDist2 = 13.0f;
  size_t id = 0;
  size_t minId=-1;
  for(size_t i=0; i<obj->item->VBOdata.size(); i+=obj->item->VBOstride())
  {
    vec3f pt{obj->item->VBOdata[i], obj->item->VBOdata[i+1], obj->item->VBOdata[i+2]};
    //std::cout << "pt=(" << pt[0] << ", " << pt[1] << ", " << pt[2] << ")\n";
    vec2f p = ren->project(pt);
    //std::cout << "p=(" << p[0] << ", " << p[1] << ")\n";
    GLfloat dist2 = (p - pos) * (p - pos);
    if(dist2 < minDist2) {
      minDist2 = dist2;
      minId = id;
    }
    id++;
  }
  res.push_back(std::make_tuple(obj,minDist2,minId));
  //std::cout << "obj: " << obj->name << " minDist2=" << minDist2 << '\n';
}

vec2f renderer::project(const vec3f& pt) const {
  vec4f in{pt[0], pt[1], pt[2], 1.f};
  vec4f v{.0f, .0f, .0f, .0f};
  for(int j = 0; j < 4; j++) {
    v[j] = .0f;
    for(int i = 0; i < 4; i++)
      v[j] += in[i] * view_matrix[i * 4 + j];
  }

  vec4f o=v;
  for(int j = 0; j < 4; j++) {
    o[j] = .0f;
    for(int i = 0; i < 4; i++) {
      o[j] += v[i] * proj_matrix[i * 4 + j];
    }
  }
  o[0] /= o[3];
  o[1] /= o[3];
  o[3] /= o[3];
  return vec2f{o[0], o[1]};
}

void renderer::select(bool onlyControlPoints) const {
  std::cout << "select\n";
  selectionResults.clear();
  select_object(obj, this, selectionResults, onlyControlPoints);
  std::sort(selectionResults.begin(), selectionResults.end(), [](const std::tuple<const object*, GLfloat, size_t>& a, const std::tuple<const object*, GLfloat, size_t>& b) {return std::get<1>(a) > std::get<1>(b);});
  for(const auto& it : selectionResults) {
    auto fullname = std::get<0>(it)->fullName();
    auto itt = fullname.end();
    itt--;
    for(; itt != fullname.begin(); itt--)
      std::cout << *itt << ':';
    std::cout << *itt << ':' << std::get<2>(it);
    
    std::cout << " : " << std::get<1>(it) << '\n';
  }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  renderer* r = (renderer*)glfwGetWindowUserPointer(window);
  
  if (r->nocallbacks)
    return;
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    r->mouse_state = renderer::e_mouse_state::ROTATE;

  if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    r->mouse_state = renderer::e_mouse_state::PAN;

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    r->mouse_state = renderer::e_mouse_state::SELECT_DRAG;
    r->select(true);
    if(r->selectionResults.size()) {
      const object* obj = std::get<0>(r->selectionResults.back());
      size_t item_id = std::get<2>(r->selectionResults.back());
      r->selectedPointRadius = obj->item->VBOdata[item_id * obj->item->VBOstride() + 3];
      obj->item->VBOdata[item_id * obj->item->VBOstride() + 3] = r->selectedPointRadius * 1.5f;
      obj->item->vboCopied = false;
      for(int i=0; i<3; i++)
        r->selectedPointPos0[i] = obj->item->VBOdata[item_id * obj->item->VBOstride() + i];
      glfwGetCursorPos(window, &(r->xPressPos), &(r->yPressPos));
    }
  }

  if (action == GLFW_RELEASE) {
    if(r->mouse_state == renderer::e_mouse_state::SELECT_DRAG) {
      if(r->selectionResults.size()) {
        const object* obj = std::get<0>(r->selectionResults.back());
        size_t item_id = std::get<2>(r->selectionResults.back());
        vec3f d = getD(window, r);
        for(int i=0; i<3; i++)
          obj->item->VBOdata[item_id * obj->item->VBOstride() + i] = r->selectedPointPos0[i]+d[i];
        obj->item->VBOdata[item_id * obj->item->VBOstride() + 3] = r->selectedPointRadius;
        obj->item->vboCopied=false;
        if(r->controlPointMoved) {
          auto sId = obj->fullName();
          sId.push_back(std::to_string(item_id));
          (*r->controlPointMoved)(r->callbackData, sId, r->selectedPointPos0[0]+d[0], r->selectedPointPos0[1]+d[1], r->selectedPointPos0[2]+d[2]);
        }
      }
    }
    r->mouse_state = renderer::e_mouse_state::HOVER;
  }
  
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    r->select();
  }
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

static void uset_callbacks(GLFWwindow* window) {
  glfwSetCursorPosCallback(window, nullptr);
  glfwSetMouseButtonCallback(window, nullptr);
  glfwSetScrollCallback(window, nullptr);
  glfwSetFramebufferSizeCallback(window, nullptr);
  glfwSetWindowCloseCallback(window, nullptr);
}

static void window_close_callback(GLFWwindow* window) {
  renderer* r = (renderer*)glfwGetWindowUserPointer(window);
  r->win = nullptr;
  uset_callbacks(window);
}

void renderer::set_callbacks(GLFWwindow* window) {
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetWindowCloseCallback(window, window_close_callback);
}

static void iterate_collect_items(std::vector<OGLitem*>& items, const object* obj) {
  if(!obj->visible)
    return;
  for (auto& child : obj->children())
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
  std::cout << "renderer::render\n";
  if(!win)
    return;
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
      if(!std::isnan(proj) && !std::isinf(proj)) {
        f = f < proj ? f : proj;
        n = n > proj ? n : proj;
      }
    }
  }
  if(fabs(f-n) < 1e-3 * fabs(f)) {
    f+=1e-3*f;
    n-=1e-3*n;
  }
  
  //std::cout << "f=" << f << ", n=" << n << ", cam_pos=" << cam_pos << ", cam_z=" << cam_z << '\n';
  
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
    proj_matrix[10] = 2./(f-n);
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
  /*
  GLfloat nearfarVal[2];
  glGetFloatv(GL_DEPTH_RANGE, nearfarVal);
  std::cout << "near=" << nearfarVal[0] << ", far=" << nearfarVal[1] << '\n';
  std::cout << "n=" << n << ", f=" << f << '\n';
  */
  GLfloat light[3]{static_cast<GLfloat>(light_dir[0]), static_cast<GLfloat>(light_dir[1]), static_cast<GLfloat>(light_dir[2])};

  for(auto& item : items) {
    item->draw(view_matrix.data(), proj_matrix.data(), light);
  }
}

GLuint renderer::getShader(const std::string& vs_name, const std::string& fs_name, const std::string& gs_name) {
  #include "shaderRAMfs.cpp.inl"
  GLuint shader;
  auto it = shaders.find(std::array<std::string,3>{vs_name, fs_name, gs_name});
  if(it == shaders.end()) {
    std::stringstream vs;
    std::stringstream gs;
    std::stringstream fs;
    
    auto readFromSomewhere=[](std::stringstream& ss, const std::string& f_name) {
      if(f_name.empty())
        return;
      auto itt = shaderRAMfs.find(f_name);
      if(itt != shaderRAMfs.end())
        ss << itt->second;
      else {
        std::ifstream inf(f_name.c_str());
        ss << inf.rdbuf();
        inf.close();
      }
    };
    
    readFromSomewhere(gs, gs_name);
    readFromSomewhere(vs, vs_name);
    readFromSomewhere(fs, fs_name);

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

  cam_pos = fp_pos + vec3f{0.f, 0.f, sqrtf(geo_d*geo_d)};

  cam_up = vec3f{0.f,1.f,0.f};

}
