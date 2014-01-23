//
//  endian.h
//  CaveStory
//
//  Created on 8/29/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef CaveStory_endian_h
#define CaveStory_endian_h

#if !defined(WIN32)
# include <endian.h>
#endif

#if !defined(htole16)
# if defined(__APPLE__)
#  include <CoreFoundation/CoreFoundation.h>
#  define htole16(x) CFSwapInt16HostToLittle(x)
# elif defined(WIN32)
#  define htole16(x) x
# endif
#endif

#endif
