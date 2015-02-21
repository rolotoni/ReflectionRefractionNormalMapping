// ---------------------------------------------------------------------------
// Project Name		:	Simple Framework
// File Name		:	deafult.fs
// Author			:	Sun Tjen Fam
// Creation Date	:	2009/10/05
// Purpose			:	basic vertex shader
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// varyings

varying vec3 vNormal;

// ---------------------------------------------------------------------------

void main()
{
	// calculate the position in projection space
	gl_Position		=	ftransform();
	
	// calculate the normal in camera space
	vNormal			=	gl_NormalMatrix * gl_Normal;

	// pass along the vertex color unmodified
	gl_FrontColor	=	gl_Color;
	
	// multiply the incoming texture coordinate with the texture matrix
	// associated with each texture unit to generate 2 texture coordinates
	gl_TexCoord[0]	=	gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_TexCoord[1]	=	gl_TextureMatrix[1] * gl_MultiTexCoord0;
}



