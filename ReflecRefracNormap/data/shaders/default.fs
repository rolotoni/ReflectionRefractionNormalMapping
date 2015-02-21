// ---------------------------------------------------------------------------
// Project Name		:	Simple Framework
// File Name		:	deafult.fs
// Author			:	Sun Tjen Fam
// Creation Date	:	2009/10/05
// Purpose			:	basic fragment shader
// ---------------------------------------------------------------------------

#define CM_TEXTURE		0
#define CM_CONST		1
#define CM_VTX_COLOR	2
#define CM_NORMAL		3
#define CM_TEXCOORD		4

// ---------------------------------------------------------------------------
// uniforms

// setup the 2 texture samplers
uniform sampler2D uTex[2];

uniform int  uFlagClr;

// ---------------------------------------------------------------------------
// varyings

varying vec3	vNormal;

// ---------------------------------------------------------------------------

void main()
{
	// check if want to render the vertex color
	if (uFlagClr == CM_VTX_COLOR)
	{
		gl_FragColor = gl_Color;
		return;
	}
	// check if want to render the normal as color
	else if (uFlagClr == CM_NORMAL)
	{
		gl_FragColor = vec4(vNormal, 1.0);
		return;
	}
	// check if want to render the texture coordinate as color
	else if (uFlagClr == CM_TEXCOORD)
	{
		gl_FragColor = gl_TexCoord[0];
		return;
	}
	else if (uFlagClr == CM_TEXTURE)
	{
		// average the 2 textures color and mutiply the vertex color
		gl_FragColor = (texture2D(uTex[0], gl_TexCoord[0].st) + texture2D(uTex[1], gl_TexCoord[1].st)) * 0.5 * gl_Color;
	}
	else
	{
		gl_FragColor = vec4(0.5, 0.5, 1.0, 1.0);
	}
}


