// ---------------------------------------------------------------------------
// File Name		:	phong.fs
// Author			:	Jesse Harrison
// Creation Date	:	6/7/11
// Purpose			:	phong fragment shader
// ---------------------------------------------------------------------------

#define CAM_FAR    70.0
#define CAM_NEAR  .01

#define CM_TEXTURE		0
#define CM_CONST		1
#define CM_VTX_COLOR	2
#define CM_NORMAL		3
#define CM_TANGENT      4
#define CM_BITANGENT    5
#define CM_TEXSAMPLE    6
#define CM_TEXCOORD		7

#define POINT_LIGHT     0
#define DIR_LIGHT       1
#define SPOT_LIGHT      2
// ---------------------------------------------------------------------------
// uniforms
uniform sampler2D emissive_tex;
uniform sampler2D diffuse_tex;
uniform sampler2D ambient_tex;
uniform sampler2D specular_tex;
uniform sampler2D normal_tex;
uniform sampler2D depth_tex;
uniform sampler2D shadow_tex;

uniform vec4 Idiffuse;
uniform vec4 Iambient;
uniform vec4 Ispecular;
uniform vec4 Gambient;

uniform vec3 lights[8];
uniform vec3 light_dirs[8];

uniform vec3 d_att;

uniform int  uFlagClr;
uniform int  num_lights;
uniform int  light_type;

uniform float SLinner;
uniform float SLouter;

uniform bool perform_shadow;

// ---------------------------------------------------------------------------
// varyings

varying vec3 normal;
varying vec3 pos;
varying vec3 tan;
varying vec3 bitan;
varying vec4 Vl;
varying vec4 TC;

// ---------------------------------------------------------------------------

vec4 emissive  = vec4(0,0,0,0);
vec4 Kdiffuse  = vec4(0.8,0.8,0.8,1);
vec4 Kambient  = vec4(0.0);
vec4 Kspecular = vec4(1.0);


float sampled_depth;
float fragment_depth; 

float ns = 1.0;//shiny
float SE = 1.0;//Spot effect
int SLfalloff = 1;

const float filter_size = 3.0;
const float sample_num = 49.0;

float PCF(float FilterSize, float SampleNum, vec2 tc)
{
  	float total = 0.0;
    vec2 tc_temp;
	float tex_step = 1.0/1024.0;//shadow map size 1024 * 1024
	  
	for(float j = 0.0; j < FilterSize; ++j)
	{
      for(float i = 0.0; i < sqrt(SampleNum); ++i)
	  {
		  if(i == 0)
		    if(fragment_depth < sampled_depth)
		      total += 1.0; 

		  tc_temp.x = tc.x + tex_step * (i -(j+1.0));
		  tc_temp.y = tc.y + tex_step * (i -(j+1.0));
		  sampled_depth  = texture2D(shadow_tex,tc_temp).z;
		  if(fragment_depth < sampled_depth)
		    if(tc_temp.x != tc.x || tc_temp.y != tc.y)
		      total += 1.0; 

		  tc_temp.x = tc.x - tex_step * (i -(j+1.0));
		  sampled_depth  = texture2D(shadow_tex,tc_temp).z;
		  if(fragment_depth < sampled_depth)
		    if(tc_temp.x != tc.x || tc_temp.y != tc.y)
		      total += 1.0; 

		  tc_temp.x = tc.x + tex_step * (i -(j+1.0));
		  tc_temp.y = tc.y;
		  sampled_depth  = texture2D(shadow_tex,tc_temp).z;
		  if(fragment_depth < sampled_depth)
		    if(tc_temp.x != tc.x || tc_temp.y != tc.y)
		      total += 1.0; 

		  tc_temp.x = tc.x;
		  tc_temp.y = tc.y + tex_step * (i -(j+1.0));
		  sampled_depth  = texture2D(shadow_tex,tc_temp).z;
		  if(fragment_depth < sampled_depth)
		    if(tc_temp.x != tc.x || tc_temp.y != tc.y)
		      total += 1.0; 

		}
	  }
	
	return (total / SampleNum);
}
 


