// ---------------------------------------------------------------------------
// Project Name		:	Simple Framework
// File Name		:	OpenGLSample_Basic.h
// Author			:	Sun Tjen Fam
// Creation Date	:	2010/01/13
// Purpose			:	implementation of basic OpenGL sample
// History			:
// - 20100512: update for summer semester
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// includes

#include <iostream>
#include <fstream>
#include "SFW.h"
#include "SFW_Camera.h"
#include "SFW_Shapes.h"
#include "SFW_Shader.h"

#include "GameStates/OpenGLSample_Basic.h"

#include "Light.h"

// ---------------------------------------------------------------------------
// local definitions

#define CAM_CLIP_NEAR		0.01f
#define CAM_CLIP_FAR		800.0f
#define CAM_FOV				60.0	// fov is in degree

// toggle the shader example on/off
#define SHADER_SAMPLE		0

// ---------------------------------------------------------------------------

// anonymous namespace to make everything local to the file
namespace
{

enum
{
	CM_TEXTURE = 0, 
	CM_CONST, 
	CM_VTX_COLOR, 
	CM_NORMAL, 
	CM_TEXCOORD, 
	CM_TANGENT,
	CM_BITANGENT,
	CM_TEXSAMPLE,
	
	CM_NUM
};

enum
{
	W_REG = 0,
	W_NORM, 
	W_WIRE, 
	W_WIRENORM,
	
