#pragma once

#ifndef NO_ERROR
#define error(vars) { printf("ERROR: %s:\t", __FUNCTION__); printf vars; printf("\n"); }
#else
#define error(vars) ;
#endif

#ifndef NO_WARNING
#define warn(vars) { printf("WARN: %s:\t", __FUNCTION__); printf vars; printf("\n"); }
#else
#define warn(vars) ;
#endif

#ifndef NO_INFO
#define info(vars) { printf("INFO: %s:\t", __FUNCTION__); printf vars; printf("\n"); }
#else
#define info(vars) ;
#endif

#ifndef NO_DEBUG
#define debug(vars) { printf("DEBUG: %s:\t", __FUNCTION__); printf vars; printf("\n"); }
#else
#define debug(vars) {}
#endif
