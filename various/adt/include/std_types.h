/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __STD_TYPES_H__
#define __STD_TYPES_H__

typedef int 		(*STDCompareFunc)( 
	const void *a, 
	const void *b);

typedef int 		(*STDFunc) ( 
	void *data, 
	void *user_data);

typedef int     	(*STDCompareDataFunc) (
	const void *a,
	const void *b,
	void *user_data);

typedef unsigned int (*STDHashFunc) (const void*key);
typedef int          (*STDEqualFunc)(const void*a, const void*b);
typedef void         (*STDDestroyNotify) (void *data);
typedef void         (*STDHFunc) (
	void * key,
        void * value,
        void * user_data);

typedef void (*STDFreeFunc) (void * data);

#endif /* __STD_TYPES_H__ */

