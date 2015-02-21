// ---------------------------------------------------------------------------
// Project Name		:	Simple Framework
// File Name		:	SFW_Geometry.h
// Author			:	Sun Tjen Fam
// Creation Date	:	2010/01/13
// Purpose			:	header file for the geometry library
// ---------------------------------------------------------------------------

#ifndef SFW_GEOMETRY_H
#define SFW_GEOMETRY_H

// ---------------------------------------------------------------------------
// includes

#include "SFW.h"

// ---------------------------------------------------------------------------

namespace SFW
{
namespace Geo
{

// ---------------------------------------------------------------------------
// Structure definitions

// use the default Vec3 from SFW as the basic vector class
typedef Vec3 Vec;

// ---------------------------------------------------------------------------

enum
{
	CP_FRONT = 0, 
	CP_BEHIND, 
	CP_STRADDLE, 
	CP_COPLANAR, 
};

// ---------------------------------------------------------------------------

struct Segment
{
	// start and end of the line segment
	union
	{
		struct
		{
			Vec start, end;
		};
		struct
		{
			Vec v[2];			
		};
	};

		Segment(const Vec& s, const Vec& e)
		{
			start = s;
			end   = e;
		}

	f32	Length	() const	
	{	
		return (end - start).Length();
	}
};

// ---------------------------------------------------------------------------

// Line structure
struct Line
{
	Vec	start;	// line starting position
	Vec	dir;	// line direction (might not be normalized)
		
		Line	(const Vec& s, const Vec& d)
		{
			start = s;
			dir   = d;
		}
		
		Line	(const Segment& seg)
		{
			start = seg.start;
			dir   = seg.end - seg.start;
		}
};

// ---------------------------------------------------------------------------

// Ray structure. Practically the same as line
struct Ray
{
	Vec	start;	// ray starting point
	Vec	dir;	// ray direction (might not be normalized)
		
		Ray	(const Vec& s, const Vec& d)
		{
			start = s;
			dir   = d;
		}
		
		Ray	(const Segment& seg)
		{
			start = seg.start;
			dir   = seg.end - seg.start;
		}
};

// ---------------------------------------------------------------------------

struct Plane
{
	union
	{
		struct
		{
			Vec	n;
		};
		struct
		{
			f32	a, b, c;
		};
	};
	f32		d;

			Plane	()
			{
				n = Vec(0.0f);
				d = 0.0f;
			}

			// construct a plane from a normal and -distance of origin from the plane (the 'd')
			Plane	(const Vec& nn, f32 dd)
			{
				n = nn;
				d = dd;
			}

			// construct with the equations parameters
			Plane	(f32 aa, f32 bb, f32 cc, f32 dd)
			{
				a = aa;
				b = bb;
				c = cc;
				d = dd;
			}

			// construct from three points
			Plane	(const Vec& v0, const Vec& v1, const Vec& v2)
			{
				n = (v1 - v0).Cross(v2 - v0);
				d = -n.Dot(v0);
			}

			// construct from array of three points
			Plane	(const Vec* pV)
			{
				n = (pV[1] - pV[0]).Cross(pV[2] - pV[0]);
				d = -n.Dot(pV[0]);
			}
	
	Plane	Normalize()		{	f32 len = n.Length(); return Plane(n / len, d / len);	}
	Plane&	NormalizeThis()	{	return (*this) = this->Normalize();						}
};

// ---------------------------------------------------------------------------

struct Triangle
{
	union
	{
		struct
		{
			Vec v0, v1, v2;
		};
		struct
		{
			Vec v[3];
		};
	};

		// construct with the triangle from 3 points
		Triangle	(const Vec& u0, const Vec& u1, const Vec& u2)
		{
			v0 = u0;
			v1 = u1;
			v2 = u2;
		}

	Vec	Normal() const
	{
		return (v1 - v0).Cross(v2 - v0).Normalize();
	}
};

// ---------------------------------------------------------------------------

struct Sphere
{
	Vec				center;	// center of the sphere
	f32				radius;	// sphere radius

					// construct from given center and radius
					Sphere	(const Vec& c, f32 r)
					{
						center = c;
						radius = r;
					}
					
					// construct from a set of points
					Sphere	(const Vec* pPos, u32 posNum)
					{
						*this = Create(pPos, posNum);
					}

	// create bounding sphere
	static Sphere	Create				(const Vec* pPos, u32 posNum);
	static Sphere	Create_AABB			(const Vec* pPos, u32 posNum);
	static Sphere	Create_Iterative	(const Vec* pPos, u32 posNum);
	static Sphere	Create_Welz			(const Vec* pPos, u32 posNum);

