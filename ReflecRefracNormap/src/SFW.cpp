// ---------------------------------------------------------------------------
// Project Name   : Simple Framework
// File Name    : SFW.h
// Author     : Sun Tjen Fam
// Creation Date  : 2008/09/15
// Purpose      : header file for a simple OpenGL framework
// ---------------------------------------------------------------------------

#include "SFW.h"

// ---------------------------------------------------------------------------

namespace SFW
{

// ---------------------------------------------------------------------------
// static variables

static System*  spSystem;
static Input* spInput;

// ---------------------------------------------------------------------------

LRESULT CALLBACK winCallBack(HWND hWin, UINT msg, WPARAM wp, LPARAM lp);

// ---------------------------------------------------------------------------
// implementation of matrix class


// ---------------------------------------------------------------------------

System* System::Instance()
{
  if (spSystem)
    return spSystem;

  // allocate a new system class
  spSystem = new System;
  assert(spSystem);

  return spSystem;
}

// ---------------------------------------------------------------------------

void System::Free()
{
  if (spSystem)
  {
    delete spSystem;
    spSystem = 0;
  }
}

// ---------------------------------------------------------------------------

System::System() : 
  mhAppInstance(0), 
  mpWinTitle(0), 
  mpWinClassName(0), 
  mhWin(0), 
  mWinExists(false), 
  mWinActive(false), 
  mpConsoleTitle(0), 
  mhDeviceContext(0), 
  mhRenderContext(0), 
  mFrameRateMax(60), 
  mFrameCounter(0), 
  mFrameRate(0), 
  mFrameTime(0)
{
  // reset frame related variables
  FrameReset();
}

// ---------------------------------------------------------------------------

System::~System()
{
  if (mhRenderContext)
    wglDeleteContext(mhRenderContext);
  if (mhDeviceContext)
    ReleaseDC(mhWin, mhDeviceContext);

  // free the window

  // free the console

  delete [] mpWinClassName;
}

// ---------------------------------------------------------------------------

bool System::InitWindow(HINSTANCE hAppInst, LPCSTR pTitle, u32 width, u32 height)
{
  // store the window width and height
  mWinSizeX = width;
  mWinSizeY = height;
  
  // keep a copy of the handle to the application
  mhAppInstance = hAppInst;

  if (!pTitle)
    pTitle = "NONAME";

  mpWinTitle    = 
  mpWinClassName  = new s8 [strlen(pTitle) + 1];
  assert(mpWinTitle);

  strcpy((char*)(mpWinTitle), pTitle);

  // setup the window class
  mWinClass.style     = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  mWinClass.lpfnWndProc = winCallBack;
  mWinClass.cbClsExtra  = 0;
  mWinClass.cbWndExtra  = 0;
  mWinClass.hInstance   = mhAppInstance;
  mWinClass.hIcon     = LoadIcon(NULL,IDI_EXCLAMATION);
  mWinClass.hCursor   = LoadCursor(NULL,IDC_ARROW);
  mWinClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
  mWinClass.lpszMenuName  = NULL;
  mWinClass.lpszClassName = mpWinClassName;
  
  RegisterClass(&mWinClass);
  
  // get the screen size
  long scrSizeX = GetSystemMetrics(SM_CXSCREEN);
  long scrSizeY = GetSystemMetrics(SM_CYSCREEN);

  // get the rect size
  long rectSizeX = mWinSizeX;
  long rectSizeY = mWinSizeY;
  
  RECT rect;
  
  // center the window in the screen
  rect.left = (scrSizeX - rectSizeX) / 2;
  rect.top  = (scrSizeY - rectSizeY) / 2;
  rect.right  = rect.left + rectSizeX;
  rect.bottom = rect.top  + rectSizeY;

  // adjust the rect to take into account window's title bar and border
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

  // create the window
  mhWin = CreateWindow(mpWinClassName, mpWinTitle, WS_OVERLAPPEDWINDOW, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, mhAppInstance, NULL);

  ShowWindow  (mhWin, SW_SHOW);
  UpdateWindow(mhWin);

  // window is currently exists
  mWinExists = true;
  
  // =========================
  // initialize openGL window
  // =========================

  mhDeviceContext = 0;
  mhRenderContext = 0;

  // get the device context
  if ((mhDeviceContext = GetDC(mhWin)) == 0)
    return false;
  
  PIXELFORMATDESCRIPTOR pfd;

  memset(&pfd, 0, sizeof(pfd));

  pfd.nSize     = sizeof(pfd);
  pfd.nVersion    = 1;
  pfd.dwFlags     = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType    = PFD_TYPE_RGBA;
  pfd.cColorBits    = 32;
  pfd.cDepthBits    = 24;
  pfd.iLayerType    = PFD_MAIN_PLANE;
  pfd.cStencilBits  = 8;
  int iFormat     = ChoosePixelFormat(mhDeviceContext, &pfd);
  
  SetPixelFormat(mhDeviceContext, iFormat, &pfd);

  // create OpenGL context
  if ((mhRenderContext = wglCreateContext(mhDeviceContext)) == 0)
  {
    ReleaseDC(mhWin, mhDeviceContext);
    mhDeviceContext = 0;
    return false;
  }

  // set the current openGL render context
  wglMakeCurrent(mhDeviceContext, mhRenderContext);

  // make sure GLEW initialized correctly
  if (glewInit() != GLEW_OK)
    SFW_HALT("fail to initialize GLEW!!");

  // make sure that there is OpenGL 2.0 support
  // * need at LEAST 2.0 for shaders
  if (!GLEW_VERSION_2_0)
    SFW_HALT("OpenGL 2.0 is not supported!!");

  // create an openGL render context
  if (wglCreateContextAttribsARB)
  {
    int contextAttrs[] = 
    {
      // request for v2.0 context
      WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
      WGL_CONTEXT_MINOR_VERSION_ARB, 0,

      WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
      0
    };

    HGLRC prevRenderContext = mhRenderContext;

    // create an openGL render context
    if ((mhRenderContext = wglCreateContextAttribsARB(mhDeviceContext, 0, contextAttrs)) == 0)
      // revert to the previous one
      mhRenderContext = prevRenderContext;
  }

  // set the current openGL render context
  wglMakeCurrent(mhDeviceContext, mhRenderContext);
  
  return true;
}

// ---------------------------------------------------------------------------

bool System::InitConsole(LPCSTR pTitle)
{
  if (AllocConsole() == false)
    return false;
  
  FILE* file;
  
  freopen_s(&file, "CONOUT$", "wt", stdout);
  freopen_s(&file, "CONOUT$", "wt", stderr);
  freopen_s(&file, "CONOUT$", "wt", stdin);

  SetConsoleTitle(pTitle);
  
  return true;
}

// ---------------------------------------------------------------------------

void System::WinText(const char* pText)
{
  if (!pText)
    return;

  SetWindowText(mhWin, pText);
}

// ---------------------------------------------------------------------------

void System::FrameReset(u32 frameRateMax)
{
  mFrameRateMax = Max<u32>(frameRateMax, 1);
  mFrameTimeMin = 1.0 / mFrameRateMax;
  
  mFrameCounter = 0;
  mFrameRate    = mFrameRateMax;
  mFrameTime    = mFrameTimeMin;
}

// ---------------------------------------------------------------------------

void System::FrameStart()
{
  // make the OpenGL context current
  wglMakeCurrent(mhDeviceContext, mhRenderContext);

  // record the time the frame start
  GetHiResTime(&mFrameTimeStart);
}

// ---------------------------------------------------------------------------

void System::FrameEnd()
{
  // wait till OpenGL is done and swap the frame buffer
  glFinish();

  // get the total time the CPU consumed
  GetHiResTime(&mFrameTimeCPU);
  mFrameTimeCPU -= mFrameTimeStart;

  // wait till at least mFrameTimeMin seconds passed
  do
  {
    GetHiResTime(&mFrameTimeEnd);
    mFrameTime = mFrameTimeEnd - mFrameTimeStart;
  }
  while (mFrameTime < mFrameTimeMin);

  // swap the frame buffers
  SwapBuffers(mhDeviceContext);

  // increment the frame counter
  ++mFrameCounter;

  // =======================
  // handle window messages
  // =======================
  {
    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage (&msg);
    }
  }
}

