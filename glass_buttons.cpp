#include <iostream>
#include "glass_buttons.h"
#include "shaders.h"
#include <GLFW/glfw3.h>
#include "grpng_reader.h"

#define CODETOCHARv150(X) "#version 150\n" #X

grayImage::grayImage(int nx_, int ny_) : nx(nx_), ny(ny_) { this->resize(nx*ny); }

const char* glass_btn_vs = (const char*) CODETOCHARv150
(
            in vec2 vert; // pixels
            in vec2 texcoords; //pixels
            in vec2 btn_center;
            in vec2 btn_sz;
            uniform int flex;
            uniform vec2 fbDim; // pixels
            uniform vec2 texDim; // pixels
            uniform vec2 mousePos; // viewport
            out vec2 texcoords_;
            out vec2 vert_;
            out float hover;
            vec3 func(float x, float d, float w) {
              vec3 res;
              if(abs(x) < d)
                res = vec3(x, 1., 1. - abs(x)/d);
              if(abs(x) >= d && abs(x) < d+w)
                res = vec3((x > .0 ? 1. : -1. ) * (d - (abs(x)-d)/w*d), 1., .0);
              if(abs(x) >= d+w)
                res = vec3(.0, .0, .0);
              return vec3(int(.17 * res.x), res.y, res.z);
            }
            void main() {
              texcoords_ = vec2(texcoords.x / texDim.x, texcoords.y / texDim.y);
              vec2 v = vert;
              hover = .0;
              vec2 delta = mousePos - btn_center;
              if(flex == 1)
              {
                const float r = 8.;
                int close = 0;
                vec3 dx = func(delta.x, btn_sz.x/2., r);
                vec3 dy = func(delta.y, btn_sz.y/2, r);
                hover = dx.z > .0 && dy.z > .0 ? dx.z * dx.z + dy.z * dy.z : hover;
                v += (dx.y == 1. && dy.y == 1. ? vec2(dx.x,dy.x) : vec2(.0, .0));
              } else
                hover = 2.*abs(delta.x) < btn_sz.x && 2.*abs(delta.y) < btn_sz.y ? 1 : 0;
              vec2 pos = vec2(v.x / fbDim.x * 2. - 1., 1. - v.y / fbDim.y * 2.);
              vert_ = .5 * (pos + vec2(1., 1.));
              gl_Position = vec4(pos, .0, 1.);
            }
);

const char* glass_btn_fs = (const char*) CODETOCHARv150
(
            uniform sampler2D btnTex;
            uniform sampler2D bgTex;
            uniform vec2 fbDim; // pixels
            in vec2 texcoords_;
            in vec2 vert_;
            in float hover;
            uniform int pressed;
            out vec4 outputF;
            void main() {
              const int r = 5;
              vec4 c = vec4(.0,.0,.0,.0);
              int n = 0;
              for(int j = -r; j <= r; j++) {
                for(int i = -r; i <= r; i++) {
                  c += texture(bgTex, vert_ + vec2(i/fbDim.x,j/fbDim.y));
                  n ++;
                }
              }
              c = c/n;
              float dens = (.7 + hover*.3) * (1. - texture(btnTex, texcoords_).r);
              outputF = (hover > .0 && pressed==1 ? .5 : .8) * c + .7 * dens * vec4(1,1,1,1);
            }
);

glass_button::glass_button(int Id, int x0, int y0, int W, int H, int tx0, int ty0, int tx1, int ty1, void (*clbk)(int buttonID, glass_button::eaction act, void* data) = nullptr, void* dat = nullptr) : id(Id), X0(x0), Y0(y0), w(W), h(H), texX0(tx0), texY0(ty0), texX1(tx1), texY1(ty1), callback(clbk), data(dat) {}

bool glass_button::inside(int x, int y) const {
  return (x-X0) * (X0+w - x) > 0 && (y-Y0) * (Y0+h-y) > 0;
}

static bool getAnyMouseButtonState(GLFWwindow* window) {
  bool anyMouseButtonPushed_ = false;
  anyMouseButtonPushed_ |= glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
  anyMouseButtonPushed_ |= glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
  anyMouseButtonPushed_ |= glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
  return anyMouseButtonPushed_;
}

