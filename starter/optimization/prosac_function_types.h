/*
 Copyright 2005, 2006 Computer Vision Lab,
 Ecole Polytechnique Federale de Lausanne (EPFL), Switzerland.
 Modified by Damian Stewart <damian@frey.co.nz> 2009-2010;
 modifications Copyright 2009, 2010 Damian Stewart <damian@frey.co.nz>.

 Distributed under the terms of the GNU General Public License v3.
 
 This file is part of The Artvertiser.
 
 The Artvertiser is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 The Artvertiser is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public License
 along with The Artvertiser.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef PROSAC_FUNCTION_TYPES_H
#define PROSAC_FUNCTION_TYPES_H

typedef bool (*prosac_function_20scalars)(ls_minimizer_type * state, 
                                          ls_minimizer_type s0, ls_minimizer_type s1, ls_minimizer_type s2, ls_minimizer_type s3, ls_minimizer_type s4, 
                                          ls_minimizer_type s5, ls_minimizer_type s6, ls_minimizer_type s7, ls_minimizer_type s8, ls_minimizer_type s9, 
                                          ls_minimizer_type s10, ls_minimizer_type s11, ls_minimizer_type s12, ls_minimizer_type s13, ls_minimizer_type s14, 
                                          ls_minimizer_type s15, ls_minimizer_type s16, ls_minimizer_type s17, ls_minimizer_type s18, ls_minimizer_type s19, 
                                          void ** user_data);

#endif // PROSAC_FUNCTION_TYPES_H