// ---------------------------------------------------------------------------

LRESULT CALLBACK System::winCallBack(HWND hWin, UINT msg, WPARAM wp, LPARAM lp) 
{
  HDC     dc;   
  PAINTSTRUCT ps;

  switch (msg)
  {
  // when the window is created
  case WM_CREATE:
    break;

  // when the rectangle is drawn
  case WM_PAINT:
    dc = BeginPaint(hWin, &ps);

    // Cleans up the painting process
    EndPaint(hWin, &ps);
    break;

  // When it's time for the window to go away
  case WM_DESTROY:
    //PostQuitMessage(0);
    spSystem->mWinExists = false;
    break;

  // called any time the window is moved
  case WM_MOVE:
    // Invalidate the rect to force a redraw
    InvalidateRect(hWin, NULL, FALSE);
    break;

  case WM_ACTIVATE:
    spSystem->mWinActive = (LOWORD(wp) == WA_INACTIVE) ? false : true;
    break;

  case WM_SIZE:
    {
    // adjust the window size
    spSystem->mWinSizeX = LOWORD(lp);
    spSystem->mWinSizeY = HIWORD(lp);
    break;
    }

#if 0
  case WM_MOUSEMOVE:
    {
        //int xPosAbsolute = GET_X_PARAM(lp); 
        //int yPosAbsolute = GET_Y_PARAM(lp);

    PRINT("%d %d\n", LOWORD(lp), HIWORD(lp));

    

    break;
    }

  case WM_KEYDOWN:
    {
    PRINT("%d\n", LOWORD(lp), HIWORD(lp));

    

    break;
    }

  SM_MOUSEWHEELPRESENT

  case WM_SIZE:
    {
    PRINT("%d\n", LOWORD(lp), HIWORD(lp));

    

    break;
    }
#endif

  default:
    return DefWindowProc(hWin, msg, wp, lp);
  }

  return 0;
}


// ---------------------------------------------------------------------------
// implementation of input class

Input* Input::Instance()
{
  if (spInput)
    return spInput;

  // create an instance of the input class
  spInput = new Input;
  assert(spInput);
  
  return spInput;
}

// ---------------------------------------------------------------------------

void Input::Free()
{
  if (spInput)
  {
    delete spInput;
    spInput = 0;
  }
}

// ---------------------------------------------------------------------------

Input::Input()
{
  mpKeyStateCurr = new u8 [256];
  mpKeyStatePrev = new u8 [256];
  
  assert(mpKeyStateCurr && mpKeyStatePrev);
  
  memset(mpKeyStateCurr, 0, sizeof(u8) * 256);
  memset(mpKeyStatePrev, 0, sizeof(u8) * 256);

  mCursorPosX   = 0;
  mCursorPosY   = 0;
  mCursorDeltaX = 0;
  mCursorDeltaY = 0;
}

