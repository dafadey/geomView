#include "shaders.h"
#include "draw.h"
#include "interface.h"
#include "imgui_controls.h"
#include "object.h"
#include "fb2way.h"
#include "glass_buttons.h"

#define CODETOCHARv150(X) "#version 150\n" #X

const char* sha_fb_vs = "#version 150\n"
            "out vec2 fbCoord;\n"
            "void main() {\n"
            "    const vec2 corners[6] = vec2[] (vec2(0,0), vec2(1,0), vec2(1,1), vec2(0,0), vec2(1,1), vec2(0,1));\n"
            "    fbCoord = corners[gl_VertexID];\n"
            "    gl_Position = 2. * vec4(fbCoord, .0, .5) + vec4(-1., -1., .0, .0);\n"
            "}\n";

const char* sha_fb_fs = (const char*) CODETOCHARv150
(
            out vec4 outputF;
            in vec2 fbCoord;
            uniform sampler2D fbTexture;
            uniform sampler2D fbhlTexture;
            uniform int fb_wx;
            uniform int fb_wy;
            void main() {
              /*
              //funny shader
              vec2 rv = fbCoord-vec2(.5,.5);
              float r = rv.x*rv.x + rv.y*rv.y;
              r *= 0.03;
              float n=.0;
              outputF = vec4(0,0,0,0);
              for(float y=-r;y<r;y+=0.0013)
              {
                for(float x=-r;x<r;x+=0.0013)
                {
                  float f = .01/(0.01 + x*x+y*y);
                  n += f;
                  outputF += texture(fbTexture, fbCoord + vec2(x,y)) * f;
                }
              }
              outputF = outputF * (1./n);
              */

              ivec2 coord = ivec2(fbCoord.x * fb_wx, fbCoord.y * fb_wy);

              vec4 this_color = texelFetch(fbhlTexture, coord, 0);

              int r0 = 3;
              int nearest0 = 2*r0*r0;
              int nearest = nearest0;
              for(int j = -r0; j <= r0; j++) {
                for(int i = -r0; i <= r0; i++) {
                  ivec2 ipos = coord + ivec2(i, j);
                  if(ipos.x<0 || ipos.y<0 || ipos.x >= fb_wx || ipos.y >= fb_wy)
                    continue;
                  vec4 c = texelFetch(fbhlTexture, ipos, 0);
                  int r2 = i*i+j*j;
                  if(c != this_color && r2 < nearest)
                    nearest = r2;
                }
              }
              
              float f = sqrt(float(nearest) / float(nearest0));
              
              if(nearest == nearest0)
                outputF = texelFetch(fbTexture, coord, 0);
              else
                outputF = (1.f - f) * vec4(1,1,0,1) + f * texelFetch(fbTexture, coord, 0);

            }
);

