#include <iostream>
#include <limits>
#include <thread>
#include <fstream>

#include "draw.h"
#include "interface.h"
#include "object.h"
#include "geom_view.h"
#include "imgui_controls.h"
#include "fb2way.h"
#include "glass_buttons.h"
#include "buttons.png.inl"
#include "grpng_reader.h"
#include "tools.h"

#ifdef _WIN32
#include <ole2.h>
#include <oleidl.h>

class DragAndDrop : public IDropTarget {
  HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) override;
  HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE DragLeave() override { return S_OK; }
  HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override { 
	  //std::cout << "!!! QueryInterface: REFIID=" << (void*) &riid << '\n';
	  return S_OK;
  }
  ULONG STDMETHODCALLTYPE AddRef() override {
	  //std::cout << "!!! AddRef\n"; 
	  return 1;
  }
  ULONG STDMETHODCALLTYPE Release() override {
	  //std::cout << "!!! Release\n"; 
	  return 0;
  }
public:
  object* obj_root = nullptr;
  renderer* ren = nullptr;
};

HRESULT DragAndDrop::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) {
	//std::cout << "!!! gotcha!\n";
	FORMATETC fmc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM res;
	if (pDataObj->GetData(&fmc, &res) != S_OK) {
		std::cout << "cannot get data from drop\n";
	    return S_FALSE;
	}
	HDROP hdrop = (HDROP) res.hGlobal;
	UINT file_count = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
	for (int i = 0; i < file_count; i++) {
  	  wchar_t file[MAX_PATH];
	  DragQueryFile(hdrop, i, file, MAX_PATH);
	  if (obj_root && ren)
	    load_objects(obj_root, SFW(file), ren);
	}

	if (file_count && ren)
	  ren->reset_camera();
	
	ReleaseStgMedium(&res);
	return S_OK;
}
#endif

const int geom_view::LEFT_BUTTON = GLFW_MOUSE_BUTTON_LEFT;
const int geom_view::MIDDLE_BUTTON = GLFW_MOUSE_BUTTON_MIDDLE;
const int geom_view::RIGHT_BUTTON = GLFW_MOUSE_BUTTON_RIGHT;

geom_view::geom_view() {
  viewControls.rotateButton = LEFT_BUTTON;
  viewControls.panButton = MIDDLE_BUTTON;
  viewControls.selectButton = RIGHT_BUTTON;
  
	iface = new imgui_interface;
}

geom_view::~geom_view() {
  if(obj_root)
    delete obj_root;
  obj_root = nullptr;
  if(ren_ptr)
    delete ren_ptr;
  ren_ptr = nullptr;
  if(fb2)
    delete fb2;
  fb2 = nullptr;
  
  iface->close();
  delete iface;
}

void geom_view::setOffscreen(int nx, int ny) {
  if(iface && iface->inited) {
    std::cerr << "ERROR setting ofsscreen mode, please set offscreen mode prior to initialization\n";
    return;
  }
  offscreen = true;
  screenGeo[0] = nx;
  screenGeo[1] = ny;
  runInThread = false;
}

bool geom_view::isOffscreen() {
  return offscreen;
}

void proc_xyz(int id, glass_button::eaction act, void* dat);

void geom_view::getBuffer(int& nx, int& ny, std::vector<unsigned char>& buff) {
  if(fb2) {
    fb2->getBuffer(buff);
    nx = fb2->fb_wx;
    ny = fb2->fb_wy;
  }
}

void geom_view::makeShot() {
  if(!offscreen) {
    std::cerr << "ERROR making of a shot, run geomView in offscreen mode with setOffscreen() before initialization\n";
    return;
  }
  if(!fb2) {
    std::cerr << "ERROR making of a shot, initialize geomView with init(...) prior to making a shot\n";
    return;
  }
  mainloop_pipeline(this, fb2, nullptr, nullptr);
}