// ---------------------------------------------------------------------------

Input::~Input()
{
  if (mpKeyStateCurr)
    delete [] mpKeyStateCurr;
  if (mpKeyStatePrev)
    delete [] mpKeyStatePrev;

  mpKeyStateCurr = 0;
  mpKeyStatePrev = 0;
}

// ---------------------------------------------------------------------------

void Input::update()
{
  // backup the current to previous
  memcpy(mpKeyStatePrev, mpKeyStateCurr, sizeof(u8) * 256);

  // get current keyboard state
  GetKeyboardState(mpKeyStateCurr);

  if (spSystem)
  {
    POINT coord;

    // get the cursor position and adjust it to the client area
    GetCursorPos  (&coord);
    ScreenToClient  (spSystem->mhWin, &coord);

    mCursorDeltaX = coord.x - mCursorPosX;
    mCursorDeltaY = coord.y - mCursorPosY;
    mCursorPosX   = coord.x;
    mCursorPosY   = coord.y;
  }
  else
  {
    mCursorPosX   = 0;
    mCursorPosY   = 0;
    mCursorDeltaX = 0;
    mCursorDeltaY = 0;
  }
}

// ---------------------------------------------------------------------------
// implementatio of matrix class


// ---------------------------------------------------------------------------
// constructors

Mtx44::Mtx44()
{
}

// ---------------------------------------------------------------------------

Mtx44::Mtx44(const f32* mm)
{ 
  RowCol(0, 0) = mm[ 0]; RowCol(0, 1) = mm[ 1]; RowCol(0, 2) = mm[ 2]; RowCol(0, 3) = mm[ 3]; 
  RowCol(1, 0) = mm[ 4]; RowCol(1, 1) = mm[ 5]; RowCol(1, 2) = mm[ 6]; RowCol(1, 3) = mm[ 7]; 
  RowCol(2, 0) = mm[ 8]; RowCol(2, 1) = mm[ 9]; RowCol(2, 2) = mm[10]; RowCol(2, 3) = mm[11]; 
  RowCol(3, 0) = mm[12]; RowCol(3, 1) = mm[13]; RowCol(3, 2) = mm[14]; RowCol(3, 3) = mm[15]; 
}

// ---------------------------------------------------------------------------

Mtx44::Mtx44(f32 m00, f32 m01, f32 m02, f32 m03, 
       f32 m10, f32 m11, f32 m12, f32 m13, 
       f32 m20, f32 m21, f32 m22, f32 m23, 
       f32 m30, f32 m31, f32 m32, f32 m33)
{
  RowCol(0, 0) = m00; RowCol(0, 1) = m01; RowCol(0, 2) = m02; RowCol(0, 3) = m03; 
  RowCol(1, 0) = m10; RowCol(1, 1) = m11; RowCol(1, 2) = m12; RowCol(1, 3) = m13; 
  RowCol(2, 0) = m20; RowCol(2, 1) = m21; RowCol(2, 2) = m22; RowCol(2, 3) = m23; 
  RowCol(3, 0) = m30; RowCol(3, 1) = m31; RowCol(3, 2) = m32; RowCol(3, 3) = m33; 
}

// ---------------------------------------------------------------------------

Mtx44::Mtx44(const Mtx44& rhs)
{
  (*this) = rhs;
}

// ---------------------------------------------------------------------------
// operators overload

Mtx44 Mtx44::operator+(const Mtx44& rhs) const
{
  return Mtx44(RowCol(0, 0) + rhs.RowCol(0, 0), RowCol(0, 1) + rhs.RowCol(0, 1), RowCol(0, 2) + rhs.RowCol(0, 2), RowCol(0, 3) + rhs.RowCol(0, 3), 
         RowCol(1, 0) + rhs.RowCol(1, 0), RowCol(1, 1) + rhs.RowCol(1, 1), RowCol(1, 2) + rhs.RowCol(1, 2), RowCol(1, 3) + rhs.RowCol(1, 3), 
         RowCol(2, 0) + rhs.RowCol(2, 0), RowCol(2, 1) + rhs.RowCol(2, 1), RowCol(2, 2) + rhs.RowCol(2, 2), RowCol(2, 3) + rhs.RowCol(2, 3), 
         RowCol(3, 0) + rhs.RowCol(3, 0), RowCol(3, 1) + rhs.RowCol(3, 1), RowCol(3, 2) + rhs.RowCol(3, 2), RowCol(3, 3) + rhs.RowCol(3, 3));
}

// ---------------------------------------------------------------------------

Mtx44 Mtx44::operator-(const Mtx44& rhs) const
{
  return Mtx44(RowCol(0, 0) - rhs.RowCol(0, 0), RowCol(0, 1) - rhs.RowCol(0, 1), RowCol(0, 2) - rhs.RowCol(0, 2), RowCol(0, 3) - rhs.RowCol(0, 3), 
         RowCol(1, 0) - rhs.RowCol(1, 0), RowCol(1, 1) - rhs.RowCol(1, 1), RowCol(1, 2) - rhs.RowCol(1, 2), RowCol(1, 3) - rhs.RowCol(1, 3), 
         RowCol(2, 0) - rhs.RowCol(2, 0), RowCol(2, 1) - rhs.RowCol(2, 1), RowCol(2, 2) - rhs.RowCol(2, 2), RowCol(2, 3) - rhs.RowCol(2, 3), 
         RowCol(3, 0) - rhs.RowCol(3, 0), RowCol(3, 1) - rhs.RowCol(3, 1), RowCol(3, 2) - rhs.RowCol(3, 2), RowCol(3, 3) - rhs.RowCol(3, 3));
}