void fb2way::init(int wx, int wy) {
  if(!fb_texture) {
    glGenTextures(1, &fb_texture);
    glBindTexture(GL_TEXTURE_2D, fb_texture);
    fb_wx = wx;
    fb_wy = wy;
    if(wx && wy)
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fb_wx, fb_wy, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
  }

  if(!fbhl_texture) {
    glGenTextures(1, &fbhl_texture);
    glBindTexture(GL_TEXTURE_2D, fbhl_texture);
    fb_wx = wx;
    fb_wy = wy;
    if(wx && wy)
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fb_wx, fb_wy, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
  }

  
  if(!fb_zs_texture) {
    glGenTextures(1, &fb_zs_texture);
    glBindTexture(GL_TEXTURE_2D, fb_zs_texture);
    fb_wx = wx;
    fb_wy = wy;
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, wx, wy, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    if(wx && wy)
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, wx, wy, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  }
  
  if(!fb) {
    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_texture, 0);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, fb_zs_texture, 0); 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb_zs_texture, 0); 
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "failed to bind frame buffer\n";
  }

  if(!fbhl) {
    glGenFramebuffers(1, &fbhl);
    glBindFramebuffer(GL_FRAMEBUFFER, fbhl);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbhl_texture, 0);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, fb_zs_texture, 0); 
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb_zs_texture, 0); 
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "failed to bind frame buffer\n";
  }
  
  if(!fb_shader)
    fb_shader = loadShaders(sha_fb_vs, sha_fb_fs);

  fbtexloc = glGetUniformLocation(fb_shader, "fbTexture");
  fbhltexloc = glGetUniformLocation(fb_shader, "fbhlTexture");
  //fbztexloc = glGetUniformLocation(fb_shader, "fbzTexture");
  //std::cout << "fbtexloc=" << fbtexloc << ", fbztexloc=" << fbztexloc << '\n';
  fb_wx_loc = glGetUniformLocation(fb_shader, "fb_wx");
  fb_wy_loc = glGetUniformLocation(fb_shader, "fb_wy");

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void fb2way::resize(int wx, int wy) {
  if(!fb_texture)
    return;
  if(fb_wx == wx && fb_wy == wy)
    return;
  fb_wx = wx;
  fb_wy = wy;
  glBindTexture(GL_TEXTURE_2D, fb_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fb_wx, fb_wy, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindTexture(GL_TEXTURE_2D, fbhl_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fb_wx, fb_wy, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindTexture(GL_TEXTURE_2D, fb_zs_texture);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, wx, wy, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, wx, wy, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

}

void fb2way::getBuffer(std::vector<unsigned char>& buff) {
  int stride = 3 * fb_wx;
  int add = stride % 4;
  stride += (4 - add)%4;
  if(buffer.size() != stride * fb_wy)
    buffer.resize(stride * fb_wy);
  glBindTexture(GL_TEXTURE_2D, fb_texture);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
  if(buff.size() != 3 * fb_wx * fb_wy)
    buff.resize(3 * fb_wx * fb_wy);
  
  for(int j=0; j < fb_wy; j++) {
    for(int i=0; i < fb_wx; i++) {
      for(int c=0; c < 3; c++)
        buff[(i + j*fb_wx)*3 + c] = buffer[i*3 + j*stride + c];
    }
  }
}

void fb2way::set_default() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void fb2way::set_custom() {
  glBindFramebuffer(GL_FRAMEBUFFER, fb);
}

void fb2way::set_custom_hl() {
  glBindFramebuffer(GL_FRAMEBUFFER, fbhl);
}

void fb2way::render() {
  if(!vao)
    glGenVertexArrays(1, &vao);
  glUseProgram(fb_shader);
  glBindVertexArray(vao);
  glDisable(GL_DEPTH_TEST);
  glUniform1i(fbtexloc, 0);
  //glUniform1i(fbztexloc, 1);
  glUniform1i(fbhltexloc, 2);
  glUniform1i(fb_wx_loc, fb_wx);
  glUniform1i(fb_wy_loc, fb_wy);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, fb_texture);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, fbhl_texture);
  //glActiveTexture(GL_TEXTURE1);
  //glBindTexture(GL_TEXTURE_2D, fb_zs_texture);
  //std::cout << "fb2way rendering texture " << fb_texture << '\n';
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

fb2way::~fb2way() {
  if(fb)
    glDeleteFramebuffers(1, &fb);
  if(fb_texture)
    glDeleteTextures(1, &fb_texture);
  if(fbhl_texture)
    glDeleteTextures(1, &fbhl_texture);
  if(fb_zs_texture)
    glDeleteTextures(1, &fb_zs_texture);
}

void mainloop_pipeline(geom_view* gv, fb2way* fb2, glass_buttons* btns, geom_view::UIappearance* appearance) {
    renderer* renptr = gv->ren_ptr;
    imgui_interface* iface = gv->iface;
    object* obj_root = gv->obj_root;
  
    GLFWwindow* window = iface->window;
    geom_view::UIappearance default_UIappearance;
    if(!appearance)
      appearance = &default_UIappearance;
    
    renderer& ren = *renptr;
    //std::cout << "gv->screenGeo:" << gv->screenGeo[0] << 'x' << gv->screenGeo[1] << '\n';
    if(!gv->isOffscreen()) {
      int wx, wy;
      glfwGetWindowSize(window, &wx, &wy);
      gv->screenGeo = std::array<int,2> {wx, wy};
    }
    
    if(fb2) {
      fb2->set_custom_hl();
      fb2->resize(gv->screenGeo[0], gv->screenGeo[1]);
      glClearColor(1, 1, 1, 1.);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glEnable(GL_DEPTH_TEST);
      ren.draw_mode = renderer::e_draw_mode::HIGHLIGHT;
      ren.render();
    }

    if(fb2) {
      fb2->set_custom();
      fb2->resize(gv->screenGeo[0], gv->screenGeo[1]);
    }

    if(!gv->isOffscreen()) {
      ren.nocallbacks = ImGui::GetIO().WantCaptureMouse;
      if(btns)
        ren.nocallbacks |= btns->wantsToGrabCursor();
        
      if(ren.nocallbacks)
        glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
      else
        ren.set_callbacks(window);
    }
    glClearColor(ren.bg_color[0], ren.bg_color[1], ren.bg_color[2], 1.);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ren.draw_mode = renderer::e_draw_mode::NORMAL;
    ren.render();

    if(fb2) {
      fb2->set_default();
      fb2->render();
    }
    
    if(gv->isOffscreen())
      return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    
    ImGui::NewFrame();

    iface->CustomControls();

    if(appearance->imgui_object_control)
      ImGui::ObjectsControl(obj_root, &ren);

    if(appearance->imgui_cam_control) {
      ImGui::Begin("camera");
      ImGui::CameraControl(&ren);
      ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::Text("Memory: %d Mb", obj_root->memory() /1024/1024);
      ImGui::End();
    }

    if(ren.need_bg_color_picker) {
      ImGui::Begin("pick background color");
      if(ImGui::Button("close"))
        ren.need_bg_color_picker=false;
      ImGui::ColorPicker3("bg color standalone", ren.bg_color.data());
      ImGui::End();
    }
    
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    
    if(appearance->native_cam_control)
    {
      if(btns) {
        btns->process();
        btns->draw();
        //std::cout << "active_button=" << btns->active_button << '\n';
        btns->finalize();
      }
    }
}