	f32				Volume				() const	{	return 4.0f * PI * radius * radius * radius / 3.0f;	}
	f32				SurfaceArea			() const	{	return 4.0f * PI * radius * radius;					}
};

// ---------------------------------------------------------------------------

struct AABB
{
	Vec			center;	// center of the AABB
	Vec			extend;	// the AABB extend (half of its size)

				AABB	(const Vec& c, const Vec& e)
				{
					center = c;
					extend = e;
				}

				AABB	(const Vec& c, f32 w, f32 h, f32 d)
				{
					center = c;
					extend = Vec(w, h, d);
				}

				AABB	(const Vec* pPos, u32 posNum)
				{
					*this = Create(pPos, posNum);
				}

	static AABB	Create		(const Vec* pPos, u32 posNum);

	f32			Volume		() const	{	return 4.0f * extend.x * extend.y * extend.z;										}
	f32			SurfaceArea	() const	{	return 2.0f * (extend.x * extend.y + extend.y * extend.z + extend.z * extend.x);	}
};


// ---------------------------------------------------------------------------

struct OBB
{
	Vec center;		// center of the AABB
	Vec extend[3];	// the AABB extend (half of its size)
#if 0
		OBB	(const Vec& c, const Vec& e0, const Vec& e1, const Vec& e2)
		{
			center    = c;
			extend[0] = e0;
			extend[1] = e1;
		}

		OBB	(const Vec& c, f32 w, f32 h, f32 d)
		{
			center = c;
			extend = Vec(w, h, d);
		}