	W_NUM
};


// ---------------------------------------------------------------------------
// Global/Static variables

// get the current window size
f32 WinSizeX = (f32)(SFW::System::Instance()->WinSizeX());
f32 WinSizeY = (f32)(SFW::System::Instance()->WinSizeY());

// array of texture object name
static GLuint sTexId[15];

//shader programs
GLuint Program0;
GLuint Program1;

//renderbuffer and framebuffer objects
GLuint rboId, fboId;

// the currently drawn object type
static u32	            sObjType = SFW::Shape::TYPE_NUM;

// flag to toggle various things
static bool				sLightAnm = false;
static bool				sObjAnm = false;
static u32				sFlagClr = CM_TEXTURE;
static u32              sFlag = 0;

static u32              wFlag = W_REG;

// procedural animation frame counter
static u32				sFrameCtr = 0;
static u32				sFrameCtr_ = 0;

//lights and shaders
int num_lights = 1;
int fShaderObject;
int vShaderObject;
char* fShaderSource = 0;
char* vShaderSource = 0;
const int max_lights = 8;

int light_type = 0;

bool normal_map = 0;


SFW::Vec4 GlobalAmbient(0.2,0.2,0.2,1);
SFW::Vec4 Ambient(0,0,0,0);
SFW::Vec4 Diffuse(0.8,0.8,0.8,1);
SFW::Vec4 Specular(1,1,1,1);

SFW::Vec3 DAtt(1,0.1,0);

SFW::Vec3 Lights[max_lights];
SFW::Vec3 light_dirs[max_lights];

SFW::Vec3 ObjPos[SFW::Shape::TYPE_NUM];

SFW::Vec3 main_obj(0.0,0.0,0.0);
SFW::Vec3 main_obj_scale(1.0,1.0,1.0);

// simple camera
SFW::Camera				sCamera(CAM_CLIP_NEAR * 8.0, CAM_CLIP_FAR);

bool perform_shadows = true;

SFW::Mtx44 InvCam;

// ---------------------------------------------------------------------------

#if SHADER_SAMPLE
SFW::Vec3*				spGridPos;
SFW::Vec3*				spGridDir;
u32						sGridSizeX, 
						sGridSizeY;
#endif

// ---------------------------------------------------------------------------
// Static function protoypes

static void drawScene();

// ---------------------------------------------------------------------------
// Static functions implementation

void gameStateLoad(void)
{
	// =================================
	// load the texture(s) from file(s)
	// =================================
	SFW::Image* pImage[12];

	pImage[0] = SFW::Image::Load("data\\textures\\TopBottom_Diffuse.tga");
	pImage[1] = SFW::Image::Load("data\\textures\\TopBottom_Height.tga");
	pImage[2] = SFW::Image::Load("data\\textures\\CubeMap_Back.tga");
	pImage[3] = SFW::Image::Load("data\\textures\\CubeMap_Bottom.tga");
	pImage[4] = SFW::Image::Load("data\\textures\\CubeMap_Front.tga");

	//creates normal map from height map and saves the normal image
	HeightMapToNormalMap(pImage[1]);

	pImage[5] = SFW::Image::Load("data\\textures\\CubeMap_Left.tga");
	pImage[6] = SFW::Image::Load("data\\textures\\CubeMap_Right.tga");
	pImage[7] = SFW::Image::Load("data\\textures\\normal_test.tga");
	pImage[8] = SFW::Image::Load("data\\textures\\CubeMap_Top.tga");
	
	SFW_ASSERT_MESG(pImage[0] && pImage[1] && pImage[2] && pImage[3] && pImage[4] && pImage[5] &&
	                pImage[6] && pImage[7] && pImage[8]);

	// ==============================
	// load the texture(s) to OpenGL
	// ==============================

	// generate the texture names
	glGenTextures	(15, sTexId);

	//FrameBufferObject Texture
	// bind the texture name
	glBindTexture	(GL_TEXTURE_2D, sTexId[9]);
		
	// load the texture data to OpenGL
	glTexImage2D(
		GL_TEXTURE_2D, 
		0, 
		GL_RGBA, 
		1024, 
		1024,
		0,
		GL_RGBA, 
		GL_FLOAT,
		0);

		// setup the texture parameters
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//FrameBufferObject Texture
	// bind the texture name
	glBindTexture	(GL_TEXTURE_2D, sTexId[10]);
		
	// load the texture data to OpenGL
	glTexImage2D(
		GL_TEXTURE_2D, 
		0, 
		GL_RGBA, 
		1024, 
		1024,
		0,
		GL_RGBA, 
		GL_FLOAT,
		0);

			// setup the texture parameters
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//FrameBufferObject Texture
	// bind the texture name
	glBindTexture	(GL_TEXTURE_2D, sTexId[11]);
		
	// load the texture data to OpenGL
	glTexImage2D(
		GL_TEXTURE_2D, 
		0, 
		GL_RGBA, 
		1024, 
		1024,
		0,
		GL_RGBA, 
		GL_FLOAT,
		0);

	// setup the texture parameters
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//FrameBufferObject Texture
	// bind the texture name
	glBindTexture	(GL_TEXTURE_2D, sTexId[12]);
		
	// load the texture data to OpenGL
	glTexImage2D(
		GL_TEXTURE_2D, 
		0, 
		GL_RGBA, 
		1024, 
		1024,
		0,
		GL_RGBA, 
		GL_FLOAT,
		0);

			// setup the texture parameters
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//FrameBufferObject Texture
	// bind the texture name
	glBindTexture	(GL_TEXTURE_2D, sTexId[13]);
		
	// load the texture data to OpenGL
	glTexImage2D(
		GL_TEXTURE_2D, 
		0, 
		GL_RGBA, 
		1024, 
		1024,
		0,
		GL_RGBA, 
		GL_FLOAT,
		0);

	// setup the texture parameters
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//FrameBufferObject Texture
	// bind the texture name
	glBindTexture	(GL_TEXTURE_2D, sTexId[14]);
		
	// load the texture data to OpenGL
	glTexImage2D(
		GL_TEXTURE_2D, 
		0, 
		GL_RGBA, 
		1024, 
		1024,
		0,
		GL_RGBA, 
		GL_FLOAT,
		0);

		// setup the texture parameters
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//FrameBufferObject Texture
	// bind the texture name
	glBindTexture	(GL_TEXTURE_2D, sTexId[15]);
		
	// load the texture data to OpenGL
	glTexImage2D(
		GL_TEXTURE_2D, 
		0, 
		GL_RGBA, 
		1024, 
		1024,
		0,
		GL_RGBA, 
		GL_FLOAT,
		0);


	// setup the texture parameters
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
	
	for (s32 i = 0; i < 9; ++i)
	{
		// bind the texture name
		glBindTexture	(GL_TEXTURE_2D, sTexId[i]);
		
		// load the texture data to OpenGL
		// * note that TGA format store the color component in BGRA order
		glTexImage2D	(
			GL_TEXTURE_2D, 
			0, 
			(pImage[i]->BPP() == 32) ? GL_RGBA : GL_RGB, 
			pImage[i]->SizeX(), 
			pImage[i]->SizeY(), 
			0, 
			(pImage[i]->BPP() == 32) ? GL_BGRA : GL_BGR, GL_UNSIGNED_BYTE, pImage[i]->Data());

		// setup the texture parameters
		glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		// deallocate the image
		// * when texture is loaded to OpenGL, OpenGL will actually make a copy
		//   of the texture. So, the copy that you have in 'your' memory is no
		//   longer needed.
		SFW::Image::Free(pImage[i]);
		
		pImage[i] = 0;
	}

	// ===========================
	// load the 'default' shaders
	// ===========================

	// load the vertex shader and compile it
/*	if (SFW::Shader::SourceFromFile(SFW::Shader::TYPE_VERTEX, "ShadowVert", "data/shaders/shadowmap.vs") == 0)
		SFW_HALT();
	// load the fragment shader and compile it
	if (SFW::Shader::SourceFromFile(SFW::Shader::TYPE_FRAGMENT, "ShadowFrag", "data/shaders/shadowmap.fs") == 0)
		SFW_HALT();

	// create an empty program
	SFW::Shader::Program* pProgram = SFW::Shader::Program::Create("Shadow", 2);
	if (pProgram == 0)
		SFW_HALT();

	// associate the shader source code to the program
	pProgram->ShaderName(0, "ShadowVert");
	pProgram->ShaderName(1, "ShadowFrag");

	// link the shaders to create the shader program
	if (!pProgram->Link())
		SFW_HALT();
	
*/
	Program0 = InitShaders("data/shaders/reflect.vs", "data/shaders/reflect.fs");


	// ======================================
	// setup default OpenGL states or values
	// ======================================
	
	// enable back face culling
    glEnable(GL_CULL_FACE);

	// enable depth test
    glEnable(GL_DEPTH_TEST);
	
	// set the clear color and depth value
	glClearColor(0.25, 0.25, 0.25, 1.0);
	glClearDepth(1.0f);

	// ===========================
	// initialize the shape class
	// ===========================

	SFW::Shape::Init();

	//calculate tan and bitan of each shape for normal mapping
	for(u32 i = SFW::Shape::TYPE_PLANE; i < SFW::Shape::TYPE_NUM; ++i)
	  CalculateTangentAndBitanget(i);

    // create a framebuffer object
    glGenFramebuffers(1, &fboId);

	// create a renderbuffer object to store depth info
    glGenRenderbuffers(1, &rboId);
    glBindRenderbuffer(GL_RENDERBUFFER, rboId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                                        1024, 1024);
    //glBindRenderbuffer(GL_RENDERBUFFER, 0);
    

}

// ---------------------------------------------------------------------------

void gameStateInit()
{
	// reset the frame counter to 0
	sFrameCtr = 0;
	sFrameCtr_ = 0;

	// reset any runtime variable
	sCamera.Reset();
	sCamera.Dist(5.0f);
	sObjType = 4;//sphere


}

// ---------------------------------------------------------------------------

void gameStateUpdate()
{
	if (SFW::Input::Instance()->CheckTriggered('0'))
	{
	  light_type = 2;
	}
	// toggle texture mapping on/off
	if (SFW::Input::Instance()->CheckTriggered('1'))
      sObjType = (sObjType + 1) % (SFW::Shape::TYPE_NUM + 1);
	  if(sObjType == 6)
	    sObjType = 0;//plane this is a hack :(
	if (SFW::Input::Instance()->CheckTriggered('2'))
		sFlagClr = (sFlagClr + 1) % CM_NUM;
	if (SFW::Input::Instance()->CheckTriggered('3'))
	  wFlag = (wFlag + 1) % W_NUM;
	if (SFW::Input::Instance()->CheckTriggered('4'))
		sObjAnm = !sObjAnm;
	if (SFW::Input::Instance()->CheckTriggered('5'))//animation for lights
		sLightAnm = !sLightAnm;
	if (SFW::Input::Instance()->CheckTriggered('6'))//normal mapping
		normal_map = !normal_map;
	if (SFW::Input::Instance()->CheckTriggered('8'))
		perform_shadows = !perform_shadows;
	if (SFW::Input::Instance()->CheckTriggered('7'))//reflection refraction
	{
	  if(sFlag < 3)
	    ++sFlag;
	  else
	    sFlag = 0;
	}
	if (SFW::Input::Instance()->CheckTriggered('9'))//add lights
	{
	  if(num_lights)
	    num_lights = 0;
	  else
	    num_lights = 1;
	}
	
	// cycle through camera mode
	if (SFW::Input::Instance()->CheckTriggered('C'))
	  sCamera.Mode((sCamera.Mode() + 1) % SFW::Camera::MODE_NUM);

	//change x scale of main object
	if (SFW::Input::Instance()->CheckPressed('U'))
	  main_obj_scale.x += .02;
	if (SFW::Input::Instance()->CheckPressed('J'))
	  main_obj_scale.x -= .02;

	if(main_obj_scale.x < .10)
	  main_obj_scale.x = .10;

	//change y scale of main object
	if (SFW::Input::Instance()->CheckPressed('I'))
	  main_obj_scale.y += .02;
	if (SFW::Input::Instance()->CheckPressed('K'))
	  main_obj_scale.y -= .02;

	if(main_obj_scale.y < .10)
	  main_obj_scale.y = .10;

	//change z scale of main object
	if (SFW::Input::Instance()->CheckPressed('O'))
	  main_obj_scale.z += .02;
	if (SFW::Input::Instance()->CheckPressed('L'))
	  main_obj_scale.z -= .02;

    if(main_obj_scale.z < .10)
	  main_obj_scale.z = .10;

	// update the camera
	sCamera.Update();

	// increment the frame counter
	if (sLightAnm)
	  ++sFrameCtr;

	if(sObjAnm)
	  ++sFrameCtr_;


}

// ---------------------------------------------------------------------------

void gameStateDraw()
{

	// check for OpenGL error
	SFW_GL_ERROR();

	// push all current attributes to the stack
	glPushAttrib		(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib	(GL_CLIENT_ALL_ATTRIB_BITS);

	// ===============================================================
	// clear the framebuffer and depth buffer to the specified values
	// ===============================================================

	glClear				(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// =================================================
	// setup the matrix for the transformation pipeline
	// =================================================

	// get the current window size
	f32 winSizeX = (f32)(SFW::System::Instance()->WinSizeX());
	f32 winSizeY = (f32)(SFW::System::Instance()->WinSizeY());

	// set the viewport
	glViewport			(0, 0, winSizeX, winSizeY);
	
	// load the projection matrix
	glMatrixMode		(GL_PROJECTION);
    glLoadIdentity		();
	gluPerspective		(CAM_FOV, (float)(winSizeX)/(float)(winSizeY), CAM_CLIP_NEAR, CAM_CLIP_FAR);


	// load the world->view (camera) matrix onto the stack. 
	// * Later on, just before each object is rendered, push the model->world matrix of that 
	//   object and pop the model->world matrix after object rendering.
	// * internally, sCamera.LoadMatrix will call gluLookAt with the current camera position 
	//   and target.
	glMatrixMode		(GL_MODELVIEW);
	glLoadIdentity		();
	sCamera.LoadMatrix	();
	
	// check for OpenGL error
	SFW_GL_ERROR();

	// draw the scene
	drawScene			();

	// check for OpenGL error
	SFW_GL_ERROR();
	
	// restore the attribute
	glPopAttrib			();
	glPopClientAttrib	();


}

// --------------------------------------------------------------------------- 

void gameStateFree()
{
	// no need to implement as the application will quit
	// * let Windows clean up the memory for you
}

// ---------------------------------------------------------------------------

void gameStateDone()
{
	// no need to implement as the application will quit
	// * let Windows clean up the memory for you
}

// ---------------------------------------------------------------------------

void drawScene()
{

  InvCamMatrix();
  ////loadAllTextures//
  LoadTextures();

  ///////////SkyBox/////////
  DrawSkyBox(sCamera.Pos());

  SetUpCubeMap();
  DrawCubeMap();

  CreateScene(true);
  


  //check for GL Error
  SFW_GL_ERROR();
  
  glDisable(GL_TEXTURE_2D);

}

// ---------------------------------------------------------------------------

} // namespace

// ---------------------------------------------------------------------------

void GameState_OpenGLSample_Basic()
{
	extern void (*GameStateLoad)();		// load and initialize data 1st time
	extern void (*GameStateInit)();		// restart the current state
	extern void (*GameStateDraw)();		// draw game state
	extern void (*GameStateUpdate)();	// update game state
	extern void (*GameStateFree)();		// free memory
	extern void (*GameStateDone)();		// free all memory in preparation for the next state

	GameStateLoad		=	gameStateLoad;
	GameStateInit		=	gameStateInit;
	GameStateUpdate		=	gameStateUpdate;			
	GameStateDraw		=	gameStateDraw;
	GameStateFree		=	gameStateFree;
	GameStateDone		=	gameStateDone;
}

// ---------------------------------------------------------------------------

void CreateLights()
{

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  //ring animation
  glRotatef	((sFrameCtr & 0x0FF) / 255.0f * 360.0f, 0.0f, 1.0f, 0.0f);

  for(unsigned i = 0; i < num_lights; ++i)
  {
    glPushMatrix(); 
	glTranslatef(0,5,0);
	glRotatef	((360.0f/num_lights)*i, 0.0, 1.0, 0.0);
	glTranslatef(5,0,0);
	glScalef    (.5,.5,.5);

	//get the lights postition
	SFW::Mtx44 m; 
	glGetFloatv(GL_MODELVIEW_MATRIX, m.v);
	m.TransposeThis();
	Lights[i] = m * SFW::Vec4(0.0, 0.0, 0.0, 1.0);

    SFW::Shape::Draw(SFW::Shape::TYPE_SPHERE);
    glPopMatrix();
  }
  glPopMatrix();
}


u32 GetFileLength(const char* filename)
{
    FILE* file;
	file = fopen( filename, "rb" );
	// set the file pointer to end of file
	//to get file size
    fseek( file, 0, SEEK_END );
    u32 size = ftell( file );

	fclose(file);

	return size;
}

char* loadshader(const char* filename)
{   
	
	u32 size = GetFileLength(filename);

	std::ifstream infile ( filename , std::ifstream::in );

	
    //allocate our shaders memory
	char* ShaderSource = new char[size];

	
    for(unsigned i = 0; infile.good(); ++i)
	{
	  ShaderSource[i] = infile.get();
	  if(infile.eof())
	    ShaderSource[i] = 0;
	}
	  
    infile.close();

    return ShaderSource;
}


int unloadshader(const char* ShaderSource)
{
   if (ShaderSource != 0)
     delete[] ShaderSource;
   ShaderSource = 0;

   return 0;
}

void CompileShader(GLint shader)
{
  GLint compiled;
  glCompileShader(shader);

  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (compiled == GL_TRUE)
  {
    return;// it compiled
  }     
  else if(compiled == GL_FALSE)
  {

    GLsizei slen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH , &compiled);       

    GLchar* compiler_log = new GLchar[compiled];

	std::cout << "compile_failed_creating_log" << std::endl;
    glGetShaderInfoLog(shader, compiled, &slen, compiler_log);
	std::cout << "compiler_log:\n" << static_cast<char *>(compiler_log) << std::endl;
    delete [] compiler_log;
    
  }

}

void LinkShader(GLuint program)
{  
  GLint linked = 1;

  //link to program
  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (linked == GL_TRUE)
    std::cout << "link successful" << std::endl;    
  else 
  {
	GLsizei slen = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH , &linked);       

    GLchar* linker_log = new GLchar[linked];

	std::cout << "link_failed_creating_log" << std::endl;
    glGetProgramInfoLog(program, linked, &slen, linker_log);
	std::cout << "link_log:\n" << static_cast<char *>(linker_log) << std::endl;
    delete [] linker_log;
  }

}

