// ---------------------------------------------------------------------------
// File Name		:	phong.vs
// Author			:	Jesse Harrison
// Creation Date	:	6/7/11
// Purpose			:	phong fragment shader
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// uniforms
//----------------------------------------------------------------------------

uniform mat4 InvCam;
uniform mat4 Lv;
uniform mat4 Lp;
uniform mat4 Ls;

// ---------------------------------------------------------------------------
// varyings

varying vec3 normal;
varying vec3 pos;
varying vec3 tan;
varying vec3 bitan;
varying vec4 Vl;
varying vec4 TC;
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// attributes

attribute vec3 Tan;
attribute vec3 BiTan;

// ---------------------------------------------------------------------------



void main()
{
	vec4 Vv;
	tan   = (gl_ModelViewMatrix * vec4(Tan.xyz,0.0)).xyz;
	bitan = (gl_ModelViewMatrix * vec4(BiTan.xyz,0.0)).xyz;
	// calculate the position in projection space
	gl_Position		=	ftransform();
	
	// calculate the normal in camera space
	normal			=	gl_NormalMatrix * gl_Normal;
	
	pos = (gl_ModelViewMatrix * gl_Vertex).xyz;
	Vv  = gl_ModelViewMatrix * gl_Vertex;

	Vl = Lv * InvCam * Vv;
	TC = Ls * Lp * Vl;

	// pass along the vertex color unmodified
	gl_FrontColor	=	gl_Color;
	
	// multiply the incoming texture coordinate with the texture matrix
	// associated with each texture unit to generate 2 texture coordinates
	gl_TexCoord[0]	=	gl_TextureMatrix[0]  * gl_MultiTexCoord0;
	gl_TexCoord[1]	=	gl_TextureMatrix[1]  * gl_MultiTexCoord0;
	gl_TexCoord[2]	=	gl_TextureMatrix[2]  * gl_MultiTexCoord0;
	gl_TexCoord[3]	=	gl_TextureMatrix[3]  * gl_MultiTexCoord0;
	gl_TexCoord[5]	=	gl_TextureMatrix[5]  * gl_MultiTexCoord0;
	gl_TexCoord[6]	=	gl_TextureMatrix[6]  * gl_MultiTexCoord0;

}

