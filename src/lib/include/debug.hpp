// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef debug_hpp
#define debug_hpp

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

#endif