void geom_view::thread_func(geom_view* gv) {
  gv->th_id = std::this_thread::get_id();
  imgui_interface& iface = *(gv->iface);
  iface.init(gv->offscreen);
  
  gv->obj_root = new object;
  gv->obj_root->name = "root";
  gv->ren_ptr = new renderer(gv->viewControls);
  gv->fb2 = new fb2way;
  
  renderer& ren = *gv->ren_ptr;
  ren.outputGeo_ptr = &gv->screenGeo;
  ren.init(gv->obj_root);
  
  gv->reloadLock.lock();
  for(const auto& file : gv->files) {
    if(!file.first.empty())
      load_objects(gv->obj_root, file.first.c_str(), &ren);
  }
  gv->files.clear();
  gv->reloadLock.unlock();

  ren.reset_camera();

  if(!gv->isOffscreen())
    glfwSetWindowUserPointer(iface.window, (void*) &ren);
  
  gv->windowCreationCV.notify_all(); // unlock main thread
  
  gv->fb2->init(1, 1);

  if(gv->isOffscreen())
    return;
    
  glass_buttons btns(iface.window);
  btns.init(buttons_png, buttons_png_len);
  btns.bgtex = gv->fb2->fb_texture;
  
  for(int i=0;i<13;i++)
    btns.addButton(glass_button(i, 10+(10+32)*i, 10, 32, 32, 32*i, 0, 32*(i+1), 32, proc_xyz, gv));

  #ifdef _WIN32
  if (OleInitialize(NULL) != S_OK)
    std::cout << "failed to initialize COM\n";
  DragAndDrop dragAndDropTarget;
  dragAndDropTarget.obj_root = gv->obj_root;
  dragAndDropTarget.ren = gv->ren_ptr;
  HWND hWnd = gv->iface->nativeMSWindowHandler;
  auto RegDADres = RegisterDragDrop(hWnd, &dragAndDropTarget);
  if (RegDADres != S_OK)
	std::cout << "register drag and drop failed: " << RegDADres << '\n';
  #endif

  while (!glfwWindowShouldClose(iface.window))  {
    glfwWaitEvents();
    gv->reloadLock.lock();
    if(gv->reloadFlag) {
      std::cout << "files size is " << gv->files.size() << '\n';
      gv->reloadFlag = false;
      reload_files(gv->obj_root, gv->ren_ptr, gv->files);
      gv->files.clear();
    }
    if(gv->changeVisibilityFlag) {
      gv->changeVisibilityFlag = false;
      changeVisibility_for_files(gv->obj_root, gv->ren_ptr, gv->files);
      gv->files.clear();
    }
    if(gv->resetCameraRequest) {
      gv->resetCameraRequest = false;
      gv->ren_ptr->reset_camera();
    }
    gv->reloadLock.unlock();

    mainloop_pipeline(gv, gv->fb2, &btns, &gv->appearance);

    glfwSwapBuffers(iface.window);

    glFlush();
  }
  
  #ifdef _WIN32
  RevokeDragDrop(hWnd);
  OleUninitialize();
  #endif

  delete gv->obj_root;
  gv->obj_root = nullptr;
  delete gv->ren_ptr;
  gv->ren_ptr = nullptr;
  delete gv->fb2;
  gv->fb2 = nullptr;
  
  iface.close();
  gv->windowCreationCV.notify_all(); // unlock main thread, it should be waiting for mainloop to finish by should close flag

  std::cout << "geom view thread finishes\n";
}

void geom_view::init() {
  if(!iface || iface->window)
    return;
  if(runInThread)
  {
    std::unique_lock<std::mutex> ul(windowCreationLock);
    std::thread th(geom_view::thread_func, this);
    th.detach();
    windowCreationCV.wait(ul);
  } else
    geom_view::thread_func(this);
}

void geom_view::close() {
  if(!iface || !iface->window)
    return;
	glfwSetWindowShouldClose(iface->window, 1);
  glfwPostEmptyEvent();
  std::unique_lock<std::mutex> ul(windowCreationLock);
  windowCreationCV.wait(ul);
}

void geom_view::init(const std::vector<std::string>& filenames_) {
  reloadLock.lock();
  files.clear();
  for(const auto& fn : filenames_)
    files.push_back(std::make_pair(fn, true));
  reloadLock.unlock();
  init();
}

void geom_view::init(const std::string& filename) {
  std::vector<std::string> filenames;
  filenames.push_back(filename);
  init(filenames);
}

void geom_view::reload(bool resetCam) {
  if(!ren_ptr)
    init();
  
  if(std::this_thread::get_id() == th_id) { // in case of callbacks we need to load right away
    files.clear();
    if(obj_root) {
      for(auto& item : obj_root->children())
        files.push_back(std::make_pair(item->name, true));
    }
    reload_files(obj_root, ren_ptr, files);
    files.clear();
    return;
  }
    
  reloadLock.lock();
  reloadFlag = true;
  files.clear();
  if(obj_root) {
    for(auto& item : obj_root->children())
      files.push_back(std::make_pair(item->name, true));
  }
  resetCameraRequest = resetCam;
  reloadLock.unlock();
  glfwPostEmptyEvent();  

  //no need for that stuff with context ownership management since it is more stable just to send request to mainthread. EmptyEvent allows to process request -- that is it!
  //glfwMakeContextCurrent(ren_ptr->win); // typically calling thread differs from one that creates geomView main window so to make GL functions work properly we have to set current GL context to one created during main window creation.
  //::reload(obj_root, ren_ptr);
}

void geom_view::reload(const std::vector<std::pair<std::string, bool>>& files_, bool resetCam) {
  if(!ren_ptr)
    init();
  while(true) {
    std::unique_lock<std::mutex> ul(reloadLock);
    if(files.size()==0)
      break;
  }
  reloadLock.lock();
  reloadFlag = true;
  files = files_;
  resetCameraRequest = resetCam;
  reloadLock.unlock();
  glfwPostEmptyEvent();
}

void geom_view::visibilities(const std::vector<std::pair<std::string, bool>>& files_) {
  if(!ren_ptr)
    init();
  
  while(true) {
    std::unique_lock<std::mutex> ul(reloadLock);
    if(files.size()==0)
      break;
  }
  
  reloadLock.lock();
  changeVisibilityFlag = true;
  files = files_;
  reloadLock.unlock();
  glfwPostEmptyEvent();
}