bool glass_buttons::wantsToGrabCursor() {
  double x,y;
  glfwGetCursorPos(window, &x, &y);
  bool anyMouseButtonPushed_ = getAnyMouseButtonState(window);
  bool inside_ = false;
  for(auto& b : *this) {
    bool inside_b = b.inside(x,y);
    inside_ |= inside_b;
    b.action = inside_b ? b.action : glass_button::eaction::DEFAULT;
    active_button = inside_b ? &b : active_button;
  }
  bool wants = inside_ && (!anyMouseButtonPushed || (anyMouseButtonPushed && !anyMouseButtonPushed_));
  active_button = wants ? active_button : nullptr;
  //we want to grab focus only if we are inside and if previous state of mouse button was 'ALL RELEASED' or previous state was 'ANY PUSHED' but while being insde it changed to 'ALL RELEASED'.
  return wants;
}

//we want to process LMB release action
void glass_buttons::process() {
  if(!active_button || !(active_button->callback))
    return;
  int act = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
  glass_button::eaction new_action = act == GLFW_PRESS ? glass_button::eaction::PRESSED : glass_button::eaction::DEFAULT;
  if(act == GLFW_RELEASE && active_button->action == glass_button::eaction::PRESSED) // release event
    active_button->callback(active_button->id, new_action, active_button->data);
  active_button->action = new_action;
  //std::cout << "setting new action=" << new_action << '\n';
}

glass_buttons::glass_buttons(GLFWwindow* win) : window(win) {}

void glass_buttons::init(const char* tex_data, int tex_size) {
  if(!vao)
    glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  if(!vbo)
    glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  shader = loadShaders(glass_btn_vs, glass_btn_fs);

  #define DOLOC(X) X##_loc = glGetUniformLocation(shader, #X); //std::cout << #X << "_loc=" << X##_loc << '\n';
  DOLOC(btnTex)
  DOLOC(bgTex)
  DOLOC(fbDim)
  DOLOC(texDim)
  DOLOC(mousePos)
  DOLOC(flex)
  DOLOC(pressed)
  #undef DOLOC

  #define DOLOC(X) X##_loc = glGetAttribLocation(shader, #X); //std::cout << #X << "_loc=" << X##_loc << '\n';
  DOLOC(vert)
  DOLOC(texcoords)
  DOLOC(btn_center)
  DOLOC(btn_sz)
  #undef DOLOC
  
  grayImage teximage;

  if(!tex_data || tex_size==0) {
    teximage=grayImage(32*11, 32);
    for(unsigned int j=0;j<32;j++) {
      for(unsigned int i=0;i<32*11;i++)
        teximage[(j*32*11+i)] = (unsigned char) ((i/5 + j/5) % 2) * 196 + 16;
    }
  } else
    teximage = load_grayscale_png(tex_data, tex_size);
  
  tex_nx = teximage.nx;
  tex_ny = teximage.ny;
  
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tex_nx, tex_ny, 0, GL_RED, GL_UNSIGNED_BYTE, teximage.data());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
}

glass_buttons::~glass_buttons() {
  if(vbo)
    glDeleteBuffers(1, &vbo);
  if(vao)
    glDeleteVertexArrays(1, &vao);
}

void glass_buttons::addButton(const glass_button& btn) {
  this->push_back(btn);
  update = true;
}