// ---------------------------------------------------------------------------

void Mtx44::Print() const
{
  for (u32 i = 0; i < 16; i++)
  {
    printf("%8f ", v[i]);

    if (((i + 1) & 0x03) == 0)
      printf("\n");
  }
}

// ---------------------------------------------------------------------------

Mtx44 Mtx44::Mult(f32 s) const
{
  return Mtx44(s * RowCol(0, 0), s * RowCol(0, 1), s * RowCol(0, 2), s * RowCol(0, 3), 
         s * RowCol(1, 0), s * RowCol(1, 1), s * RowCol(1, 2), s * RowCol(1, 3), 
         s * RowCol(2, 0), s * RowCol(2, 1), s * RowCol(2, 2), s * RowCol(2, 3), 
         s * RowCol(3, 0), s * RowCol(3, 1), s * RowCol(3, 2), s * RowCol(3, 3));
}

// ---------------------------------------------------------------------------

const Mtx44& Mtx44::MultThis(f32 s)
{
  RowCol(0, 0) *= s; RowCol(0, 1) *= s; RowCol(0, 2) *= s; RowCol(0, 3) *= s;
  RowCol(1, 0) *= s; RowCol(1, 1) *= s; RowCol(1, 2) *= s; RowCol(1, 3) *= s;
  RowCol(2, 0) *= s; RowCol(2, 1) *= s; RowCol(2, 2) *= s; RowCol(2, 3) *= s;
  RowCol(3, 0) *= s; RowCol(3, 1) *= s; RowCol(3, 2) *= s; RowCol(3, 3) *= s;
  
  return (*this);
}

// ---------------------------------------------------------------------------

Mtx44 Mtx44::Mult(const Mtx44& rhs) const
{
  return Mtx44(
    RowCol(0, 0) * rhs.RowCol(0, 0) + RowCol(0, 1) * rhs.RowCol(1, 0) + RowCol(0, 2) * rhs.RowCol(2, 0) + RowCol(0, 3) * rhs.RowCol(3, 0), 
    RowCol(0, 0) * rhs.RowCol(0, 1) + RowCol(0, 1) * rhs.RowCol(1, 1) + RowCol(0, 2) * rhs.RowCol(2, 1) + RowCol(0, 3) * rhs.RowCol(3, 1), 
    RowCol(0, 0) * rhs.RowCol(0, 2) + RowCol(0, 1) * rhs.RowCol(1, 2) + RowCol(0, 2) * rhs.RowCol(2, 2) + RowCol(0, 3) * rhs.RowCol(3, 2), 
    RowCol(0, 0) * rhs.RowCol(0, 3) + RowCol(0, 1) * rhs.RowCol(1, 3) + RowCol(0, 2) * rhs.RowCol(2, 3) + RowCol(0, 3) * rhs.RowCol(3, 3), 

    RowCol(1, 0) * rhs.RowCol(0, 0) + RowCol(1, 1) * rhs.RowCol(1, 0) + RowCol(1, 2) * rhs.RowCol(2, 0) + RowCol(1, 3) * rhs.RowCol(3, 0), 
    RowCol(1, 0) * rhs.RowCol(0, 1) + RowCol(1, 1) * rhs.RowCol(1, 1) + RowCol(1, 2) * rhs.RowCol(2, 1) + RowCol(1, 3) * rhs.RowCol(3, 1), 
    RowCol(1, 0) * rhs.RowCol(0, 2) + RowCol(1, 1) * rhs.RowCol(1, 2) + RowCol(1, 2) * rhs.RowCol(2, 2) + RowCol(1, 3) * rhs.RowCol(3, 2), 
    RowCol(1, 0) * rhs.RowCol(0, 3) + RowCol(1, 1) * rhs.RowCol(1, 3) + RowCol(1, 2) * rhs.RowCol(2, 3) + RowCol(1, 3) * rhs.RowCol(3, 3), 

    RowCol(2, 0) * rhs.RowCol(0, 0) + RowCol(2, 1) * rhs.RowCol(1, 0) + RowCol(2, 2) * rhs.RowCol(2, 0) + RowCol(2, 3) * rhs.RowCol(3, 0), 
    RowCol(2, 0) * rhs.RowCol(0, 1) + RowCol(2, 1) * rhs.RowCol(1, 1) + RowCol(2, 2) * rhs.RowCol(2, 1) + RowCol(2, 3) * rhs.RowCol(3, 1), 
    RowCol(2, 0) * rhs.RowCol(0, 2) + RowCol(2, 1) * rhs.RowCol(1, 2) + RowCol(2, 2) * rhs.RowCol(2, 2) + RowCol(2, 3) * rhs.RowCol(3, 2), 
    RowCol(2, 0) * rhs.RowCol(0, 3) + RowCol(2, 1) * rhs.RowCol(1, 3) + RowCol(2, 2) * rhs.RowCol(2, 3) + RowCol(2, 3) * rhs.RowCol(3, 3), 

    RowCol(3, 0) * rhs.RowCol(0, 0) + RowCol(3, 1) * rhs.RowCol(1, 0) + RowCol(3, 2) * rhs.RowCol(2, 0) + RowCol(3, 3) * rhs.RowCol(3, 0), 
    RowCol(3, 0) * rhs.RowCol(0, 1) + RowCol(3, 1) * rhs.RowCol(1, 1) + RowCol(3, 2) * rhs.RowCol(2, 1) + RowCol(3, 3) * rhs.RowCol(3, 1), 
    RowCol(3, 0) * rhs.RowCol(0, 2) + RowCol(3, 1) * rhs.RowCol(1, 2) + RowCol(3, 2) * rhs.RowCol(2, 2) + RowCol(3, 3) * rhs.RowCol(3, 2), 
    RowCol(3, 0) * rhs.RowCol(0, 3) + RowCol(3, 1) * rhs.RowCol(1, 3) + RowCol(3, 2) * rhs.RowCol(2, 3) + RowCol(3, 3) * rhs.RowCol(3, 3));
}

