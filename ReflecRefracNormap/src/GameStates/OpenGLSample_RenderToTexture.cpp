// ---------------------------------------------------------------------------
// Project Name		:	Simple Framework
// File Name		:	OpenGLSample_RenderToTexture.h
// Author			:	Sun Tjen Fam
// Creation Date	:	2010/03/16
// Purpose			:	implementation of basic OpenGL sample for render to 
//                      texture
// ---------------------------------------------------------------------------
// includes

#include "SFW.h"
#include "SFW_Camera.h"
#include "SFW_Shapes.h"
#include "SFW_Shader.h"

#include "GameStates/OpenGLSample_RenderToTexture.h"

// ---------------------------------------------------------------------------
// local definitions

#define CAM_CLIP_NEAR		0.01f
#define CAM_CLIP_FAR		100.0f
#define CAM_FOV				60.0

// enable/disable sample code for FBO/TexCopy
// * SAMPLE_FBO takes priority, i.e. if both are set to 1, the code will do FBO
// * FBO method is faster (if your hardware can do it)
// * glCopyTexImage2D method is easier to understand
#define SAMPLE_FBO		0
#define SAMPLE_TEX_COPY	1

// * SAMPLE_FBO:      the size of the framebuffer object.
// * SAMPLE_TEX_COPY: the area of frame buffer to be copied (should be less than the window size)
// * size should be a power of 2
#define DST_MAP_SIZE_X	512
#define DST_MAP_SIZE_Y	512

// ---------------------------------------------------------------------------
// Global/Static variables

extern void (*GameStateLoad)();		// load and initialize data 1st time
extern void (*GameStateInit)();		// restart the current state
extern void (*GameStateDraw)();		// draw game state
extern void (*GameStateUpdate)();	// update game state
extern void (*GameStateFree)();		// free memory
extern void (*GameStateDone)();		// free all memory in preparation for the next state

static SFW::System*		spSystem;
static SFW::Input*		spInput;

// array of texture object name
static GLuint			sTexId[2];

// the current shape being rendered
static u32				sShapeIdx = SFW::Shape::TYPE_CUBE;

static bool				sFlagTex = true;
static bool				sFlagNrm = false;
static bool				sFlagAnm = false;

// instance of a camera
static SFW::Camera		sCamera(CAM_CLIP_NEAR, CAM_CLIP_FAR);

// simple frame counter to keep track the procedural animation
static u32				sFrameCtr = 0;
static u32				sFrameCtr_ = 0;

// ---------------------------------------------------------------------------

#if SAMPLE_FBO

// frame buffer object
GLuint					sFBO;
// render buffer object (for frame buffer's depth)
GLuint					sRBO;
// color buffer object (for frame buffer's color). Could be multiple of this.
GLuint					sCBO;

#elif SAMPLE_TEX_COPY

// name of texture object used to store the copied frame buffer
GLuint					sTexCopyId;

#endif // SAMPLE_TEX_COPY

// ---------------------------------------------------------------------------
// Static function protoypes

static void	gameStateLoad();
static void	gameStateInit();
static void	gameStateUpdate();
static void	gameStateDraw();
static void	gameStateFree();
static void	gameStateDone();

static void drawScene();

// ---------------------------------------------------------------------------

void GameState_OpenGLSample_RenderToTexture()
{
	GameStateLoad		=	gameStateLoad;
	GameStateInit		=	gameStateInit;
	GameStateUpdate		=	gameStateUpdate;
	GameStateDraw		=	gameStateDraw;
	GameStateFree		=	gameStateFree;
	GameStateDone		=	gameStateDone;
}

// ---------------------------------------------------------------------------
// Static functions implementation

