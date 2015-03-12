// ---------------------------------------------------------------------------
// Project Name   : Simple Framework
// File Name    : SFW_Mesh.h
// Author     : Sun Tjen Fam
// Creation Date  : 2010/02/01
// Purpose      : Simple mesh class
// ---------------------------------------------------------------------------

#ifndef SFW_MESH_H
#define SFW_MESH_H

// ---------------------------------------------------------------------------
// includes

#include "SFW.h"
#include "SFW_TokenTree.h"

// ---------------------------------------------------------------------------

namespace SFW
{

// ---------------------------------------------------------------------------

class Mesh
{
public:
  
  // ---------------------------------------------------------------------------
  // simple triangle structure. It contains the 3 indices to the vertex list.

  struct Tri
  {
    union
    {
      struct
      {
        u32 p0, p1, p2;
      };
      u32 v[3];
    };
  };

  // ---------------------------------------------------------------------------
  
  static Mesh*  Load      (const char* pFilename, bool deindex = true);
  static void   Free      (Mesh* pMesh);
  
  // ---------------------------------------------------------------------------

  void      Draw      ();
  void      DrawNormal    ();
  void      DrawWireFrame ();
  
  // ---------------------------------------------------------------------------

  const Vec3*   VtxPos      () const  { return mpVtxPos;  }
  u32       VtxPosNum   () const  { return mVtxPosNum;  }
  const Tri*    TriPos      () const  { return mpTriPos;  }
  u32       TriPosNum   () const  { return mTriPosNum;  }

  const Vec3*   VtxNrm      () const  { return mpVtxNrm;  }
  u32       VtxNrmNum   () const  { return mVtxNrmNum;  }
  const Tri*    TriNrm      () const  { return mpTriNrm;  }
  u32       TriNrmNum   () const  { return mTriNrmNum;  }

  const Vec4*   VtxClr      () const  { return mpVtxClr;  }
  u32       VtxClrNum   () const  { return mVtxClrNum;  }
  const Tri*    TriClr      () const  { return mpTriClr;  }
  u32       TriClrNum   () const  { return mTriClrNum;  }

  const Vec3*   VtxTex      () const  { return mpVtxTex;  }
  u32       VtxTexNum   () const  { return mVtxTexNum;  }
  const Tri*    TriTex      () const  { return mpTriTex;  }
  u32       TriTexNum   () const  { return mTriTexNum;  }

  // ---------------------------------------------------------------------------

  void      Deindex     ();
  void      Print     () const;

private:
  Vec3*     mpVtxPos;
  u32       mVtxPosNum;
  Tri*      mpTriPos;
  u32       mTriPosNum;

  Vec3*     mpVtxNrm;
  u32       mVtxNrmNum;
  Tri*      mpTriNrm;
  u32       mTriNrmNum;
  
  Vec4*     mpVtxClr;
  u32       mVtxClrNum;
  Tri*      mpTriClr;
  u32       mTriClrNum;

  Vec3*     mpVtxTex;
  u32       mVtxTexNum;
  Tri*      mpTriTex;
  u32       mTriTexNum;

  // ---------------------------------------------------------------------------

          Mesh    ();
          Mesh    (const Mesh& rhs);
          ~Mesh   ();

  Mesh&     operator= (const Mesh& rhs);

  // ---------------------------------------------------------------------------

  bool      init    (TokenTree* pTT, const TokenTree::Token& mesh);
};

// ---------------------------------------------------------------------------

} // namespace SFW

// ---------------------------------------------------------------------------

#endif // SFW_H


