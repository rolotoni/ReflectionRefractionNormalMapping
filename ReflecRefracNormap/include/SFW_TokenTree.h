// ---------------------------------------------------------------------------
// Project Name   : Simple Framework
// File Name    : SFW_TokenTree.h
// Author     : Sun Tjen Fam
// Creation Date  : 2009/02/01
// Purpose      : header for a token tree class
// ---------------------------------------------------------------------------

#ifndef SFW_TOKEN_TREE_H
#define SFW_TOKEN_TREE_H

// ---------------------------------------------------------------------------
// includes

#include "SFW.h"

// ---------------------------------------------------------------------------

namespace SFW
{

// ---------------------------------------------------------------------------
// class definitions

class TokenTree
{
public:
  class Token
  {
    friend class TokenTree;

  public:
            Token   () : mpData(0), mpParent(0), mpChild(0), mpNext(0), mpPrev(0) { }
            ~Token    ();


    // functions to get the data
    const s8*   AsString  () const  { return data();      }
    f64       AsFloat   () const  { return atof(data());  }
    s32       AsInteger () const  { return atoi(data());  }

    s32       AsArray   (u8*  pArray, u32 numMax = 0) const;
    s32       AsArray   (u16* pArray, u32 numMax = 0) const;
    s32       AsArray   (u32* pArray, u32 numMax = 0) const;
    s32       AsArray   (f32* pArray, u32 numMax = 0) const;
    s32       AsArray   (f64* pArray, u32 numMax = 0) const;

    // alias for signed integer types
    s32       AsArray   (s8*  pArray, u32 numMax = 0) const { return AsArray((u8*) (pArray), numMax); }
    s32       AsArray   (s16* pArray, u32 numMax = 0) const { return AsArray((u16*)(pArray), numMax); }
    s32       AsArray   (s32* pArray, u32 numMax = 0) const { return AsArray((u32*)(pArray), numMax); }

    // accessors
    Token     Parent    () const;
    Token     Child   () const;
    Token     Next    () const;
    Token     Prev    () const;

    // token index in the token list
    // * return 0 if actual token tree is contructed
    u32       Index   () const;
    
    // count the number of children of the current node
    s32       ChildNum  () const;

    bool      IsNull    () const;

  private:
    s8*       mpData;
    Token*      mpParent;
    Token*      mpChild;
    Token*      mpNext;
    Token*      mpPrev;

    const s8*   data    () const  { return mpData;    }
    Token*      insertNext  (s8* mpData);
    Token*      insertChild (s8* mpData);
  };

  // ---------------------------------------------------------------------------

  // create a token tree from a string
  // * the 'const' version will make a copy of the original data and 
  //   it will actually construct a tree
  // * the 'non-const' version will not construct a tree. Data access will be 
  //   MUCH slower. But, it will use much less memory. (sizeof(Token) bytes less per token).
  static TokenTree* Create  (const s8* pRaw);
  static TokenTree* Create  (s8* pRaw);
  
  // load a token tree from file
  static TokenTree* Load  (const char* pFilename);

  // save the tree to a file
  bool        Save  (const char* pFilename, bool format = true) const;

  // free allocated tree
  static void     Free  (TokenTree* pTokenTree);
  void        Free  ()      { Free(this);     }
  
  // get the root
  Token       Root  () const  { return *mpRoot;   }

  // get the number of token
  // * return 0 if an actual tree is constructed
  u32         TokenNum() const  { return mTokenNum; }

  // find a given string in the tree starting at the given node
  Token       Find  (s8* pData, Token start = Token()) const;

  // print the tree
  void        Print () const  { print(Token());     }

private:
  static Token    msDummy;
  
  Token*        mpRoot;

  s8**        mppToken;
  u32         mTokenNum;

            TokenTree ();
            TokenTree (const TokenTree& rhs);
            ~TokenTree();

  TokenTree&      operator= (const TokenTree& rhs);

  bool        save    (FILE* pFile, Token token, bool format = true, u32 depth = 0) const;
  void        free    (Token* pToken);
  void        print   (Token token, u32 depth = 0) const;
};

// ---------------------------------------------------------------------------

} // namespace SFW

// ---------------------------------------------------------------------------

#endif