void gameStateLoad(void)
{
	// ==================
	// load the textures
	// ==================

	// generate the texture names
	glGenTextures	(2, sTexId);

	// load the test texture 1
	SFW::Image* pImage = SFW::Image::Load("data\\textures\\Test1.tga");
	SFW_ASSERT_MESG(pImage);

	// load texture 1 to OpenGL
	glBindTexture	(GL_TEXTURE_2D, sTexId[0]);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D	(
		GL_TEXTURE_2D, 
		0, 
		(pImage->BPP() == 32) ? GL_RGBA : GL_RGB, 
		pImage->SizeX(), 
		pImage->SizeY(), 
		0, 
		(pImage->BPP() == 32) ? GL_BGRA : GL_BGR, GL_UNSIGNED_BYTE, pImage->Data());

	// deallocate the image
	SFW::Image::Free(pImage);

	// load the test texture 2
	pImage = SFW::Image::Load("data\\textures\\Test2.tga");
	SFW_ASSERT_MESG(pImage);

	// load texture 2 to OpenGL
	glBindTexture	(GL_TEXTURE_2D, sTexId[1]);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D	(
		GL_TEXTURE_2D, 
		0, 
		(pImage->BPP() == 32) ? GL_RGBA : GL_RGB, 
		pImage->SizeX(), 
		pImage->SizeY(), 
		0, 
		(pImage->BPP() == 32) ? GL_BGRA : GL_BGR, GL_UNSIGNED_BYTE, pImage->Data());

	SFW::Image::Free(pImage);

	// ===========================
	// load the 'default' shaders
	// ===========================

	if (SFW::Shader::SourceFromFile(SFW::Shader::TYPE_VERTEX, "DefaultVert", "data/shaders/default.vs") == 0)
		SFW_HALT();
	if (SFW::Shader::SourceFromFile(SFW::Shader::TYPE_FRAGMENT, "DefaultFrag", "data/shaders/default.fs") == 0)
		SFW_HALT();

	SFW::Shader::Program* pProgram = SFW::Shader::Program::Create("Default", 2);

	if (pProgram == 0)
		SFW_HALT();

	pProgram->ShaderName(0, "DefaultVert");
	pProgram->ShaderName(1, "DefaultFrag");
	if (!pProgram->Link())
		SFW_HALT();

	// ===========================
	// initialize the Shape class
	// ===========================

	SFW::Shape::Init(32);

#if SAMPLE_FBO

	// ==============================
	// setup the frame buffer object
	// ==============================

	// generate a frame buffer object
	glGenFramebuffersEXT			(1, &sFBO);
	glBindFramebufferEXT			(GL_FRAMEBUFFER_EXT, sFBO);

	// generate a render buffer object and allocate memory for it
	glGenRenderbuffersEXT			(1, &sRBO);
	glBindRenderbufferEXT			(GL_RENDERBUFFER_EXT, sRBO);
	glRenderbufferStorageEXT		(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, DST_MAP_SIZE_X, DST_MAP_SIZE_Y);
	
	// attach the depth buffer to the frame buffer
	glFramebufferRenderbufferEXT	(GL_FRAMEBUFFER_EXT,  GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, sRBO);
	
	// generate the color buffer object and allocate memory for it
	glGenTextures					(1, &sCBO);
	glBindTexture					(GL_TEXTURE_2D, sCBO);
	glTexParameteri					(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri					(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri					(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri					(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D					(GL_TEXTURE_2D, 0, GL_RGB, DST_MAP_SIZE_X, DST_MAP_SIZE_Y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	
	// attach the color buffer to the frame buffer
	glFramebufferTexture2DEXT		(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, sCBO, 0);
	
	// check if the frame buffer object is created succesfully
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	assert(status == GL_FRAMEBUFFER_COMPLETE_EXT);
	
	// do not use it by default
	glBindFramebufferEXT		(GL_FRAMEBUFFER_EXT, 0);
	glBindTexture				(GL_TEXTURE_2D, 0);

#elif SAMPLE_TEX_COPY

	// ===============================================
	// setup a texture to store the frame buffer copy
	// ===============================================

	// generate a texture object to store the copied framebuffer
	glBindTexture	(GL_TEXTURE_2D, sTexCopyId);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri	(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D	(GL_TEXTURE_2D, 0, GL_RGB, DST_MAP_SIZE_X, DST_MAP_SIZE_Y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

#endif // SAMPLE_TEX_COPY

	// ============================
	// setup default OpenGL states or values
	// ============================
	
	// enable/disable states
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

	// set the clear color and depth value
	glClearColor		(0.25, 0.25, 0.25, 1.0);
	glClearDepth		(1.0f);
}

// ---------------------------------------------------------------------------

void gameStateInit()
{
	sCamera.Reset();
	sCamera.Dist(5.0f);

	sFrameCtr = 0.0f;
	sFrameCtr_= 0.0f;
}

// ---------------------------------------------------------------------------

void gameStateUpdate()
{
	if (SFW::Input::Instance()->CheckTriggered('1'))
		sShapeIdx = (sShapeIdx + 1) % SFW::Shape::TYPE_NUM;
	if (SFW::Input::Instance()->CheckTriggered('2'))
		sFlagTex = !sFlagTex;
	if (SFW::Input::Instance()->CheckTriggered('3'))
		sFlagNrm = !sFlagNrm;
	if (SFW::Input::Instance()->CheckTriggered('4'))
		sFlagAnm = !sFlagAnm;

	// cycle through camera mode
	if (SFW::Input::Instance()->CheckTriggered('C'))
		sCamera.Mode((sCamera.Mode() + 1) % SFW::Camera::MODE_NUM);
	
	// update the camera
	sCamera.Update();

	// update the current frame
	sFrameCtr += (u32)(sFlagAnm);
}

// ---------------------------------------------------------------------------

void gameStateDraw()
{
	// check for OpenGL error
	SFW_GL_ERROR();

	// push all current attributes to the stack
	glPushAttrib		(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib	(GL_CLIENT_ALL_ATTRIB_BITS);

#if SAMPLE_FBO || SAMPLE_TEX_COPY

	// check for OpenGL error
	SFW_GL_ERROR();

#if SAMPLE_FBO
	// bind the frame buffer object
	// * any drawing from this point on will be drawn to this frame buffer
	glBindFramebufferEXT	(GL_FRAMEBUFFER_EXT, sFBO);
	glDrawBuffer			(GL_COLOR_ATTACHMENT0_EXT);
#endif // SAMPLE_FBO

	// setup the viewport
	// * mstch it with the frame buffer size
	glViewport				(0, 0, DST_MAP_SIZE_X, DST_MAP_SIZE_Y);
	glClearColor			(0.0f, 0.0f, 0.0f, 0.0f);
	glClear					(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode			(GL_PROJECTION);
	glPushMatrix			();
	glMatrixMode			(GL_MODELVIEW);
	glPushMatrix			();

	// load the projection matrix
	glMatrixMode			(GL_PROJECTION);
    glLoadIdentity			();
	gluPerspective			(90.0f, (float)(DST_MAP_SIZE_X)/(float)(DST_MAP_SIZE_Y), CAM_CLIP_NEAR, CAM_CLIP_FAR);

	// check for OpenGL error
	SFW_GL_ERROR();
	
	// load the model -> view matrix
	// * animate the camera around the origin and have it look at the object center
	glMatrixMode		(GL_MODELVIEW);
	glLoadIdentity		();
	gluLookAt			(
		2.0f * sinf((SFW::FrameCounter() & 0x03FF) * TWO_PIf / 1023.0f), 
		1.0f * sinf((SFW::FrameCounter() & 0x03FF) * TWO_PIf / 1023.0f), 
		2.0f * cosf((SFW::FrameCounter() & 0x03FF) * TWO_PIf / 1023.0f), 
		0.0f, 
		0.0f, 
		0.0f,
		0.0f, 1.0f, 0.0f);
	
	// check for OpenGL error
	SFW_GL_ERROR();

	// draw the scene
	drawScene			();

	glMatrixMode		(GL_PROJECTION);
	glPopMatrix			();
	glMatrixMode		(GL_MODELVIEW);
	glPopMatrix			();

#if SAMPLE_FBO

	// unbind the framebuffer
	// * from this point on, any drawing is done back to the usual back buffer
	glBindFramebufferEXT	(GL_FRAMEBUFFER_EXT, 0);
	glDrawBuffer			(GL_BACK_LEFT);

#else // SAMPLE_FBO

	// ==========================
	// copy out the frame buffer
	// ==========================

	// set where to read from
	// * rendering is done to back buffer
	glReadBuffer		(GL_BACK);

	// bind the texture to store the frame buffer data
	glBindTexture		(GL_TEXTURE_2D, sTexCopyId);

	// initiate the copy from the framebuffer to the texture
	glCopyTexImage2D	(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, DST_MAP_SIZE_X, DST_MAP_SIZE_Y, 0);

#endif // SAMPLE_FBO

	// check for OpenGL error
	SFW_GL_ERROR();

#endif // SAMPLE_FBO || SAMPLE_TEX_COPY

	// clear the framebuffer
	glClearColor		(0.25, 0.25, 0.25, 0.0);
	glClear				(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the viewport
	glViewport			(0, 0, SFW::System::Instance()->WinSizeX(), SFW::System::Instance()->WinSizeY());

	// load the projection matrix
	glMatrixMode		(GL_PROJECTION);
    glLoadIdentity		();
	gluPerspective		(CAM_FOV, (float)(SFW::System::Instance()->WinSizeX())/(float)(SFW::System::Instance()->WinSizeY()), CAM_CLIP_NEAR, CAM_CLIP_FAR);

	// check for OpenGL error
	SFW_GL_ERROR();

	// load the model -> view matrix
	glMatrixMode		(GL_MODELVIEW);
	glLoadIdentity		();

	// load the camera matrix
	sCamera.LoadMatrix();

	// check for OpenGL error
	SFW_GL_ERROR();

	// draw the scene
	drawScene			();
	
	// check for OpenGL error
	SFW_GL_ERROR();

#if SAMPLE_FBO || SAMPLE_TEX_COPY

	// =======================================================
	// draw the copied texture or the color buffer of the FBO
	// out as a quad
	// =======================================================

	// load default matrices
	glMatrixMode		(GL_TEXTURE);
	glPushMatrix		();
	glLoadIdentity		();
	glMatrixMode		(GL_PROJECTION);
	glPushMatrix		();
	glLoadIdentity		();
	glOrtho				(0, SFW::System::Instance()->WinSizeX(), 0, SFW::System::Instance()->WinSizeY(), -1.0f, 1.0f);
	glMatrixMode		(GL_MODELVIEW);
	glPushMatrix		();
	glLoadIdentity		();

	// disable z-buffer and enable texture mapping
	glDisable			(GL_DEPTH_TEST);
	glEnable			(GL_TEXTURE_2D);

	// bind the proper texture object (sCBO or sTexCopyId)
#if SAMPLE_FBO
	glBindTexture		(GL_TEXTURE_2D, sCBO);
#else // SAMPLE_FBO
	glBindTexture		(GL_TEXTURE_2D, sTexCopyId);
#endif // SAMPLE_FBO
	
	glBegin				(GL_QUADS);
	glTexCoord2f		(0.0f, 0.0f);	glVertex2f(  0.0f,   0.0f);
	glTexCoord2f		(1.0f, 0.0f);	glVertex2f(128.0f,   0.0f);
	glTexCoord2f		(1.0f, 1.0f);	glVertex2f(128.0f, 128.0f);
	glTexCoord2f		(0.0f, 1.0f);	glVertex2f(  0.0f, 128.0f);
	glEnd				();
	
	// disable texture mapping
	glBindTexture		(GL_TEXTURE_2D, 0);
	glDisable			(GL_TEXTURE_2D);

	// re-enable z-buffer
	glEnable			(GL_DEPTH_TEST);

	// pop all matrices
	glMatrixMode		(GL_TEXTURE);
	glPopMatrix			();
	glMatrixMode		(GL_PROJECTION);
	glPopMatrix			();
	glMatrixMode		(GL_MODELVIEW);
	glPopMatrix			();

#endif // SAMPLE_FBO || SAMPLE_TEX_COPY

	// restore the attribute
	glPopAttrib			();
	glPopClientAttrib	();
}

// --------------------------------------------------------------------------- 

void gameStateFree()
{
}

// ---------------------------------------------------------------------------

void gameStateDone()
{
}

// ---------------------------------------------------------------------------

void drawScene()
{
	// toggle the texture map
	if (sFlagTex)
	{
		// load the 1st texture
		// * glActiveTexture is called to 'select' a texture unit
		// * Any subsequent texture related functions affect that particular texture unit
		glActiveTexture	(GL_TEXTURE0);
		glEnable		(GL_TEXTURE_2D);
		glBindTexture	(GL_TEXTURE_2D, sTexId[0]);
		glMatrixMode	(GL_TEXTURE);
		glLoadIdentity	();

		// load the 2nd texture
		glActiveTexture	(GL_TEXTURE1);
		glEnable		(GL_TEXTURE_2D);
		glBindTexture	(GL_TEXTURE_2D, sTexId[1]);
		glMatrixMode	(GL_TEXTURE);
		glLoadIdentity	();
		// apply some tiling on the 2nd map
		glScalef		(8.0f, 8.0f, 8.0f);

		// not really necessary, I just prefer to have texture unit 0 as the default one
		glActiveTexture	(GL_TEXTURE0);

		// by default, set the matrix mode to modelview
		glMatrixMode	(GL_MODELVIEW);

		//glEnable		(GL_BLEND);
	}

	// ========================================
	// load model->world camera
	// * camera matrix is already in the stack
	// ========================================

	// do some simple animation
	glPushMatrix();
	glTranslatef(0.0, 0.25f * sinf((sFrameCtr & 0x3FF) / 1023.0f * 2.0f * PI), 0.0f);
	glRotatef	((sFrameCtr & 0x1FF) / 511.0f * 360.0f, 0.0f, 1.0f, 0.0f);
	glRotatef	(sinf((sFrameCtr & 0x0FF) / 255.0f * TWO_PI) * 15.0f, 1.0f, 0.0f, 0.0f);

	// ==================
	// enable the shader
	// ==================

	// execute the shader
	SFW::Shader::Program* pProg = SFW::Shader::Program::Find("Default");
	
	// store the attibute location
	//GLint attrLoc[1] = { -1 };
	
	if (pProg)
	{
		pProg->Execute();
		
		glUniform1i(glGetUniformLocation(pProg->ObjectId(), "uFlagTex"),  sFlagTex);
		glUniform1i(glGetUniformLocation(pProg->ObjectId(), "uFlagNrm"),  sFlagNrm);

		// set the texture samplers association
		glUniform1i(glGetUniformLocation(pProg->ObjectId(), "uTex[0]"), 0);
		glUniform1i(glGetUniformLocation(pProg->ObjectId(), "uTex[1]"), 1);

		// get the attribute variable location
		//attrLoc[0] = glGetAttribLocation(pProg->ObjectId(), "aTan");
	}

	// set the attribute pointer and enable the attribute array
	//if (attrLoc[0] >= 0)
	//{
	//	glVertexAttribPointer		(attrLoc[0], 3, GL_FLOAT, GL_FALSE, sizeof(SFW::Shape::Vtx), (GLvoid*)(&SFW::Shape::VtxList(SFW::Shape::TYPE_PLANE)->tan));
	//	glEnableVertexAttribArray	(attrLoc[0]);
	//}

#if 1

	// draw the shape normally
	SFW::Shape::Draw(sShapeIdx);

#elif 0

	// draw the shape in wireframe
	SFW::Shape::DrawWireFrame(sShapeIdx);

#else

	// draw the shape in points
	glBegin(GL_POINTS);
	for (s32 i = SFW::Shape::VtxNum(sShapeIdx); i >= 0; --i)
	{
		SFW::Shape::Vtx* pVtx = SFW::Shape::VtxList(sShapeIdx) + i;

		glNormal3fv  (pVtx->nrm.v);
		glColor4fv   ((GLfloat*)(&pVtx->clr));
		glTexCoord2fv(pVtx->tex.v);
		glVertex3fv  (pVtx->pos.v);
	}
	glEnd();

#endif

	// disable the attribute
	//if (attrLoc[0] >= 0)
	//	glDisableVertexAttribArray	(attrLoc[0]);

	if (sFlagTex)
	{
		// disable texturing on both texture units
		glActiveTexture	(GL_TEXTURE1);
		glDisable		(GL_TEXTURE_2D);
		glActiveTexture	(GL_TEXTURE0);
		glDisable		(GL_TEXTURE_2D);

		glDisable(GL_BLEND);
	}

	// ===================
	// disable the shader
	// ===================

	glUseProgram(0);

	// draw the normals
	if (sFlagNrm)
		SFW::Shape::DrawNormal(sShapeIdx);
	
	glPopMatrix();
}

// ---------------------------------------------------------------------------



// ---------------------------------------------------------------------------
