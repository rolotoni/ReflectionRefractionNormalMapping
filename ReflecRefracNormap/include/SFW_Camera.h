// ---------------------------------------------------------------------------
// Project Name		:	Simple Framework
// File Name		:	SFW_Camera.h
// Author			:	Sun Tjen Fam
// Creation Date	:	2009/09/29
// Purpose			:	Simple camera class
// ---------------------------------------------------------------------------

#ifndef SFW_CAMERA_H
#define SFW_CAMERA_H

// ---------------------------------------------------------------------------
// includes

#include "SFW.h"

// ---------------------------------------------------------------------------

namespace SFW
{

// ---------------------------------------------------------------------------
// class declarations

class Camera
{
public:
	enum
	{
		MODE_SPHERICAL = 0, 
		MODE_VIEWER, 

		// keep it last
		MODE_NUM
	};

					Camera		(f32 distMin = 1.0f, f32 distMax = 1000.0f);

	// do one update
	void			Reset		();

	// update the camera data based on current input
	void			Update		(bool useInput = true);

	// load the transformation matrix on top of OpenGL modelview stack
	void			LoadMatrix	();

	// switch the camera mode
	void			Mode		(u8 mode)		{	mMode = Clamp<s8>(mode, 0, MODE_NUM - 1);	}
	u8				Mode		() const		{	return mMode;								}

	// get and set the x and y rotation angle and the camera distance
	void			Alpha		(f32 alpha)		{	mAlpha  = Clamp<f32>(alpha, -(f32)(HALF_PI), (f32)(HALF_PI));		}
	f32				Alpha		() const		{	return mAlpha;										}
	void			Beta		(f32 beta)		{	mBeta   = Wrap<f32>(beta,  -(f32)(PI), (f32)(PI));				}
	f32				Beta		() const		{	return mBeta;										}
	void			Dist		(f32 dist)		{	mDist   = Clamp<f32>(dist,  mDistMin, mDistMax);	}
	f32				Dist		() const		{	return mDist;										}
	
	void			DistMin		(f32 dist)		{	mDistMin = Clamp<f32>(dist, 0.0f, mDistMax);		}
	f32				DistMin		() const		{	return mDistMin;									}
	void			DistMax		(f32 dist)		{	mDistMax = Max<f32>(mDistMin, dist);				}
	f32				DistMax		() const		{	return mDistMax;										}

	// functions to adjust the target/position
	// * depending on the camera mode, new values might not be used
	void			Target		(const Vec3& p)	{	mTarget = p;	}
	const Vec3&		Target		() const		{	return mTarget;	}
	void			Pos			(const Vec3& p)	{	mPos = p;		}
	const Vec3&		Pos			() const		{	return mPos;	}

	// get the current camera up and direction (they are normalized)
	const Vec3&		Up			() const		{	return mUp;		}
	const Vec3&		Dir			() const		{	return mDir;	}

	// get the current camera matrix
	// * only accurate IF LoadMatrix is called with identity in the modelview stack.
	const f32*		Matrix		() const		{	return mMatrix;	}

private:
	u8				mMode;

	// camera target, position, up vector and direction
	Vec3			mTarget;
	Vec3			mPos;
	Vec3			mUp;
	Vec3			mDir;

	// world to camera spaace matrix
	// * data is column major (OpenGL)
	f32				mMatrix[16];
	
	// camara rotation around the x and y axis
	f32				mAlpha;
	f32				mBeta;
	
	// camera position distance from the target
	// * in viewer camera mode, mDist is used as speed instead
	f32				mDist;

	// distance clamp
	f32				mDistMin, 
					mDistMax;
};

// ---------------------------------------------------------------------------

} // namespace SFW

// ---------------------------------------------------------------------------

#endif // SFW_H


