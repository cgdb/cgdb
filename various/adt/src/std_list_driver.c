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

#include <assert.h>
#include <stdio.h>
#include "std_list.h"

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
my_list_compare_one (const void* a, const void* b)
{
  int one = *((const int*)a);
  int two = *((const int*)b);
  return one-two;
}

static int
my_list_compare_two (const void* a, const void* b)
{
  int one = *((const int*)a);
  int two = *((const int*)b);
  return two-one;
}

int
main (int   argc,
      char *argv[])
{
  struct std_list *list, *t;
  int nums[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  int morenums[10] = { 8, 9, 7, 0, 3, 2, 5, 1, 4, 6};
  int i;

  list = NULL;
  for (i = 0; i < 10; i++)
    list = std_list_append (list, &nums[i]);
  list = std_list_reverse (list);

  for (i = 0; i < 10; i++)
    {
      t = std_list_nth (list, i);
      assert (*((int*) std_list_get_data ( t )) == (9 - i));
    }

  for (i = 0; i < 10; i++)
    assert (std_list_position(list, std_list_nth (list, i)) == i);

  std_list_free (list);
  list = NULL;

  for (i = 0; i < 10; i++)
    list = std_list_insert_sorted (list, &morenums[i], my_list_compare_one);

  /*
  g_print("\n");
  std_list_foreach (list, my_list_print, NULL);
  */

  for (i = 0; i < 10; i++)
    {
      t = std_list_nth (list, i);
      assert (*((int*) std_list_get_data ( t )) == i);
    }

  std_list_free (list);
  list = NULL;

  for (i = 0; i < 10; i++)
    list = std_list_insert_sorted (list, &morenums[i], my_list_compare_two);

  /*
  g_print("\n");
  std_list_foreach (list, my_list_print, NULL);
  */

  for (i = 0; i < 10; i++)
    {
      t = std_list_nth (list, i);
      assert (*((int*) std_list_get_data ( t )) == (9 - i));
    }

  std_list_free (list);
  list = NULL;

  for (i = 0; i < 10; i++)
    list = std_list_prepend (list, &morenums[i]);

  list = std_list_sort (list, my_list_compare_two);

  /*
  g_print("\n");
  std_list_foreach (list, my_list_print, NULL);
  */

  for (i = 0; i < 10; i++)
    {
      t = std_list_nth (list, i);
      assert (*((int*) std_list_get_data ( t )) == (9 - i));
    }

  std_list_free (list);

  printf ( "PASSED\n" );

  return 0;
}