void geom_view::setCallBack(void* data, void (*callback)(void*, std::vector<std::string>& sId, double x, double y, double z)) {
  if(ren_ptr) {
    ren_ptr->controlPointMoved = callback;
    ren_ptr->callbackData = data;
  }
  //controlPointMoved = callback;
  //callbackData = data;
}

void geom_view::setSelectCallBack(void* data, void (*callback)(void*, const std::vector<std::tuple<std::vector<std::string>, size_t, float>>&)) {
  if(ren_ptr) {
    ren_ptr->selected = callback;
    ren_ptr->selectedData = data;
  }
  //controlPointMoved = callback;
  //selectedData = data;
  
}

void geom_view::centerCamera() {
  if(ren_ptr)
    ren_ptr->center_camera();
  glfwPostEmptyEvent();
}

void geom_view::resetCamera() {
  if(ren_ptr)
    ren_ptr->reset_camera();
  glfwPostEmptyEvent();
}  

void geom_view::showOrigin(bool show) {
  if(ren_ptr)
    ren_ptr->o.show = show;
}

#ifdef _WIN32
HWND geom_view::getNativeWin32Handler() {
	return iface->nativeMSWindowHandler;
}

void geom_view::setParentWin32Handler(HWND _parentMSWindowHandler) {
	iface->parentMSWindowHandler = _parentMSWindowHandler;
}
#endif

void geom_view::addCustomControl(const std::shared_ptr<geom_view_control>& cc) {
  iface->custom_controls.push_back(cc);
}

void proc_xyz(int id, glass_button::eaction act, void* dat) {
  geom_view* g = (geom_view*) dat;
  renderer* ren = g->ren_ptr;
  //std::cout << "xyz button " << id << " experienced action " << act << " rnd=" << rand() << '\n';
  vec3f dir = ren->fp_pos - ren->cam_pos;
  vec3f up = ren->cam_up;
  normalize(dir);
  normalize(up);
  if(id==0) {
    dir = vec3f{1,0,0};
    up = vec3f{0,1,0};
  }
  if(id==1) {
    dir = vec3f{-1,0,0};
    up = vec3f{0,1,0};
  }
  if(id==2) {
    dir = vec3f{0,1,0};
    up = vec3f{1,0,0};
  }
  if(id==3) {
    dir = vec3f{0,-1,0};
    up = vec3f{1,0,0};
  }
  if(id==4) {
    dir = vec3f{0,0,1};
    up = vec3f{1,0,0};
  }
  if(id==5) {
    dir = vec3f{0,0,-1};
    up = vec3f{1,0,0};
  }
  //rotate 90
  if(id==6)
    up = cross_prod(up, dir);
  if(id==7)
    up = cross_prod(dir, up);
  //center and reset
  if(id==8)
    ren->center_camera();
  if(id==9)
    ren->reset_camera();
  //background color
  if(id==10)
    ren->need_bg_color_picker = !ren->need_bg_color_picker;

  if(id==11)
    ren->o.show = !ren->o.show;

  if(id==12) {
    int nx,ny;
    std::vector<unsigned char> buff;
    g->getBuffer(nx,ny,buff);
    image img(nx, ny);
    for(int j = 0; j < ny; j++) {
      for(int i = 0; i < nx; i++) {
        unsigned char* c_ptr = &buff[(j*nx + i)*3];
        color c((double) *c_ptr, (double) *(c_ptr+1), (double) *(c_ptr+2));
        img.set(c, i, j);
      }
    }
    auto png_bin_buff = write_png_file(&img);
    static int file_id = 0;
    std::ofstream pngf((std::string("fb_") + std::to_string(file_id++) + std::string(".png")).c_str(), std::ios_base::binary);
    pngf.write((const char*) png_bin_buff.data(), png_bin_buff.size());
    pngf.close();
  }

  if(id < 8 && dir*dir) {
    ren->cam_pos = ren->fp_pos - std::sqrt((ren->cam_pos - ren->fp_pos) * (ren->cam_pos - ren->fp_pos)) * dir;
    ren->cam_up = up;
  }
  glfwPostEmptyEvent();
}

static void iterate_highlight(object* obj, const std::vector<std::string>& sobject, bool value=true, int level=0) {
  for(object* c : obj->children())
  {
    if(level + 1 >= sobject.size() || c->name == sobject[level + 1])
      iterate_highlight(c, sobject, value, level+1);
  }
  if(obj->item) {
    if(level + 1 >= sobject.size() || sobject[level + 1] == "-1")
      obj->item->highlight(-1, value);
    else {
      int pid = atoi(sobject[level + 1].c_str());
      obj->item->highlight(pid, value);
    }
  }
}

void geom_view::highlight(const std::vector<std::string>& item, bool value) {
  iterate_highlight(obj_root, item, value, 0);
  glfwPostEmptyEvent();
}

void geom_view::highlight(const std::string& item, bool value) {
  auto items = tokenize(item, ":");
  highlight(items, value);
}

std::vector<std::string> geom_view::tokenize(const std::string& in, const std::string& delim) {
  return split(in, delim);
}
