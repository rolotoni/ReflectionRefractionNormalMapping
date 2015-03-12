// ---------------------------------------------------------------------------
// Project Name   : Simple Framework
// File Name    : main.cpp
// Author     : Sun Tjen Fam
// Creation Date  : 2008/09/08
// Purpose      : main for the simple framework
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// includes

#include "SFW.h"
#include "SFW_Camera.h"
#include "SFW_Shapes.h"
#include "SFW_Shader.h"
#include "SFW_Mesh.h"

#include "GameStates/OpenGLSample_Basic.h"
#include "GameStates/OpenGLSample_RenderToTexture.h"

// ---------------------------------------------------------------------------
// local definitions

#define TIME_HIST_SIZE  64

// ---------------------------------------------------------------------------
// Global/Static variables

// game state function pointers
void (*GameStateLoad)();  // load and initialize data 1st time
void (*GameStateInit)();  // initialization at the beginning of the state
void (*GameStateDraw)();  // draw game state
void (*GameStateUpdate)();  // update game state
void (*GameStateFree)();  // free memory
void (*GameStateDone)();  // free all memory in preparation for the next state

// ---------------------------------------------------------------------------

static SFW::System*   spSystem;
static SFW::Input*    spInput;

// ---------------------------------------------------------------------------
// Static function protoypes

// ---------------------------------------------------------------------------
// Functions implementations

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR command_line, int show)
{
  // get pointer to the system and input instance
  spSystem  = SFW::System::Instance();
  spInput   = SFW::Input::Instance();
  assert(spSystem && spInput);

  // create a window and setup OpenGL context
  if (spSystem->InitWindow(hInst, "Jesse_Harrison-Assignment_4", 800, 600) == false)
    SFW_HALT("Cannot create window!!");

  // initialize a console
  if (spSystem->InitConsole("Jesse_Harrison-Assignment_4") == false)
    SFW_HALT("Cannot create console!!");

  // setup the gamestate function pointers
  GameState_OpenGLSample_Basic();
  //GameState_OpenGLSample_RenderToTexture();
  
  GameStateLoad();
  GameStateInit();
  
  f64 timeHist[TIME_HIST_SIZE];
  u32 timeHistIdx = 0;
  memset(timeHist, 0, sizeof(f64) * TIME_HIST_SIZE);
  
  while (spSystem->WinExists())
  {
    // tell the system that a new frame is starting
    spSystem->FrameStart();
    
    // update the input
    spInput->Update();

    // do game update
    GameStateUpdate();

    // do game draw
    GameStateDraw();
    
    // tell system that frame is done
    spSystem->FrameEnd();

    // check if need to quit the application
    if (spInput->CheckTriggered(VK_ESCAPE))
      break;

    // store the current CPU time to the history list
    timeHist[timeHistIdx = (timeHistIdx + 1) % TIME_HIST_SIZE] = spSystem->FrameTimeCPU();

    // calculate the average
    f64 timeAvg = 0.0;
    for (s32 i = 0; i < TIME_HIST_SIZE; i++)
      timeAvg += timeHist[i] / TIME_HIST_SIZE;
    
    // show the data on the window's title
    char temp[1024];
    sprintf(temp, "Jesse_Harrison-Assignment_1 - CPU/GPU Usage: %6.4f (%5.1f%%) | Average: %6.4f (%5.1f%%)", 
      spSystem->FrameTimeCPU(), 100.0 * spSystem->FrameTimeCPU() / spSystem->FrameTimeMin(), 
      timeAvg,                  100.0 * timeAvg / spSystem->FrameTimeMin());
    SFW::System::Instance()->WinText(temp);
  }

  // free the game data
  GameStateFree();
  GameStateDone();
  
  spSystem->Free();
  spInput->Free();

  return 0;
}

// ---------------------------------------------------------------------------