GLuint InitShaders(const char* vs_filename, const char* fs_filename)
{

	//load both of our shaders from a file
	fShaderSource = loadshader(fs_filename);
	vShaderSource = loadshader(vs_filename);

	//create the shaders and our program
    fShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	vShaderObject = glCreateShader(GL_VERTEX_SHADER);
    GLuint Program = glCreateProgram();

	const char* fss = const_cast<const char*>(fShaderSource);
	const char* vss = const_cast<const char*>(vShaderSource);

    glShaderSource(fShaderObject, 1, &fss, NULL);
	glShaderSource(vShaderObject, 1, &vss, NULL);
    
    //compile the shaders
	CompileShader(fShaderObject);
	CompileShader(vShaderObject);


	//attach the shaders to our program	
	glAttachShader(Program, vShaderObject);
    glAttachShader(Program, fShaderObject);

    //link the shaders
	LinkShader(Program);

    //de-allocate memory for shaders
	unloadshader(fShaderSource);
    unloadshader(vShaderSource);

	return Program;

}

void HeightMapToNormalMap(SFW::Image* Height)
{ 
  //bytes per pixel
  int bpp = Height->mBPP / 8;

  //create an image to save our normal map as a texture
  SFW::Image* normal_image = SFW::Image::Create(Height->mSizeX,Height->mSizeY,Height->mBPP);
  //our tangents we will use to find the normal
  SFW::Vec3 S , T;
  //our normal vector
  SFW::Vec3 Normal;

  int Width = Height->mSizeX * bpp;

  //extra variables to wrap in case we go out of
  //bounds
  u32 i1 = 0;
  u32 j1 = 0;
  u32 i2 = 0;
  u32 j2 = 0;

  //loop through our height array
  for(u32 i = 0; i < Height->mSizeY; ++i)
  {
    for(u32 j = 0; j < Width; j+=bpp)
	{

	  //increment wrap count variables
	  j1 = j + bpp;
	  j2 = j - bpp;
	  i1 = i + 1;
	  i2 = i - 1;

	  //make sure we dont go out of bounds
	  //wrap
	  if(j1 > Width - bpp)
	    j1 = 0;
	  else if(j1 < 0)
	    j1 = Width - bpp;
	  if(j2 < 0)
	    j2 = Width - bpp;
	  else if(j2 > Width - bpp)
	    j2 = 0;
	  if(i1 > Height->mSizeY - 1)
	    i1 = 0;
	  else if(i1 < 0)
	    i1 = Height->mSizeY - 1;
	  if(i2 < 0)
	    i2 = Height->mSizeY - 1;
	  else if(i2 > Height->mSizeY - 1)
	    i2 = 0;

      //get our two tangents from the height map
	  S = SFW::Vec3(1,0,(Height->mpData[j1 + (i * Width)]) - (Height->mpData[ j2 + (i * Width)]));
	  T = SFW::Vec3(0,1,(Height->mpData[j + (i1 * Width)]) - (Height->mpData[j + (i2 * Width)]));

	  //[-1.0,1.0]
	  S.z = (S.z / 255.0) * 3;//3 is hardcoded scale
	  T.z = (T.z / 255.0) * 3;

	  //calculate our normal from our two tangents
	  Normal = S.Cross(T);
	  Normal.NormalizeThis();

	  
	  //store our normal_map as as a texture rgb but tga so bgr
	  normal_image->mpData[(j + (i * Width)) + 0] = 255.0 * 0.5 * (Normal.z + 1); //blue
	  normal_image->mpData[(j + (i * Width)) + 1] = 255.0 * 0.5 * (Normal.y + 1); //green
	  normal_image->mpData[(j + (i * Width)) + 2] = 255.0 * 0.5 * (Normal.x + 1); //red
	 
	}

  }

  //save our normal image
  normal_image->Save("data\\textures\\normal_test.tga");
}