// ---------------------------------------------------------------------------

const Mtx44& Mtx44::MultThis(const Mtx44& rhs)
{
  return (*this) = this->Mult(rhs);
}

// ---------------------------------------------------------------------------

Vec3 Mtx44::MultVec(const Vec3& v) const
{
  return Vec3(
    RowCol(0, 0) * v.x + RowCol(0, 1) * v.y + RowCol(0, 2) * v.z + RowCol(0, 3), 
    RowCol(1, 0) * v.x + RowCol(1, 1) * v.y + RowCol(1, 2) * v.z + RowCol(1, 3), 
    RowCol(2, 0) * v.x + RowCol(2, 1) * v.y + RowCol(2, 2) * v.z + RowCol(2, 3));
}

// ---------------------------------------------------------------------------

Vec3 Mtx44::MultVecSR(const Vec3& v) const
{
  return Vec3(
    RowCol(0, 0) * v.x + RowCol(0, 1) * v.y + RowCol(0, 2) * v.z, 
    RowCol(1, 0) * v.x + RowCol(1, 1) * v.y + RowCol(1, 2) * v.z, 
    RowCol(2, 0) * v.x + RowCol(2, 1) * v.y + RowCol(2, 2) * v.z);
}

// ---------------------------------------------------------------------------

Vec4 Mtx44::MultVec(const Vec4& v) const
{
  return Vec4(
    RowCol(0, 0) * v.x + RowCol(0, 1) * v.y + RowCol(0, 2) * v.z + RowCol(0, 3) * v.w, 
    RowCol(1, 0) * v.x + RowCol(1, 1) * v.y + RowCol(1, 2) * v.z + RowCol(1, 3) * v.w, 
    RowCol(2, 0) * v.x + RowCol(2, 1) * v.y + RowCol(2, 2) * v.z + RowCol(2, 3) * v.w, 
    RowCol(3, 0) * v.x + RowCol(3, 1) * v.y + RowCol(3, 2) * v.z + RowCol(3, 3) * v.w);
}

// ---------------------------------------------------------------------------

Mtx44 Mtx44::Transpose() const
{
  Mtx44 res;

  res.RowCol(0, 0) = RowCol(0, 0); res.RowCol(0, 1) = RowCol(1, 0); res.RowCol(0, 2) = RowCol(2, 0); res.RowCol(0, 3) = RowCol(3, 0);
  res.RowCol(1, 0) = RowCol(0, 1); res.RowCol(1, 1) = RowCol(1, 1); res.RowCol(1, 2) = RowCol(2, 1); res.RowCol(1, 3) = RowCol(3, 1);
  res.RowCol(2, 0) = RowCol(0, 2); res.RowCol(2, 1) = RowCol(1, 2); res.RowCol(2, 2) = RowCol(2, 2); res.RowCol(2, 3) = RowCol(3, 2);
  res.RowCol(3, 0) = RowCol(0, 3); res.RowCol(3, 1) = RowCol(1, 3); res.RowCol(3, 2) = RowCol(2, 3); res.RowCol(3, 3) = RowCol(3, 3);

  return res;
}

// ---------------------------------------------------------------------------

const Mtx44& Mtx44::TransposeThis()
{
  f32 f;

  // swap the 1st row
  f   = RowCol(0, 1); RowCol(0, 1) = RowCol(1, 0); RowCol(1, 0) = f;
  f   = RowCol(0, 2); RowCol(0, 2) = RowCol(2, 0); RowCol(2, 0) = f;
  f   = RowCol(0, 3); RowCol(0, 3) = RowCol(3, 0); RowCol(3, 0) = f;

  // swap the 2nd row
  f   = RowCol(1, 2); RowCol(1, 2) = RowCol(2, 1); RowCol(2, 1) = f;
  f   = RowCol(1, 3); RowCol(1, 3) = RowCol(3, 1); RowCol(3, 1) = f;

  // swap the 3rd row
  f   = RowCol(2, 3); RowCol(2, 3) = RowCol(3, 2); RowCol(3, 2) = f;
  
  return (*this);
}

// ---------------------------------------------------------------------------

Mtx44 Mtx44::Inverse() const
{
  SFW_HALT("NOT IMPLEMENTED YET!!!");
}

// ---------------------------------------------------------------------------

const Mtx44& Mtx44::InverseThis()
{
  (*this) = this->Inverse();

  return (*this);
}

// ---------------------------------------------------------------------------

// the following functions will be applied to 'this' matrix
const Mtx44& Mtx44::ZeroThis()
{
  RowCol(0, 0) = RowCol(0, 1) = RowCol(0, 2) = RowCol(0, 3) = 
  RowCol(1, 0) = RowCol(1, 1) = RowCol(1, 2) = RowCol(1, 3) = 
  RowCol(2, 0) = RowCol(2, 1) = RowCol(2, 2) = RowCol(2, 3) = 
  RowCol(3, 0) = RowCol(3, 1) = RowCol(3, 2) = RowCol(3, 3) = 0.0f;

  return (*this);
}

// ---------------------------------------------------------------------------

