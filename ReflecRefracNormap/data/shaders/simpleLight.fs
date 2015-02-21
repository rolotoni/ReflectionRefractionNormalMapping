// ---------------------------------------------------------------------------
// Project Name		:	Simple Framework
// File Name		:	simpleLight.fs
// Author			:	Sun Tjen Fam
// Creation Date	:	2010/03/17
// Purpose			:	basic fragment shader
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// uniforms

uniform sampler2D uTex0;
uniform sampler2D uTex1;

// ---------------------------------------------------------------------------
// varyings

varying vec3 vNrm;
varying vec3 vPos;
varying vec3 tan;
varying vec3 bitan;

// ---------------------------------------------------------------------------

void main()
{
	vec4 matClr = gl_Color;
	vec3 nrm    = normalize(vNrm);
	vec3 view   = -normalize(vPos);
	
	gl_FragColor = dot(view, nrm) * matClr * 0.8 + matClr * 0.2;
	gl_FragColor = mix(gl_FragColor, vec4(0.25, 0.25,0.25, matClr.a), smoothstep(10.0, 50.0, -vPos.z));
}


