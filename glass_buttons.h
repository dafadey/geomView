#pragma once
#include <vector>
#include <GL/glew.h>

struct glass_button {
  enum eaction {DEFAULT=0, PRESSED=1};
  glass_button(int Id, int x0, int y0, int W, int H, int tx0, int ty0, int tx1, int ty1, void (*clbk)(int buttonID, eaction act, void* dat), void* dat);
  //coordinates of button in texture
  int texX0{};
  int texY0{};
  int texX1{};
  int texY1{};
  //onscreen position in pixels (bottom left corner)
  int X0{};
  int Y0{};
  //button dimension
  int w{};
  int h{};
  //state:
  int id{};
  eaction action{eaction::DEFAULT};
  //callback
  void (*callback)(int buttonID, eaction act, void* data) {};
  //data
  void* data{};
  
  bool inside(int x, int y) const;
};

struct GLFWwindow;

struct glass_buttons : public std::vector<glass_button> {
  glass_buttons(GLFWwindow*);
  ~glass_buttons();
  GLuint tex{}; //texture may be shared between different buttons
  GLuint bgtex{};
  GLuint vao{};
  GLuint vbo{};
  GLuint shader{};
  GLuint texDim[2];
  int tex_nx, tex_ny;
  #define DOLOC(X) int X##_loc{};
  DOLOC(fbDim)
  DOLOC(mousePos)
  DOLOC(pressed)
  DOLOC(vert)
  DOLOC(texcoords)
  DOLOC(btn_center)
  DOLOC(btn_sz)
  DOLOC(texDim)
  DOLOC(btnTex)
  DOLOC(bgTex)
  DOLOC(flex)
  #undef DOLOC  
  GLFWwindow* window{};
  int anyMouseButtonPushed = false; // set for any button to store previous state
  void process();
  void init(const char* tex_data, int tex_size);
  bool wantsToGrabCursor();
  glass_button* active_button{};
  void addButton(const glass_button&);
  void draw();
  void finalize(); //call at the end of frame
  mutable bool update{true};
};

struct grayImage : public std::vector<unsigned char> {
  grayImage() = default;
  grayImage(int nx_, int ny_);
  int nx, ny;
};

grayImage load_grayscale_png(const char* data, int size);