void CalculateTangentAndBitanget(u32 shape)
{

  //create an array of ints to see how many times  
  //a vertex is visited used for averaging later
  u32 *vtx_count = new u32[SFW::Shape::VtxNum(shape)];
  //set vtx_count to 0
  for(u32 i=0; i < SFW::Shape::VtxNum(shape); ++i)
    vtx_count[i] = 0;

  //used for tan and bitan caluclations
  SFW::Vec3 P, Q;
  f32 S1,S2,T1,T2;

  //get the triangle list that composes our shape
  SFW::Shape::Tri* tri_list = SFW::Shape::TriList(shape);
  //get the list of vertexes
  SFW::Shape::Vtx* vtx_list = SFW::Shape::VtxList(shape);

  //set vtx_list to 0
  for(u32 i=0; i < SFW::Shape::VtxNum(shape); ++i)
  {
	vtx_list[i].bin = SFW::Vec3(0,0,0);
	vtx_list[i].tan = SFW::Vec3(0,0,0);
  }



  //calculate the tangent and bitangent of each triangle
  for(u32 i = 0; i < SFW::Shape::TriNum(shape); ++i)
  {
    P = vtx_list[tri_list[i].p1].pos - vtx_list[tri_list[i].p0].pos;
	Q = vtx_list[tri_list[i].p2].pos - vtx_list[tri_list[i].p0].pos;

    S1 = vtx_list[tri_list[i].p1].tex.s - vtx_list[tri_list[i].p0].tex.s;
	S2 = vtx_list[tri_list[i].p2].tex.s - vtx_list[tri_list[i].p0].tex.s;
	T1 = vtx_list[tri_list[i].p1].tex.t - vtx_list[tri_list[i].p0].tex.t;
	T2 = vtx_list[tri_list[i].p2].tex.t - vtx_list[tri_list[i].p0].tex.t;
    
	f32 denominator = (S2*T1-T2*S1);
	if(denominator != 0) //avoid divide by 0 
	{
	  //bitangent
	  vtx_list[tri_list[i].p0].bin += ((S2*P-S1*Q)/denominator);
      vtx_list[tri_list[i].p1].bin += ((S2*P-S1*Q)/denominator);
	  vtx_list[tri_list[i].p2].bin += ((S2*P-S1*Q)/denominator);
	  //tangent
	  vtx_list[tri_list[i].p0].tan += ((Q*T1-P*T2)/denominator);
	  vtx_list[tri_list[i].p1].tan += ((Q*T1-P*T2)/denominator);
	  vtx_list[tri_list[i].p2].tan += ((Q*T1-P*T2)/denominator);
	}
	
	//add to the count
	++vtx_count[tri_list[i].p0];
	++vtx_count[tri_list[i].p1];
	++vtx_count[tri_list[i].p2];
  
  }

  //average tan and bitan to get our final result
  for(u32 i = 0; i < SFW::Shape::VtxNum(shape); ++i)
  {
    //bitangent
    vtx_list[i].bin /= vtx_count[i];
	
	//tangent
	vtx_list[i].tan /= vtx_count[i];
  }

  //done with the vtx count
  delete [] vtx_count;

}