const Mtx44& Mtx44::IdentityThis()
{
  RowCol(0, 1) = RowCol(0, 2) = RowCol(0, 3) = 
  RowCol(1, 0) = RowCol(1, 2) = RowCol(1, 3) = 
  RowCol(2, 0) = RowCol(2, 1) = RowCol(2, 3) = 
  RowCol(3, 0) = RowCol(3, 1) = RowCol(3, 2) = 0.0f;

  RowCol(0, 0) = RowCol(1, 1) = RowCol(2, 2) = RowCol(3, 3) = 1.0f;

  return (*this);
}

// ---------------------------------------------------------------------------

const Mtx44& Mtx44::ScaleThis(f32 x, f32 y, f32 z)
{
  RowCol(0, 0) *= x; RowCol(0, 1) *= x; RowCol(0, 2) *= x; RowCol(0, 3) *= x;
  RowCol(1, 0) *= y; RowCol(1, 1) *= y; RowCol(1, 2) *= y; RowCol(1, 3) *= y;
  RowCol(2, 0) *= z; RowCol(2, 1) *= z; RowCol(2, 2) *= z; RowCol(2, 3) *= z;

  return (*this);
}

// ---------------------------------------------------------------------------

const Mtx44& Mtx44::RotateThis(f32 x, f32 y, f32 z, f32 angle)
{
  return (*this) = Mtx44::Rotate(x, y, z, angle) * (*this);
}

// ---------------------------------------------------------------------------

const Mtx44& Mtx44::TranslateThis(f32 x, f32 y, f32 z)
{
  return (*this) = Mtx44::Translate(x, y, z) * (*this);
}

// ---------------------------------------------------------------------------
// the following functions construct a matrix

Mtx44 Mtx44::Zero()
{
  static Mtx44 res(
    0.0f, 0.0f, 0.0f, 0.0f, 
    0.0f, 0.0f, 0.0f, 0.0f, 
    0.0f, 0.0f, 0.0f, 0.0f, 
    0.0f, 0.0f, 0.0f, 0.0f);

  return res;
}

// ---------------------------------------------------------------------------

Mtx44 Mtx44::Identity()
{
  static Mtx44 res(
    1.0f, 0.0f, 0.0f, 0.0f, 
    0.0f, 1.0f, 0.0f, 0.0f, 
    0.0f, 0.0f, 1.0f, 0.0f, 
    0.0f, 0.0f, 0.0f, 1.0f);
  
  return res;
}

// ---------------------------------------------------------------------------

Mtx44 Mtx44::Scale(f32 x, f32 y, f32 z)
{
  return Mtx44(
    x,    0.0f, 0.0f, 0.0f, 
    0.0f, y,    0.0f, 0.0f, 
    0.0f, 0.0f, z,    0.0f, 
    0.0f, 0.0f, 0.0f, 1.0f);
}

// ---------------------------------------------------------------------------

Mtx44 Mtx44::Rotate(f32 x, f32 y, f32 z, f32 angle)
{
  f32 c   = cos(angle);
  f32 omc = 1.0f - c;
  f32 s   = sin(angle);

  f32 xx = x * x;
  f32 xy = x * y;
  f32 xz = x * z;
  f32 yy = y * y;
  f32 yz = y * z;
  f32 zz = z * z;

  Mtx44 A(
    c,    0.0f, 0.0f, 0.0f, 
    0.0f, c,    0.0f, 0.0f, 
    0.0f, 0.0f, c,    0.0f, 
    0.0f, 0.0f, 0.0f, 1.0f);
  Mtx44 B(
    xx,   xy,   xz,   0.0f, 
    xy,   yy,   yz,   0.0f, 
    xz,   yz,   zz,   0.0f, 
    0.0f, 0.0f, 0.0f, 0.0f);
  Mtx44 C(
    0.0f,   -z,    y, 0.0f, 
       z, 0.0f,   -x, 0.0f, 
      -y,    x, 0.0f, 0.0f, 
    0.0f, 0.0f, 0.0f, 0.0f);

  return A + omc * B + s * C;
}

// ---------------------------------------------------------------------------

Mtx44 Mtx44::Translate(f32 x, f32 y, f32 z)
{
  return Mtx44(
    1.0f, 0.0f, 0.0f, x, 
    0.0f, 1.0,  0.0f, y, 
    0.0f, 0.0f, 1.0,  z, 
    0.0f, 0.0f, 0.0f, 1.0f);
}

// ---------------------------------------------------------------------------
// calculate a look at matrix
// * camera is looking along -z axis and y is up
Mtx44 Mtx44::LookAt(f32 posX, f32 posY, f32 posZ, f32 targetX, f32 targetY, f32 targetZ, f32 upX,  f32 upY,  f32 upZ)
{
  return LookAt(
    Vec3(posX, posY, posZ), 
    Vec3(targetX, targetY, targetZ), 
    Vec3(upX, upY, upZ));
}

// ---------------------------------------------------------------------------

Mtx44 Mtx44::LookAt(const Vec3& pos, const Vec3& target, const Vec3& upRef)
{
  Vec3 dir, side, up;

  // calculate the direction vector
  dir  = pos - target;
  SFW_ASSERT_MESG(!dir.IsZero(), "zero direction vector!");
  dir.NormalizeThis();
  
  // calculate the side vector
  side = upRef.Cross(dir);
  SFW_ASSERT_MESG(!side.IsZero(), "direction parallel with reference up vector!");
  side.NormalizeThis();

  // calculate the 'real' up vector
  up = dir.Cross(side);
  
  // return the matrix
  return Mtx44(
    side.x, side.y, side.z, -(pos.Dot(side)), 
    up.x,   up.y,   up.z,   -(pos.Dot(up)), 
    dir.x,  dir.y,  dir.z,  -(pos.Dot(dir)), 
    0.0f,   0.0f,   0.0f,   1.0f);
}

