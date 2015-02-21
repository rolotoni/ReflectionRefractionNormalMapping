// ---------------------------------------------------------------------------
// Project Name		:	Simple Framework
// File Name		:	shader.h
// Author			:	Sun Tjen Fam
// Creation Date	:	2008/10/23
// Purpose			:	header for shader manager
// ---------------------------------------------------------------------------

#ifndef SFW_SHADER_H
#define SFW_SHADER_H

// ---------------------------------------------------------------------------
// includes

#include "SFW.h"

// ---------------------------------------------------------------------------

namespace SFW
{

// ---------------------------------------------------------------------------

class Shader
{
public:
	enum
	{
		TYPE_VERTEX, 
		TYPE_FRAGMENT, 

		TYPE_NUM
	};

	// ---------------------------------------------------------------------------

	class Program
	{
		friend class Shader;

	public:
		static Program*				Create		(const char* pName, u32 shaderNum);
		static Program*				Find		(const char* pName);

		// ---------------------------------------------------------------------------

		bool						ShaderName	(u32 shaderIdx, const char* pShaderName);

		bool						Link		();

		void						Execute		();
		static void					Terminate	();

		// ---------------------------------------------------------------------------
		
		const char*					Name		() const	{	return mpName;		}

		GLuint						ObjectId	() const	{	return mObjectId;	}

		// ---------------------------------------------------------------------------

	private:
		char*						mpName;		// program name

		GLuint						mObjectId;	// OpenGL program object id

		GLuint*						mpShader;	// list of source id
		u16							mShaderNum;	// 

		u16							mFlag;		// general purpose flag

									Program		();
									Program		(const Program& rhs);
		const Program&				operator=	(const Program& rhs);

									~Program	();
	};

	// ---------------------------------------------------------------------------

	// create shader from file or string
	// * the name passed in must be unique!!
	static Shader*					SourceFromFile	(u32 type, const char* pName, const char* pFilename);
	static Shader*					SourceFromStr	(u32 type, const char* pName, const char* pString);
	
	static void						Free			(const char* pName);

	static Shader*					Find			(const char* pName);

	// ---------------------------------------------------------------------------

private:
	typedef std::list<Shader*>		ShaderList;
	typedef std::list<Program*>		ProgramList;

	// ---------------------------------------------------------------------------

	u32								mType;			// shader type

	char*							mpName;			// shader name
	u32								mSourceId;		// 'crc' of the source string/filename

	u32								mCounter;		// keep track how many times this shader has been created/freed
	
	GLuint							mObjectId;		// OpenGL shader object id
	
	// ---------------------------------------------------------------------------

									Shader		();
									Shader		(const Shader& rhs);
	const Shader&					operator=	(const Shader& rhs);

									~Shader		();

	// ---------------------------------------------------------------------------

	static ShaderList				msShaderList;	// list of all loaded shaders
	static ProgramList				msProgramList;	// list of all created program
};

// ---------------------------------------------------------------------------

} // namespace SFW

// ---------------------------------------------------------------------------

#endif // SHADER_H