		OBB	(const Vec* pP);
#endif
};

// ---------------------------------------------------------------------------
// calculate the projection of a given point to other entity

// calculate the projection of a given point to another entity
Vec Project(const Vec& p, const Line&  line);
Vec Project(const Vec& p, const Plane& plane);

// ---------------------------------------------------------------------------
// calculate the distance from one entity to another

// from dist from a given point to another entity
f32 CalcDist(const Vec& p0,	const Vec&		p1);
f32 CalcDist(const Vec& p,	const Line&		line);
f32 CalcDist(const Vec& p,	const Segment&	segment);
f32 CalcDist(const Vec& p,	const Ray&		ray);
f32 CalcDist(const Vec& p,	const Plane&	plane);
f32 CalcDist(const Vec& p,	const Triangle&	triangle);
f32 CalcDist(const Vec& p,	const Sphere&	sphere);
f32 CalcDist(const Vec& p,	const AABB&		aabb);

// from dist from a given line to another entity
f32 CalcDist(const Line& line0, const Line&   line1);
f32 CalcDist(const Line& line,  const Sphere& sphere);

// from dist from a given segment to another entity
f32 CalcDist(const Segment& seg0, const Segment&  seg1);

// ---------------------------------------------------------------------------
// check if a given entity is contains within another entity

// check if a given 2D point is contained in a 2D triangle
// * consider only the x and y component of the vectors
bool IsContained2D(const Vec& p, const Triangle& tri);

// check if given point is contained within another entity
bool IsContained(const Vec& p, const Line&		line);
bool IsContained(const Vec& p, const Segment&	segment);
bool IsContained(const Vec& p, const Ray&		ray);
bool IsContained(const Vec& p, const Plane&		plane);
bool IsContained(const Vec& p, const Triangle&	tri);
bool IsContained(const Vec& p, const Sphere&	sphere);
bool IsContained(const Vec& p, const AABB&		aabb);

// ---------------------------------------------------------------------------
// calculate intersection between one entity with another
// * pInterPoint is the 1st intersection point between the 2 entities
// * pT? is the 'time' of the intersection point

// pT0 = time for line0 or seg0
// pT1 = time for line1 or seg1
bool Intersect2D(const Line&    line0, const Line&    line1, Vec* pInterPoint = 0, f32* pT0 = 0, f32* pT1 = 0);
bool Intersect2D(const Segment& seg0,  const Segment& seg1,  Vec* pInterPoint = 0, f32* pT0 = 0, f32* pT1 = 0);

// pT0 = time for line
bool Intersect(const Line&    line,    const Plane& plane, Vec* pInterPoint = 0, f32* pT = 0);

// pT0 = time for line
bool Intersect(const Segment& segment, const Plane&    plane, Vec* pInterPoint = 0, f32* pT = 0);
bool Intersect(const Segment& segment, const Triangle& tri,   Vec* pInterPoint = 0, f32* pT = 0);

// pT0 = time for the ray
bool Intersect(const Ray&     ray,     const Plane&    plane,  Vec* pInterPoint = 0, f32* pT = 0);
bool Intersect(const Ray&     ray,     const Triangle& plane,  Vec* pInterPoint = 0, f32* pT = 0);
bool Intersect(const Ray&     ray,     const Sphere&   sphere, Vec* pInterPoint = 0, f32* pT = 0);
bool Intersect(const Ray&     ray,     const AABB&     aabb,   Vec* pInterPoint = 0, f32* pT = 0);

// ---------------------------------------------------------------------------
// misc test

// check if 2 entities are intersecting
bool IsIntersecting(const Sphere&   sph0,  const Sphere&   sph1);
bool IsIntersecting(const Triangle& tri0,  const Triangle& tri1);
bool IsIntersecting(const AABB&     aabb0, const AABB&     aabb1);

// check angle between 2 entities
// * return valus is in radian
f32 CalcAngle(const Line&  line0,  const Line&  line1);
f32 CalcAngle(const Line&  line,   const Plane& plane);
f32 CalcAngle(const Plane& plane0, const Plane& plane1);

// check if 2 entities are parallel
bool IsParallel(const Vec&   vec0,   const Vec&   vec1);
bool IsParallel(const Line&  line0,  const Line&  line1);
bool IsParallel(const Line&  line,   const Plane& plane);
bool IsParallel(const Plane& plane0, const Plane& plane1);

// check if 2 entities are perpendicular
bool IsPerpendicular(const Vec&   vec0,   const Vec&   vec1);
bool IsPerpendicular(const Line&  line0,  const Line&  line1);
bool IsPerpendicular(const Line&  line,   const Plane& plane);
bool IsPerpendicular(const Plane& plane0, const Plane& plane1);

// check if 2 entities are coplanar
bool IsCoplanar	(const Line&  line0,  const Line&  line1);
bool IsCoplanar	(const Line&  line,   const Plane& plane);
bool IsCoplanar	(const Plane& plane0, const Plane& plane1);

// ---------------------------------------------------------------------------
// misc functions

// calculate the barycentri coordinate of 'q' wrt triangle (p0, p1, p2). 
// * return true if barycentri coordinate can be calculated properly and 
//   pS and pT stores the calcualted coordinate.
// * return false if barycentri coordinate cannot be calculated properly 
//   (given triangle is degenerated, i.e. it is a point or a line) and
//   pS and pT will be set to 0.
bool CalcBarycentricCoord(const Vec& q, const Vec& p0, const Vec& p1, const Vec& p2, f32* pS, f32* pT);

// classify the given list of points wrt given plane
// * return front, back, straddle or coplanar
// * can return the number of vertices in front, behind and on the plane through pFront, pBack and pOn
u32 ClassifyPoint(const Vec* pSrc, u32 srcNum, const Plane& plane, u32* pFront = 0, u32* pBack = 0, u32* pOn = 0);

// clip the given convex poly (as a list of vertices) along the given plane
// * pSrc and srcNum is the list of input convex poly vertices and the count
// * pDst is the pointer to store the clipped poly (the part of pSrc that lies in front of the plane)
// * dstNumMax is the maximum number of vertices that can be stored in pDst. 
// * returns the number of vertices in pDst
// * the function do NOT allocate pDst internally. It is the caller 
//   responsibility to give anough memory to store the result. An n-sided poly can 
//   be clipped to an (n+1)-sided poly.
u32 ClipConvexPoly(const Vec* pSrc, u32 srcNum, const Plane& plane, Vec* pDst, u32 dstNumMax);

// clip the given convex poly (as a list of vertices) against the given AABB
// * internally, this function clip the poly 6 times. Once per AABB side.
// * pDst is the pointer to store the clipped poly (the part of pSrc that lies within the AABB)
// * returns the number of vertices in pDst
// * the function do NOT allocate pDst internally. It is the caller 
//   responsibility to give anough memory to store the result. An n-sided poly can 
//   be clipped to an (n+3)-sided poly.
u32 ClipConvexPoly(const Vec* pSrc, u32 srcNum, const AABB& aabb, Vec* pDst, u32 dstNumMax);

// split the given convex poly (as a list of vertices) along the given plane
// * pSrc and srcNum is the list of input convex poly vertices and the count
// * pFront and pFrontNum store the piece that lies on the front side of the plane
// * pBack and pBackNum store the piece that lies on the back side of the plane
// * the function do NOT allocate pFront and pBack internally. It is the caller 
//   responsibility to give anough memory to store the result. An n-sided poly can 
//   be clipped to an (n+1)-sided poly.
void SplitConvexPoly(const Vec* pSrc, u32 srcNum, const Plane& plane, Vec* pFront, u32* pFrontNum, Vec* pBack, u32* pBackNum);

// ---------------------------------------------------------------------------

} // namespace Geo
} // namespace SFW

// ---------------------------------------------------------------------------

#endif // SFW_GEOMETRY_H







