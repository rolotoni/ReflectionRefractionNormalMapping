// ---------------------------------------------------------------------------
// Project Name		:	Simple Framework
// File Name		:	OpenGLSample_Basic.h
// Author			:	Sun Tjen Fam
// Creation Date	:	2010/01/13
// Purpose			:	header file for a simple OpenGL sample
// ---------------------------------------------------------------------------

#ifndef OGL_SAMPLE_BASIC_H
#define OGL_SAMPLE_BASIC_H

// ---------------------------------------------------------------------------
// includes

void GameState_OpenGLSample_Basic();
void CreateLights();

//shaders
u32 GetFileLength(const char* filename);
char* loadshader(const char* filename);
int unloadshader(const char* ShaderSource);
void CompileShader(GLint shader);
void LinkShader(GLuint program);
GLuint InitShaders(const char* vs_filename, const char* fs_filename);

//takes a height map creates a normal map and saves the normal image
void HeightMapToNormalMap(SFW::Image*);

void CalculateTangentAndBitanget(u32 shape);

void CreateShapeRing();
void DrawSkyBox(SFW::Vec3 CamPos);
void LoadTextures();
void CreateScene(bool draw_main = false);
void SetUpCubeMap();
void DrawCubeMap();
void InvCamMatrix();
// ---------------------------------------------------------------------------

#endif // OGL_SAMPLE_BASIC_H