void main()
{
    vec3 norm = normalize(normal);
    
	vec3 Tan   = normalize(tan);
	vec3 BiTan = normalize(bitan);

	mat3 TBNMat = mat3(Tan,BiTan,norm.xyz);

	vec2 tc;

	tc.x = TC.x / TC.w;
	tc.y = TC.y / TC.w;

	sampled_depth  = texture2D(shadow_tex,tc).z;
	fragment_depth = (-Vl.z - CAM_NEAR) / (CAM_FAR - CAM_NEAR);

    float Shadow = 1.0;

	if(fragment_depth > sampled_depth && perform_shadow)
	  Shadow = PCF(filter_size, sample_num, tc);

	
	if (uFlagClr == CM_VTX_COLOR)
	{
      gl_FragColor = gl_Color;
	  return;
	}
	else if (uFlagClr == CM_CONST)
	{
	  emissive  = vec4(0.0);
      Kdiffuse  = vec4(1.0);
      Kambient  = vec4(1.0);
      Kspecular = vec4(1.0);

	  norm = vec3(texture2D(normal_tex, gl_TexCoord[5].st)).xyz;

	  norm = (norm * 2.0) - 1.0;//[-1,1] range
	  norm = TBNMat * norm;
	  norm = normalize(norm);

	  ns = Kspecular.w * 256.0;
	  Kspecular.w = 1.0;
	}
	// check if want to render the normal as color
	else if (uFlagClr == CM_NORMAL)
	{
	  gl_FragColor = vec4(norm, 1.0);
	  return;
	}
	// check if want to render the texture coordinate as color
	else if (uFlagClr == CM_TEXCOORD)
	{
	  gl_FragColor = gl_TexCoord[0];
	  return;
	}
	else if (uFlagClr == CM_TANGENT)
	{
	  gl_FragColor = vec4(tan,1.0);
	  return;
	}
	else if (uFlagClr == CM_BITANGENT)
	{
	  gl_FragColor = vec4(bitan,1.0);
	  return;
	}
	else if (uFlagClr == CM_TEXSAMPLE)
	{
	  gl_FragColor = texture2D(normal_tex, gl_TexCoord[5].st);
	  return;
	}
	else if (uFlagClr == CM_TEXTURE)
	{
	  // average the 2 textures color and mutiply the vertex color
	  Kambient  = texture2D(ambient_tex, gl_TexCoord[0].st);
	  Kdiffuse  = texture2D(diffuse_tex, gl_TexCoord[1].st);
	  emissive  = texture2D(emissive_tex, gl_TexCoord[2].st);
	  Kspecular = texture2D(specular_tex, gl_TexCoord[3].st);
	  norm      = vec3(texture2D(normal_tex, gl_TexCoord[5].st)).xyz;

	  norm = (norm * 2.0) - 1.0;//[-1,1] range
	  norm = TBNMat * norm;
	  norm = normalize(norm);

	  ns = Kspecular.w * 256.0;
	  Kspecular.w = 1.0;
	}

	gl_FragColor = emissive + Gambient * Kambient;
    
    for(int i = 0; i < num_lights; ++i)
    {
	  vec3 L = lights[i] - pos;

	  if(light_type == DIR_LIGHT)
	    L = -light_dirs[i]; 

	  vec3 R = 2.0 * dot(norm, normalize(L)) * norm - normalize(L);
	  float d_sq = dot(L,L);
	  float d = sqrt(d_sq);
	  L = normalize(L);
	  float Att = min(1.0/(d_att.x + d_att.y * d + d_att.z * d_sq), 1.0);

	  vec4 Ambient  = Iambient * Kambient;
	  vec4 Diffuse  = Idiffuse * Kdiffuse * max(dot(norm,L), 0.0);
	  vec4 Specular = Ispecular * Kspecular * pow((max(dot(R, -normalize(pos)),0.0)), ns);
      
	  if(light_type == SPOT_LIGHT)
	  {
	    float light_dir = dot(L, -normalize(light_dirs[i]));
	    if(light_dir > SLinner)
	      SE = 1.0 * Shadow;
	    else if(light_dir < SLouter)
	      SE = 0.0;
	    else
	      SE = (light_dir - SLouter)/(SLinner - SLouter) * Shadow;
	  }

	  gl_FragColor += Att * Ambient + Att * SE *(Diffuse + Specular);
    }
	
	
  
}