void glass_buttons::draw() {
  if(update) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    const int stride = 2 * 3 * 8; // 2 * triangles(= 3 vertices (= 2 coordinates + 2 texcoordinates) + 2 center coordinates + 2 size xy)
    std::vector<float> vbodata(this->size() * stride);
    
    for(int id = 0; id < this->size(); id++) {
      size_t offset = id * stride;
      const glass_button& btn = (*this)[id];
      float centerx = btn.X0+btn.w/2;
      float centery = btn.Y0+btn.h/2;
      float szx = btn.w;
      float szy = btn.h;
      #define ADDCONSTANTS  vbodata[offset] = centerx;      offset++; \
                            vbodata[offset] = centery;      offset++; \
                            vbodata[offset] = szx;          offset++; \
                            vbodata[offset] = szy;          offset++;

      //first triangle
      vbodata[offset] = btn.X0;       offset++;
      vbodata[offset] = btn.Y0;       offset++;
      vbodata[offset] = btn.texX0;    offset++;
      vbodata[offset] = btn.texY0;    offset++;
      ADDCONSTANTS

      vbodata[offset] = btn.X0+btn.w; offset++;
      vbodata[offset] = btn.Y0;       offset++;
      vbodata[offset] = btn.texX1;    offset++;
      vbodata[offset] = btn.texY0;    offset++;
      ADDCONSTANTS

      vbodata[offset] = btn.X0+btn.w; offset++;
      vbodata[offset] = btn.Y0+btn.h; offset++;
      vbodata[offset] = btn.texX1;    offset++;
      vbodata[offset] = btn.texY1;    offset++;
      ADDCONSTANTS

      //second triangle
      vbodata[offset] = btn.X0;       offset++;
      vbodata[offset] = btn.Y0;       offset++;
      vbodata[offset] = btn.texX0;    offset++;
      vbodata[offset] = btn.texY0;    offset++;
      ADDCONSTANTS
      
      vbodata[offset] = btn.X0+btn.w; offset++;
      vbodata[offset] = btn.Y0+btn.h; offset++;
      vbodata[offset] = btn.texX1;    offset++;
      vbodata[offset] = btn.texY1;    offset++;
      ADDCONSTANTS
      
      vbodata[offset] = btn.X0;       offset++;
      vbodata[offset] = btn.Y0+btn.h; offset++;
      vbodata[offset] = btn.texX0;    offset++;
      vbodata[offset] = btn.texY1;    offset++;
      ADDCONSTANTS
      
      #undef ADDCONSTANTS
    }
    
    glBufferData(GL_ARRAY_BUFFER, vbodata.size() * sizeof(GLfloat), vbodata.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(vert_loc, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*) 0);
    glVertexAttribPointer(texcoords_loc, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(2 * sizeof(float)));
    glVertexAttribPointer(btn_center_loc, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(4 * sizeof(float)));
    glVertexAttribPointer(btn_sz_loc, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(vert_loc);
    glEnableVertexAttribArray(texcoords_loc);
    glEnableVertexAttribArray(btn_center_loc);
    glEnableVertexAttribArray(btn_sz_loc);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    update = false;
  }
  
  glDisable(GL_DEPTH_TEST);
  glUseProgram(shader);
  glBindVertexArray(vao);

  glUniform1i(btnTex_loc, 0);
  glUniform1i(bgTex_loc, 1);
  int wx,wy;
  glfwGetWindowSize(window, &wx, &wy);
  glUniform2f(fbDim_loc, (float) wx, (float) wy);

  glUniform2f(texDim_loc, (float) tex_nx, (float) tex_ny);

  glUniform1i(flex_loc, getAnyMouseButtonState(window) && anyMouseButtonPushed ? 0 : 1);

  //std::cout << "pressed=" << (active_button && active_button->action == glass_button::eaction::PRESSED ? 1 : 0) << '\n';
  glUniform1i(pressed_loc, active_button && active_button->action == glass_button::eaction::PRESSED ? 1 : 0);

  double cursor_x, cursor_y;
  glfwGetCursorPos(window, &cursor_x, &cursor_y);
  glUniform2f(mousePos_loc, (float) cursor_x, (float) cursor_y);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, bgtex);

  glDrawArrays(GL_TRIANGLES, 0, this->size() * 2 * 3);

  glBindTexture(GL_TEXTURE_2D, 0);

  glBindVertexArray(0);
  glUseProgram(0);
}

void glass_buttons::finalize() {
  anyMouseButtonPushed = getAnyMouseButtonState(window);
}

grayImage load_grayscale_png(const char* data, int size) {
  image* img = read_png_file(data, size);
  if(!img)
    return grayImage(0,0);
  grayImage gimg(img->w, img->h);
  gimg.nx = img->w;
  gimg.ny = img->h;
  for(int j=0; j<img->h; j++) {
    for(int i=0; i<img->w; i++)
      gimg[i+j*gimg.nx] = img->get(i,j).red * 255.0;
  }
  delete img;
  return gimg;
}
