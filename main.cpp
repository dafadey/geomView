#include <iostream>
#include <limits>
#include "draw.h"
#include "interface.h"
#include "object.h"
#include "imgui_controls.h"
#include "fb2way.h"
#include "glass_buttons.h"
#include "buttons.png.inl"
#include "geom_view.h"
#include "tools.h"

#ifdef _MSC_VER
#include <ole2.h>
#include <oleidl.h>
class DragAndDrop : public IDropTarget {
  HRESULT Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) override;
  HRESULT DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) override { return S_OK; }
  HRESULT DragLeave() override { return S_OK; }
  HRESULT DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) override { return S_OK; }
  HRESULT QueryInterface(REFIID riid, void** ppvObject) override { 
	  //std::cout << "!!! QueryInterface: REFIID=" << (void*) &riid << '\n';
	  return S_OK;
  }
  ULONG AddRef() override { 
	  //std::cout << "!!! AddRef\n"; 
	  return 1;
  }
  ULONG Release() override { 
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

void proc_xyz(int id, glass_button::eaction act, void* dat) {
  renderer* ren = (renderer*) dat;
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

  if(id < 8 && dir*dir) {
    ren->cam_pos = ren->fp_pos - std::sqrt((ren->cam_pos - ren->fp_pos) * (ren->cam_pos - ren->fp_pos)) * dir;
    ren->cam_up = up;
  }
  glfwPostEmptyEvent();
}

int main(int argc, char* argv[]) {
  std::cout << "Hallo!\n";

  imgui_interface iface;
  iface.init();
    
  object* obj_root = new object;
  obj_root->name = "root";
  renderer ren;
  ren.init(iface.window, obj_root);

  for(int i=1; i<argc; i++)
    load_objects(obj_root, argv[i], &ren);
 
  ren.reset_camera();

  glfwSetWindowUserPointer(iface.window, (void*) &ren);
  
  fb2way fb2;
  
  int wx, wy;
  glfwGetWindowSize(iface.window, &wx, &wy);
  fb2.init(wx, wy);
  
  glass_buttons btns(iface.window);
  btns.init(buttons_png, buttons_png_len);
  //btns.init(nullptr, 0);
  btns.bgtex = fb2.fb_texture;
  
  for(int i=0;i<11;i++)
    btns.addButton(glass_button(i, 10+(10+32)*i, 10, 32, 32, 32*i, 0, 32*(i+1), 32, proc_xyz, &ren));
  

  #ifdef _MSC_VER
  if (OleInitialize(NULL) != S_OK)
    std::cout << "failed to initialize COM\n";
  DragAndDrop dragAndDropTarget;
  dragAndDropTarget.obj_root = obj_root;
  dragAndDropTarget.ren = &ren;
  HWND hWnd = iface.nativeMSWindowHandler;
  auto RegDADres = RegisterDragDrop(hWnd, &dragAndDropTarget);
  if (RegDADres != S_OK)
	std::cout << "register drag and drop failed: " << RegDADres << '\n';
  #endif

  while (!glfwWindowShouldClose(iface.window))  {
    glfwWaitEvents();
    mainloop_pipeline(&btns, &fb2, &ren, iface.window, obj_root);
    glfwSwapBuffers(iface.window);
    //glFlush();
  }
  
  #ifdef _MSC_VER
  RevokeDragDrop(hWnd);
  OleUninitialize();
  #endif
  
  delete obj_root;
  iface.close();
}