// ---------------------------------------------------------------------------

Image* Image::Create(u32 sizeX, u32 sizeY, u32 bpp)
{
  SFW_ASSERT_PARM(sizeX && sizeY && ((bpp == 24) || (bpp == 32)));

  Image* pImage = new Image;
  SFW_ASSERT_ALLOC(pImage);

  pImage->mBPP  = bpp;
  pImage->mSizeX  = sizeX;
  pImage->mSizeY  = sizeY;

  pImage->mpData  = new u8 [pImage->mSizeX * pImage->mSizeY * pImage->mBPP / 8];
  SFW_ASSERT_ALLOC(pImage->mpData);

  pImage->normal_data  = new SFW::Vec3 [pImage->mSizeX * pImage->mSizeY * pImage->mBPP / 8];
  SFW_ASSERT_ALLOC(pImage->normal_data );

  // zero out the memory
  memset(pImage->mpData, 0, sizeof(u8) * pImage->mSizeX * pImage->mSizeY * pImage->mBPP / 8);
  memset(pImage->normal_data, 0, sizeof(u8) * pImage->mSizeX * pImage->mSizeY * pImage->mBPP / 8);
  
  return pImage;
}

// ---------------------------------------------------------------------------

Image* Image::Load(const char* pName)
{
  SFW_ASSERT_PARM(pName);

  Image* pImage = new Image;
  SFW_ASSERT_ALLOC(pImage);

  // check the extension
  const char* pExt = pName + strlen(pName) - 4;
  SFW_ASSERT_MESG(stricmp(pExt, ".TGA") == 0, "Can only load .TGA file!!");

  FILE* pFile = fopen(pName, "rb");
  SFW_ASSERT_MESG(pFile, "Cannot open '%s'\n", pName);

  // get the file size
  fseek(pFile, 0, SEEK_END);
  u32 fileSize = ftell(pFile);
  fseek(pFile, 0, SEEK_SET);

  u8* pFileData = new u8 [fileSize];
  SFW_ASSERT_ALLOC(pFileData);

  // read in the data
  u32 readSize = (u32)(fread(pFileData, 1, fileSize, pFile));
  SFW_ASSERT_MESG(readSize == fileSize, "Error in reading '%s'\n", pName);

  if (pImage->loadTGA(pFileData, fileSize))
    return pImage;

  // deallocate the image
  delete pImage;

  return 0;
}

// ---------------------------------------------------------------------------

void Image::Save(const char* pName)
{
  SFW_ASSERT_PARM(pName);

  // check the extension
  const char* pExt = pName + strlen(pName) - 4;

  if (stricmp(pExt, ".TGA") == 0)
    saveTGA(pName);
  else if (stricmp(pExt, ".TXT") == 0)
    saveTXT(pName);
  else
  {
    SFW_HALT("Unsupported type!! (%s)", pExt);
  }
  
  return;
}

// ---------------------------------------------------------------------------

void Image::Free(Image* pImage)
{
  if (!pImage)
    return;

  // deallocate the image
  delete pImage;
}

// ---------------------------------------------------------------------------

Image::Image() :
  mpData(0), 
  mBPP(0), 
  mSizeX(0), 
  mSizeY(0)
{
}

// ---------------------------------------------------------------------------

Image::~Image()
{
  if (mpData)
    delete [] mpData;
}

// ---------------------------------------------------------------------------
  
bool Image::loadTGA(u8* pData, u32 dataSize)
{
  SFW_ASSERT_PARM(pData && dataSize)

  // get the image type
  u8 imageType = pData[2];
  SFW_ASSERT_MESG(imageType == 2, "Only support uncompressed, true-color image");

  // get the bits per pixel
  mBPP = pData[16];
  SFW_ASSERT_MESG((mBPP == 24) || (mBPP == 32), "Only support 24 or 32 bits image");

  // get image size
  mSizeX = (pData[13] << 8) | pData[12];
  mSizeY = (pData[15] << 8) | pData[14];

  // get the pointer to the image data area
  // * 18 is the header size
  // * the '0' entry is the number of bytes in the image id field (ignored!)
  u8* pImageData = pData + 18 + pData[0];

  // allocate memory for the data
  mpData = new u8 [mSizeX * mSizeY * mBPP / 8];
  SFW_ASSERT_ALLOC(mpData);

  // get the image descriptor to get the orientation
  // * bit 5 (0 = bottom, 1 = top)
  // * bit 4 (0 = left    1 = right)
  u8  desc    = pData[17];
  u32 rowSize = mSizeX * mBPP / 8;

  // check if need to mirror the image vertically
  if ((desc & 0x20) == 0)
  {
    // mirror data upside down
    for (u32 y = 0; y < mSizeY; ++y)
    {
      u32* pSrc = (u32*)(pImageData + y * rowSize);
      u32* pDst = (u32*)(mpData     + (mSizeY - 1 - y) * rowSize);
      
      memcpy(pDst, pSrc, rowSize);
    }
  }
  else
  {
    memcpy(mpData, pImageData, mSizeY * rowSize);
  }

  // check if need to mirror the image horizontally
  if (desc & 0x10)
  {
    for (u32 y = 0; y < mSizeY; ++y)
    {
      for (u32 x = 0; x < mSizeX / 2; ++x)
      {
        u8* pSrc = mpData + y * rowSize + x * mBPP / 8;
        u8* pDst = mpData + y * rowSize + (mSizeX - 1 - x) * mBPP / 8;
        
        pSrc[0] ^= pDst[0]; pDst[0] ^= pSrc[0]; pSrc[0] ^= pDst[0];
        pSrc[1] ^= pDst[1]; pDst[1] ^= pSrc[1]; pSrc[1] ^= pDst[1];
        pSrc[2] ^= pDst[2]; pDst[2] ^= pSrc[2]; pSrc[2] ^= pDst[2];
        
        if (mBPP == 32)
        {
          pSrc[3] ^= pDst[3];
          pDst[3] ^= pSrc[3];
          pSrc[3] ^= pDst[3];
        }
      }
    }
  }

  return true;
}