void CreateShapeRing()
{

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  //ring animation
  glRotatef	((sFrameCtr & 0x0FF) / 255.0f * 360.0f, 0.0f, 1.0f, 0.0f);

  for(unsigned i = 0; i < SFW::Shape::TYPE_NUM; ++i)
  {
    glPushMatrix(); 
	glRotatef	((360.0f/SFW::Shape::TYPE_NUM)*i, 0.0, 1.0, 0.0);
	glTranslatef(6,0,0);
	glScalef    (1,1,1);

	//get the Objects postitions
	SFW::Mtx44 m; 
	glGetFloatv(GL_MODELVIEW_MATRIX, m.v);
	m.TransposeThis();
	ObjPos[i] = m * SFW::Vec4(0.0, 0.0, 0.0, 1.0);

    SFW::Shape::Draw(i);
    glPopMatrix();
  }
  glPopMatrix();
}

void DrawSkyBox(SFW::Vec3 CamPos)
{

	glMatrixMode(GL_MODELVIEW);
   	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

   	glPushMatrix();
	glTranslatef(CamPos.x,CamPos.y,CamPos.z - .5);
	glScalef(1,1,1);
	glBindTexture (GL_TEXTURE_2D, sTexId[2]);
	SFW::Shape::Draw(SFW::Shape::TYPE_PLANE);
	glPopMatrix();

	glPushMatrix();
	
	glTranslatef(CamPos.x,CamPos.y,CamPos.z + .5);
	glScalef(1,1,1);
	glRotatef(180, 0, 1, 0);
	glBindTexture (GL_TEXTURE_2D, sTexId[4]);
	SFW::Shape::Draw(SFW::Shape::TYPE_PLANE);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(CamPos.x,CamPos.y - .5,CamPos.z);
	glScalef(1,1,1);
	glRotatef(-90, 1, 0, 0);
	glBindTexture (GL_TEXTURE_2D, sTexId[3]);
	SFW::Shape::Draw(SFW::Shape::TYPE_PLANE);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(CamPos.x,CamPos.y + .5,CamPos.z);
	glScalef(1,1,1);
	glRotatef(90, 1, 0, 0);
	glBindTexture (GL_TEXTURE_2D, sTexId[8]);
	SFW::Shape::Draw(SFW::Shape::TYPE_PLANE);
	glPopMatrix();

		glPushMatrix();
	glTranslatef(CamPos.x - .5,CamPos.y,CamPos.z);
	glScalef(1,1,1);
	glRotatef(90, 0, 1, 0);
	glBindTexture (GL_TEXTURE_2D, sTexId[5]);
	SFW::Shape::Draw(SFW::Shape::TYPE_PLANE);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(CamPos.x + .5,CamPos.y,CamPos.z);
	glScalef(1,1,1);
	glRotatef(-90, 0, 1, 0);
	glBindTexture (GL_TEXTURE_2D, sTexId[6]);
	SFW::Shape::Draw(SFW::Shape::TYPE_PLANE);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

}

