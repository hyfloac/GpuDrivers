#pragma once

#if defined _STL70_
#include "_70_xstddef"
#define _CNTSIZ(iter) ptrdiff_t
#elif defined _STL100_
#include "_100_xstddef"
#elif defined _STL110_
#include "_110_xstddef"
#else

// xstddef standard header
#ifndef _XSTDDEF_
#define _XSTDDEF_
#ifndef _YVALS
#include <yvals.h>
#endif
#include <cstddef>
#include <crtdefs.h>

#ifdef  _MSC_VER
#pragma pack(push,8)
#endif  /* _MSC_VER */
_STD_BEGIN
		// EXCEPTION MACROS
#if _HAS_EXCEPTIONS
#define _TRY_BEGIN	try {
#define _CATCH(x)	} catch (x) {
#define _CATCH_ALL	} catch (...) {
#define _CATCH_END	}
#define _RAISE(x)	throw (x)
#define _RERAISE	throw
#define _THROW0()	throw ()
#define _THROW1(x)	throw (x)
#define _THROW(x, y)	throw x(y)
#else /* no exceptions */
#define _TRY_BEGIN	{{
#define _CATCH(x)	} if (0) {
#define _CATCH_ALL	} if (0) {
#define _CATCH_END	}}

#define _RAISE(x)	(x)
#define _RERAISE

#define _THROW0()
#define _THROW1(x)
#define _THROW(x, y)	x(y)
#endif /* _HAS_EXCEPTIONS */
	   // explicit KEYWORD
	   // BITMASK MACROS
#define _BITMASK(E, T)	typedef int T
#define _BITMASK_OPS(T)
	   // MISCELLANEOUS MACROS
#define _DESTRUCTOR(ty, ptr)	(ptr)->~ty()
#define _MESG(str)	str
#define _PROTECTED	public
#define _TDEF(x)		= x
#define _TDEF2(x, y)	= x, y
#define _CNTSIZ(iter)	ptrdiff_t
#define _TDEFP(x)
#define _STCONS(ty, name, val)	enum {name = val}
	   // TYPE DEFINITIONS
	enum _Uninitialized { _Noinit };
			// FUNCTIONS
_CRTIMP2 void __cdecl _Nomemory();
_STD_END
#ifdef  _MSC_VER
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif /* _XSTDDEF_ */

/*
 * Copyright (c) 1995 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 */
// The file \sdpublic\sdk\inc\crt\xstddef was reviewed by LCA in June 2011 and per license is
//   acceptable for Microsoft use under Dealpoint ID 46582, 201971

#endif	// _STL70_ or _100_ or _110_
/* 88bf0570-3001-4e78-a5f2-be5765546192 */
