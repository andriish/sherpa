#ifndef ATOOLS_Org_My_Limits_H
#define ATOOLS_Org_My_Limits_H
<<<<<<< HEAD

#ifdef __GNUC__
#if __GNUC__ < 3
#else
#define USE_LIMITS
#endif
#endif

#if defined(__sgi) && !defined(__GNUC__)
#define USE_LIMITS
#endif

#if defined(_MSC_VER) 
#define USE_LIMITS
#endif

#ifdef USE_LIMITS
=======
>>>>>>> upstream/master
#include <limits>
#endif