void LoadTextures()
{

   //right now only loading first texture
   for(u32 i = 0; i < 1; ++i)
   {
     glActiveTexture(GL_TEXTURE0 + i);
	 glEnable		(GL_TEXTURE_2D);
	 glBindTexture	(GL_TEXTURE_2D, sTexId[i]);
	 glMatrixMode(GL_TEXTURE);
	 glLoadIdentity();
	 SFW_GL_ERROR();
   }
	
	glActiveTexture	(GL_TEXTURE0);
}
void CreateScene(bool draw_main)
{
  //bind TopBottom diffuse texture//////////////
  glBindTexture (GL_TEXTURE_2D, sTexId[0]);
  CreateShapeRing();

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  if(sObjAnm)
  {
    //main_obj animation
    glRotatef((sFrameCtr_ & 0x0FF) / 255.0f * 360.0f, 1.0f, 0.0f, 0.0f);
	glRotatef((sFrameCtr_ & 0x0FF) / 255.0f * 360.0f, 0.0f, 1.0f, 0.0f);
	glRotatef((sFrameCtr_ & 0x0FF) / 255.0f * 360.0f, 0.0f, 0.0f, 1.0f);
	main_obj.y = sin(((sFrameCtr_ & 0x0FF) / 255.0f * 360.0f) * (PI/180.0f));
  }

  glTranslatef(main_obj.x,main_obj.y,main_obj.z);
  glScalef(main_obj_scale.x,main_obj_scale.y,main_obj_scale.z);

  //draw shape
  if(draw_main)
  {
    if(Program0)
      glUseProgram(Program0);

	//pass in uniforms to program
	glUniformMatrix4fv(glGetUniformLocation(Program0, "InvCam")  ,1, GL_FALSE, InvCam.v);
	glUniform1i(glGetUniformLocation(Program0, "cube_back_tex")  ,14);
	glUniform1i(glGetUniformLocation(Program0, "cube_front_tex") ,13);
	glUniform1i(glGetUniformLocation(Program0, "cube_bottom_tex"),11);
	glUniform1i(glGetUniformLocation(Program0, "cube_top_tex")   ,12);
	glUniform1i(glGetUniformLocation(Program0, "cube_left_tex")  ,9);
	glUniform1i(glGetUniformLocation(Program0, "cube_right_tex") ,10);
	glUniform1i(glGetUniformLocation(Program0, "normal_tex")     ,7);
	glUniform1i(glGetUniformLocation(Program0, "uFlag"),  sFlag);
	glUniform1i(glGetUniformLocation(Program0, "normal_map"),  normal_map);
	
	//id's for normal mapping attributes
	GLuint id1, id2;

	if(normal_map)
	{
	  //get tangent and bitangent for normal mapping
	  glActiveTexture	(GL_TEXTURE7);
	  glEnable		(GL_TEXTURE_2D);
	  glBindTexture	(GL_TEXTURE_2D, sTexId[7]);
	  glMatrixMode	(GL_TEXTURE);
	  glLoadIdentity	();
	  glScalef        (32.0f,32.0f,32.0f);

	  
      id1 = glGetAttribLocation(Program0, "Tan");
	  glEnableVertexAttribArray(id1);
	  glVertexAttribPointer(id1, 3, GL_FLOAT, GL_FALSE, sizeof(SFW::Shape::Vtx), &SFW::Shape::VtxList(sObjType)->tan);
	
	  id2 = glGetAttribLocation(Program0, "BiTan");
	  glEnableVertexAttribArray(id2);
	  glVertexAttribPointer(id2, 3, GL_FLOAT, GL_FALSE, sizeof(SFW::Shape::Vtx), &SFW::Shape::VtxList(sObjType)->bin);
	}

	glActiveTexture(GL_TEXTURE9);
	glBindTexture (GL_TEXTURE_2D, sTexId[9]);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture (GL_TEXTURE_2D, sTexId[10]);
	glActiveTexture(GL_TEXTURE11);
	glBindTexture (GL_TEXTURE_2D, sTexId[11]);
	glActiveTexture(GL_TEXTURE12);
	glBindTexture (GL_TEXTURE_2D, sTexId[12]);
	glActiveTexture(GL_TEXTURE13);
	glBindTexture (GL_TEXTURE_2D, sTexId[13]);
	glActiveTexture(GL_TEXTURE14);
	glBindTexture (GL_TEXTURE_2D, sTexId[14]);

	glMatrixMode(GL_MODELVIEW);

    if(wFlag == W_REG)
      SFW::Shape::Draw(sObjType);
    else if(wFlag == W_NORM)
    {
      SFW::Shape::Draw(sObjType);
	  SFW::Shape::DrawNormal(sObjType);
    }
    else if(wFlag == W_WIRE)
      SFW::Shape::DrawWireFrame(sObjType);
    else if(wFlag == W_WIRENORM)
    {	
      SFW::Shape::DrawNormal(sObjType);
	  SFW::Shape::DrawWireFrame(sObjType);
    }

	glUseProgram(0);  

	if(normal_map)
	{
	  glDisableVertexAttribArray(id1);
	  glDisableVertexAttribArray(id2);
	}

  }

  glPopMatrix();



}