// ---------------------------------------------------------------------------

bool Image::saveTGA(const char* pName)
{
  FILE* pFile = fopen(pName, "wb");
  SFW_ASSERT_MESG(pFile, "Cannot open '%s'\n", pName);

  u8 header[18];
  memset(header, 0, sizeof(u8) * 18);

  header[ 2] = 2;

  header[12] = mSizeX & 0x0000FF;
  header[13] = (mSizeX >> 8) & 0x00FF;
  header[14] = mSizeY & 0x0000FF;
  header[15] = (mSizeY >> 8) & 0x00FF;

  header[16] = mBPP;

  // set image orientation to (top, left)
  // * bit 5 (0 = bottom, 1 = top)
  // * bit 4 (0 = left    1 = right)
  header[17] = 0x20;

  if (mBPP == 32)
    header[17] |= 0x08;
  
  u32 written = fwrite(header, 18, 1, pFile);
  SFW_ASSERT_MESG(written, "fail to write!!");

  written = fwrite(mpData, mSizeX * mSizeY * mBPP / 8, 1, pFile);
  SFW_ASSERT_MESG(written, "fail to write!!");

  fclose(pFile);

  return true;
}

// ---------------------------------------------------------------------------

bool Image::saveTXT(const char* pName)
{
  FILE* pFile = fopen(pName, "wt");
  SFW_ASSERT_MESG(pFile, "Cannot open '%s'\n", pName);

  fprintf(pFile, "// Image width:  %d\n", mSizeX);
  fprintf(pFile, "// Image height: %d\n", mSizeY);
  fprintf(pFile, "// Image bpp:    %d\n", mBPP);
  
  s32 bytePerRow = mSizeX * mBPP / 8;

  for (s32 y = 0; y < mSizeY; ++y)
  {
    for (s32 x = 0; x < mSizeX; ++x)
    {
      for (s32 b = 0; b < mBPP / 8; ++b)
      {
        fprintf(pFile, "0x%0.2x", mpData[bytePerRow * y + x * mBPP / 8 + b]);

        if ((y == mSizeY - 1) && (x == mSizeX - 1) && (b == mBPP / 8 - 1))
          break;

        fprintf(pFile, ", ");
      }
    }

    fprintf(pFile, "\n");
  }

  fprintf(pFile, "\n");

  return true;
}

// ---------------------------------------------------------------------------
// functions implementations

u32 FrameCounter()
{
  SFW_ASSERT_MESG(spSystem, "SFW::System is not instanced yet!!");
  return spSystem->FrameCounter();
}

// ---------------------------------------------------------------------------

u32 FrameRate()
{
  SFW_ASSERT_MESG(spSystem, "SFW::System is not instanced yet!!");
  return spSystem->FrameRate();
}

// ---------------------------------------------------------------------------

f64 FrameTime()
{
  SFW_ASSERT_MESG(spSystem, "SFW::System is not instanced yet!!");
  return spSystem->FrameTime();
}

// ---------------------------------------------------------------------------

void GetHiResTime(f64* pT)
{
  static bool       firstTime = true;
  static LARGE_INTEGER  f;
  LARGE_INTEGER     c;
  
  // if this is the 1st call to the function, 
  // query the performance counter frequency
  if (firstTime)
  {
    QueryPerformanceFrequency(&f);
    firstTime = false;
  }
  
  if (f.QuadPart)
  {
    QueryPerformanceCounter(&c);

    f64 r0, r1;

    r0  = (f64)(c.QuadPart / f.QuadPart);
    r1  = (c.QuadPart % f.QuadPart) / (f64)(f.QuadPart);
    *pT = r0 + r1;
  }
  else
  {
    *pT = timeGetTime() * 0.001;
  }
}

// ---------------------------------------------------------------------------

u32 StrToId(const s8* pInput)
{
  #define DO_ONE_BIT(idx)               \
  {                         \
    bit = crc & 0x80000000;             \
    crc = (crc << 1) | ((oneChar >> idx) & 0x01); \
    if(bit)                     \
      crc ^= crcKey;                \
  }

  const u32 crcKey = 0x04C11DB7;
  const s8* pCurr  = pInput;
  u32       crc = 0;
  u32       bit = 0;

  for(; *pCurr; pCurr++)
  {
    s8 oneChar = *pCurr;

    // convert lower case to upper case
    if(('a' <= oneChar) && (oneChar <= 'z'))
      oneChar &= ~32;
    
    // bit 5 is ignored to make the calculation not case sensitive
    DO_ONE_BIT(7)
    DO_ONE_BIT(6)
    DO_ONE_BIT(5)
    DO_ONE_BIT(4)
    DO_ONE_BIT(3)
    DO_ONE_BIT(2)
    DO_ONE_BIT(1)
    DO_ONE_BIT(0)
  }

  return crc;

  #undef DO_ONE_BIT
}

// ---------------------------------------------------------------------------

} // namespace SFW
