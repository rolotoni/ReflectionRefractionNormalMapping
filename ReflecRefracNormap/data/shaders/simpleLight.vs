// ---------------------------------------------------------------------------
// Project Name		:	Simple Framework
// File Name		:	simpleLight.fs
// Author			:	Sun Tjen Fam
// Creation Date	:	2010/03/17
// Purpose			:	basic vertex shader
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// varyings

varying vec3 vNrm;
varying vec3 vPos;


// ---------------------------------------------------------------------------




void main()
{
	gl_Position		=	gl_ModelViewProjectionMatrix * gl_Vertex;

	// calculate the camera space position and normal
	vPos			=	(gl_ModelViewMatrix * gl_Vertex).xyz;
	vNrm			=	gl_NormalMatrix * gl_Normal;
	
	gl_FrontColor	=	gl_Color;
	
	// multiply the incoming texture coordinate with the texture matrix
	// associated with each texture unit to generate 2 texture coordinates
	gl_TexCoord[0]	=	gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_TexCoord[1]	=	gl_TextureMatrix[1] * gl_MultiTexCoord0;
}



