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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "std_bbtree.h"

int array[10000];
int failed = 0;

#define	TEST(m,cond)	G_STMT_START { failed = !(cond); \
if (failed) \
  { if (!m) \
      g_print ("\n(%s:%d) failed for: %s\n", __FILE__, __LINE__, ( # cond )); \
    else \
      g_print ("\n(%s:%d) failed for: %s: (%s)\n", __FILE__, __LINE__, ( # cond ), (char*)m); \
  } \
else \
  g_print ("."); fflush (stdout); \
} G_STMT_END

#define	C2P(c)		((void*) ((long) (c)))
#define	P2C(p)		((char) ((long) (p)))

#define GLIB_TEST_STRING "el dorado "
#define GLIB_TEST_STRING_5 "el do"

typedef struct {
	unsigned int age;
	char name[40];
} GlibTestInfo;


static int
my_compare (const void* a,
	    const void* b)
{
  const char *cha = a;
  const char *chb = b;

  return *cha - *chb;
}

static int
my_traverse (void* key,
	     void* value,
	     void* data)
{
  char *ch = key;
  assert ((*ch) > 0);
  return 0;
}

int runtreetest ( struct std_bbtree *tree ) {
  int i, j;
  char chars[62];

  tree = std_bbtree_new (my_compare);
  i = 0;
  for (j = 0; j < 10; j++, i++)
    {
      chars[i] = '0' + j;
      std_bbtree_insert (tree, &chars[i], &chars[i]);
    }
  for (j = 0; j < 26; j++, i++)
    {
      chars[i] = 'A' + j;
      std_bbtree_insert (tree, &chars[i], &chars[i]);
    }
  for (j = 0; j < 26; j++, i++)
    {
      chars[i] = 'a' + j;
      std_bbtree_insert (tree, &chars[i], &chars[i]);
    }

  std_bbtree_foreach (tree, my_traverse, NULL);

  assert (std_bbtree_nnodes (tree) == (10 + 26 + 26));

  for (i = 0; i < 10; i++)
    std_bbtree_remove (tree, &chars[i]);

  std_bbtree_foreach (tree, my_traverse, NULL);

  std_bbtree_destroy ( tree );

  return 0;
}

int
main (int   argc,
      char *argv[])
{
  struct std_bbtree *tree = NULL;
  int i;

  for ( i = 0; i < 10000; i++ )
	  runtreetest ( tree );

  printf ( "PASSED\n" );

  return 0;
}