void SetUpCubeMap()
{
  	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                              GL_RENDERBUFFER, rboId);
	
	// attach the texture to FBO color attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                               GL_TEXTURE_2D, sTexId[9], 0);
    
	glPushAttrib(GL_VIEWPORT_BIT);
	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 	
	glViewport(0,0,1024,1024);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(90.0f,1.0f,.01f,70.0f);

    //left
	glMatrixMode		(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity		();
	gluLookAt(main_obj.x, main_obj.y, main_obj.z, main_obj.x - 1, main_obj.y, main_obj.z, 0, 1, 0);
    DrawSkyBox(main_obj);
	CreateScene();


    
	//right
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                           GL_TEXTURE_2D, sTexId[10], 0);
	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode		(GL_MODELVIEW);
	glLoadIdentity		();
	gluLookAt(main_obj.x, main_obj.y, main_obj.z, main_obj.x + 1, main_obj.y, main_obj.z, 0, 1, 0);
    DrawSkyBox(main_obj);
	CreateScene();	


	//bottom
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                           GL_TEXTURE_2D, sTexId[11], 0);
	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode		(GL_MODELVIEW);
	glLoadIdentity		();
	gluLookAt(main_obj.x, main_obj.y, main_obj.z, main_obj.x, main_obj.y - 1, main_obj.z, 0, 0, -1);
    DrawSkyBox(main_obj);
	CreateScene();	



	//top
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                           GL_TEXTURE_2D, sTexId[12], 0);
	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode		(GL_MODELVIEW);
	glLoadIdentity		();
	gluLookAt(main_obj.x, main_obj.y, main_obj.z, main_obj.x, main_obj.y + 1, main_obj.z, 0, 0, 1);
    DrawSkyBox(main_obj);
	CreateScene();	


	//front
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                           GL_TEXTURE_2D, sTexId[13], 0);
	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode		(GL_MODELVIEW);
	glLoadIdentity		();
	gluLookAt(main_obj.x, main_obj.y, main_obj.z, main_obj.x, main_obj.y, main_obj.z + 1, 0, 1, 0);
    DrawSkyBox(main_obj);
	CreateScene();	


	//back
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                           GL_TEXTURE_2D, sTexId[14], 0);
	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode		(GL_MODELVIEW);
	glLoadIdentity		();
	gluLookAt(main_obj.x, main_obj.y, main_obj.z, main_obj.x, main_obj.y, main_obj.z - 1, 0, 1, 0);
    DrawSkyBox(main_obj);
	CreateScene();	

	glPopMatrix();


    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
	  std::cout << "frame_buffer fail" << std::endl;
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();


	glPopAttrib();
	// switch back to window-system-provided framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DrawCubeMap()
{
  
	glMatrixMode		(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity		();
	
	////draw quad
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(-.8,-.8,0);
	glScalef(WinSizeY/WinSizeX, 1,0);
	glScalef(.3,.3,0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, sTexId[9]);
    glBegin (GL_QUADS);
    glTexCoord2f (1.0, 1.0);
    glVertex3f (.5, .5, 0.0);
    glTexCoord2f (0.0, 1.0);
    glVertex3f (-.5, 0.5, 0.0);
    glTexCoord2f (0.0, 0.0);
    glVertex3f (-.5, -.5, 0.0);
    glTexCoord2f (1.0, 0.0);
    glVertex3f (0.5, -.5, 0.0);
    glEnd ();
	glPopMatrix();

	////draw quad
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(-.5,-.8,0);
	glScalef(WinSizeY/WinSizeX, 1,0);
	glScalef(.3,.3,0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, sTexId[10]);
    glBegin (GL_QUADS);
    glTexCoord2f (1.0, 1.0);
    glVertex3f (.5, .5, 0.0);
    glTexCoord2f (0.0, 1.0);
    glVertex3f (-.5, 0.5, 0.0);
    glTexCoord2f (0.0, 0.0);
    glVertex3f (-.5, -.5, 0.0);
    glTexCoord2f (1.0, 0.0);
    glVertex3f (0.5, -.5, 0.0);
    glEnd ();
	glPopMatrix();

	////draw quad
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(-.2,-.8,0);
	glScalef(WinSizeY/WinSizeX, 1,0);
	glScalef(.3,.3,0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, sTexId[11]);
    glBegin (GL_QUADS);
    glTexCoord2f (1.0, 1.0);
    glVertex3f (.5, .5, 0.0);
    glTexCoord2f (0.0, 1.0);
    glVertex3f (-.5, 0.5, 0.0);
    glTexCoord2f (0.0, 0.0);
    glVertex3f (-.5, -.5, 0.0);
    glTexCoord2f (1.0, 0.0);
    glVertex3f (0.5, -.5, 0.0);
    glEnd ();
	glPopMatrix();

	////draw quad
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(.1,-.80,0);
	glScalef(WinSizeY/WinSizeX, 1,0);
	glScalef(.3,.3,0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, sTexId[12]);
    glBegin (GL_QUADS);
    glTexCoord2f (1.0, 1.0);
    glVertex3f (.5, .5, 0.0);
    glTexCoord2f (0.0, 1.0);
    glVertex3f (-.5, 0.5, 0.0);
    glTexCoord2f (0.0, 0.0);
    glVertex3f (-.5, -.5, 0.0);
    glTexCoord2f (1.0, 0.0);
    glVertex3f (0.5, -.5, 0.0);
    glEnd ();
	glPopMatrix();

	//draw quad
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(.4,-.8,0);
	glScalef(WinSizeY/WinSizeX, 1,0);
	glScalef(.3,.3,0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, sTexId[13]);
    glBegin (GL_QUADS);
    glTexCoord2f (1.0, 1.0);
    glVertex3f (.5, .5, 0.0);
    glTexCoord2f (0.0, 1.0);
    glVertex3f (-.5, 0.5, 0.0);
    glTexCoord2f (0.0, 0.0);
    glVertex3f (-.5, -.5, 0.0);
    glTexCoord2f (1.0, 0.0);
    glVertex3f (0.5, -.5, 0.0);
    glEnd ();
	glPopMatrix();

	////draw quad
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(.7,-.8,0);
	glScalef(WinSizeY/WinSizeX, 1,0);
	glScalef(.3,.3,0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, sTexId[14]);
    glBegin (GL_QUADS);
    glTexCoord2f (1.0, 1.0);
    glVertex3f (.5, .5, 0.0);
    glTexCoord2f (0.0, 1.0);
    glVertex3f (-.5, 0.5, 0.0);
    glTexCoord2f (0.0, 0.0);
    glVertex3f (-.5, -.5, 0.0);
    glTexCoord2f (1.0, 0.0);
    glVertex3f (0.5, -.5, 0.0);
    glEnd ();
	glPopMatrix();

	glMatrixMode (GL_MODELVIEW);
	glPopMatrix();
}

void InvCamMatrix()
{
    //create camera inverse matrix
	//so we can get out of modelview in vs
	//and go into light space
	SFW::Vec3 u   = sCamera.Up();
	SFW::Vec3 v   = sCamera.Dir();	
	SFW::Vec3 r   = v.Cross(u.Normalize()).Normalize();
	SFW::Vec3 eye = sCamera.Pos();

     SFW::Mtx44 InverseOrient(  
         r.x, r.y, r.z, 0.0f,  
         u.x, u.y, u.z, 0.0f,  
         -v.x, -v.y, -v.z, 0.0f,  
         0.0f, 0.0f, 0.0f, 1.0f  
     );  
    InverseOrient.TransposeThis();
     SFW::Mtx44 InverseTranslation(  
         1.0f, 0.0f, 0.0f, 0.0f,  
         0.0f, 1.0f, 0.0f, 0.0f,  
         0.0f, 0.0f, 1.0f, 0.0f,  
         eye.x, eye.y, eye.z, 1.0f  
     );  
    InverseTranslation.TransposeThis();

    InvCam = InverseTranslation * InverseOrient ;  
	InvCam.TransposeThis();

}