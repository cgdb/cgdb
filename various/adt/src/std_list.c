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

#include "std_list.h"

struct std_list
{
  void* data;
  struct std_list *next;
  struct std_list *prev;
};

/**
 * Allocate for a new list.
 *
 * @return
 * A new list on success, or NULL on error
 */
static struct std_list*
std_list_alloc (void)
{
  struct std_list *list;
  
  list = malloc ( sizeof ( struct std_list ));
  list->next = NULL;
  list->prev = NULL;

  if ( !list )
	  return NULL;
  
  return list;
}

/**
 * Free a list
 *
 * \param list
 * The list to free.
 *
 * @return 0 on success, or -1 on error.
 */
int
std_list_free (struct std_list *list)
{
  struct std_list *last;

  if ( !list )
	  return -1;
  
  while (list)
    {
      last = list;
      list = list->next;
      free (last);
    }

  return 0;
}

void
std_list_free_1 (struct std_list *list)
{
  free (list);
}

void *std_list_get_data ( struct std_list *list ) {
	if ( !list )
		return NULL;

	return list->data;
}

struct std_list*
std_list_append (struct std_list	*list,
	       void*	 data)
{
  struct std_list *new_list;
  struct std_list *last;
  
  new_list = std_list_alloc ();
  new_list->data = data;
  
  if (list)
    {
      last = std_list_last (list);
      last->next = new_list;
      new_list->prev = last;

      return list;
    }
  else
    return new_list;
}

struct std_list*
std_list_prepend (struct std_list	 *list,
		void*  data)
{
  struct std_list *new_list;
  
  new_list = std_list_alloc ();
  new_list->data = data;
  
  if (list)
    {
      if (list->prev)
	{
	  list->prev->next = new_list;
	  new_list->prev = list->prev;
	}
      list->prev = new_list;
      new_list->next = list;
    }
  
  return new_list;
}

struct std_list*
std_list_insert (struct std_list	*list,
	       void*	 data,
	       int	 position)
{
  struct std_list *new_list;
  struct std_list *tmp_list;
  
  if (position < 0)
    return std_list_append (list, data);
  else if (position == 0)
    return std_list_prepend (list, data);
  
  tmp_list = std_list_nth (list, position);
  if (!tmp_list)
    return std_list_append (list, data);
  
  new_list = std_list_alloc ();
  new_list->data = data;
  
  if (tmp_list->prev)
    {
      tmp_list->prev->next = new_list;
      new_list->prev = tmp_list->prev;
    }
  new_list->next = tmp_list;
  tmp_list->prev = new_list;
  
  if (tmp_list == list)
    return new_list;
  else
    return list;
}

struct std_list*
std_list_insert_before (struct std_list   *list,
		      struct std_list   *sibling,
		      void* data)
{
  if (!list)
    {
      list = std_list_alloc ();
      list->data = data;
      return list;
    }
  else if (sibling)
    {
      struct std_list *node;

      node = std_list_alloc ();
      node->data = data;
      if (sibling->prev)
	{
	  node->prev = sibling->prev;
	  node->prev->next = node;
	  node->next = sibling;
	  sibling->prev = node;
	  return list;
	}
      else
	{
	  node->next = sibling;
	  sibling->prev = node;
	  return node;
	}
    }
  else
    {
      struct std_list *last;

      last = list;
      while (last->next)
	last = last->next;

      last->next = std_list_alloc ();
      last->next->data = data;
      last->next->prev = last;

      return list;
    }
}

struct std_list *
std_list_concat (struct std_list *list1, struct std_list *list2)
{
  struct std_list *tmp_list;
  
  if (list2)
    {
      tmp_list = std_list_last (list1);
      if (tmp_list)
	tmp_list->next = list2;
      else
	list1 = list2;
      list2->prev = tmp_list;
    }
  
  return list1;
}

struct std_list*
std_list_remove (struct std_list	     *list,
	       const void*  data)
{
  struct std_list *tmp;
  
  tmp = list;
  while (tmp)
    {
      if (tmp->data != data)
	tmp = tmp->next;
      else
	{
	  if (tmp->prev)
	    tmp->prev->next = tmp->next;
	  if (tmp->next)
	    tmp->next->prev = tmp->prev;
	  
	  if (list == tmp)
	    list = list->next;
	  
	  std_list_free_1 (tmp);
	  
	  break;
	}
    }
  return list;
}

struct std_list*
std_list_remove_all (struct std_list	*list,
		   const void* data)
{
  struct std_list *tmp = list;

  while (tmp)
    {
      if (tmp->data != data)
	tmp = tmp->next;
      else
	{
	  struct std_list *next = tmp->next;

	  if (tmp->prev)
	    tmp->prev->next = next;
	  else
	    list = next;
	  if (next)
	    next->prev = tmp->prev;

	  std_list_free_1 (tmp);
	  tmp = next;
	}
    }
  return list;
}

static inline struct std_list*
_std_list_remove_link (struct std_list *list,
		     struct std_list *link)
{
  if (link)
    {
      if (link->prev)
	link->prev->next = link->next;
      if (link->next)
	link->next->prev = link->prev;
      
      if (link == list)
	list = list->next;
      
      link->next = NULL;
      link->prev = NULL;
    }
  
  return list;
}

struct std_list*
std_list_remove_link (struct std_list *list,
		    struct std_list *link)
{
  return _std_list_remove_link (list, link);
}

struct std_list*
std_list_delete_link (struct std_list *list,
		    struct std_list *link)
{
  list = _std_list_remove_link (list, link);
  std_list_free_1 (link);

  return list;
}

struct std_list*
std_list_copy (struct std_list *list)
{
  struct std_list *new_list = NULL;

  if (list)
    {
      struct std_list *last;

      new_list = std_list_alloc ();
      new_list->data = list->data;
      last = new_list;
      list = list->next;
      while (list)
	{
	  last->next = std_list_alloc ();
	  last->next->prev = last;
	  last = last->next;
	  last->data = list->data;
	  list = list->next;
	}
    }

  return new_list;
}

struct std_list*
std_list_reverse (struct std_list *list)
{
  struct std_list *last;
  
  last = NULL;
  while (list)
    {
      last = list;
      list = last->next;
      last->next = last->prev;
      last->prev = list;
    }
  
  return last;
}

struct std_list*
std_list_nth (struct std_list *list,
	    unsigned int  n)
{
  while ((n-- > 0) && list)
    list = list->next;
  
  return list;
}

struct std_list*
std_list_nth_prev (struct std_list *list,
		 unsigned int  n)
{
  while ((n-- > 0) && list)
    list = list->prev;
  
  return list;
}

void*
std_list_nth_data (struct std_list     *list,
		 unsigned int      n)
{
  while ((n-- > 0) && list)
    list = list->next;
  
  return list ? list->data : NULL;
}

struct std_list*
std_list_find (struct std_list         *list,
	     const void*  data)
{
  while (list)
    {
      if (list->data == data)
	break;
      list = list->next;
    }
  
  return list;
}

struct std_list*
std_list_find_custom (struct std_list         *list,
		    const void*  data,
		    STDCompareFunc   func)
{
  /* Do nothing if func is NULL */
  if ( !func )
	  return list;

  while (list)
    {
      if (! func (list->data, data))
	return list;
      list = list->next;
    }

  return NULL;
}


int
std_list_position (struct std_list *list,
		 struct std_list *link)
{
  int i;

  i = 0;
  while (list)
    {
      if (list == link)
	return i;
      i++;
      list = list->next;
    }

  return -1;
}

int
std_list_index (struct std_list         *list,
	      const void*  data)
{
  int i;

  i = 0;
  while (list)
    {
      if (list->data == data)
	return i;
      i++;
      list = list->next;
    }

  return -1;
}

struct std_list*
std_list_last (struct std_list *list)
{
  if (list)
    {
      while (list->next)
	list = list->next;
    }
  
  return list;
}

struct std_list*
std_list_first (struct std_list *list)
{
  if (list)
    {
      while (list->prev)
	list = list->prev;
    }
  
  return list;
}

unsigned int
std_list_length (struct std_list *list)
{
  unsigned int length;
  
  length = 0;
  while (list)
    {
      length++;
      list = list->next;
    }
  
  return length;
}

void
std_list_foreach (struct std_list	 *list,
		STDFunc	  func,
		void*  user_data)
{
  while (list)
    {
      struct std_list *next = list->next;
      (*func) (list->data, user_data);
      list = next;
    }
}


struct std_list*
std_list_insert_sorted (struct std_list        *list,
                      void*      data,
                      STDCompareFunc  func)
{
  struct std_list *tmp_list = list;
  struct std_list *new_list;
  int cmp;

  if ( !func )
	  return list;

  if (!list) 
    {
      new_list = std_list_alloc ();
      new_list->data = data;
      return new_list;
    }
  
  cmp = (*func) (data, tmp_list->data);
  
  while ((tmp_list->next) && (cmp > 0))
    {
      tmp_list = tmp_list->next;
      cmp = (*func) (data, tmp_list->data);
    }

  new_list = std_list_alloc ();
  new_list->data = data;

  if ((!tmp_list->next) && (cmp > 0))
    {
      tmp_list->next = new_list;
      new_list->prev = tmp_list;
      return list;
    }
   
  if (tmp_list->prev)
    {
      tmp_list->prev->next = new_list;
      new_list->prev = tmp_list->prev;
    }
  new_list->next = tmp_list;
  tmp_list->prev = new_list;
 
  if (tmp_list == list)
    return new_list;
  else
    return list;
}

static struct std_list *
std_list_sort_merge (struct std_list     *l1, 
		   struct std_list     *l2,
		   STDFunc     compare_func,
		   int  use_data,
		   void*  user_data)
{
  struct std_list list, *l, *lprev;
  int cmp;

  l = &list; 
  lprev = NULL;

  while (l1 && l2)
    {
      if (use_data)
	cmp = ((STDCompareDataFunc) compare_func) (l1->data, l2->data, user_data);
      else
	cmp = ((STDCompareFunc) compare_func) (l1->data, l2->data);

      if (cmp <= 0)
        {
	  l->next = l1;
	  l = l->next;
	  l->prev = lprev; 
	  lprev = l;
	  l1 = l1->next;
        } 
      else 
	{
	  l->next = l2;
	  l = l->next;
	  l->prev = lprev; 
	  lprev = l;
	  l2 = l2->next;
        }
    }
  l->next = l1 ? l1 : l2;
  l->next->prev = l;

  return list.next;
}

static struct std_list* 
std_list_sort_real (struct std_list    *list,
		  STDFunc     compare_func,
		  int  use_data,
		  void*  user_data)
{
  struct std_list *l1, *l2;
  
  if (!list) 
    return NULL;
  if (!list->next) 
    return list;
  
  l1 = list; 
  l2 = list->next;

  while ((l2 = l2->next) != NULL)
    {
      if ((l2 = l2->next) == NULL) 
	break;
      l1 = l1->next;
    }
  l2 = l1->next; 
  l1->next = NULL; 

  return std_list_sort_merge (std_list_sort_real (list, compare_func, use_data, user_data),
			    std_list_sort_real (l2, compare_func, use_data, user_data),
			    compare_func,
			    use_data,
			    user_data);
}

struct std_list *
std_list_sort (struct std_list        *list,
	     STDCompareFunc  compare_func)
{
  return std_list_sort_real (list, (STDFunc) compare_func, 0, NULL);
			    
}

struct std_list *
std_list_sort_with_data (struct std_list            *list,
		       STDCompareDataFunc  compare_func,
		       void*          user_data)
{
  return std_list_sort_real (list, (STDFunc) compare_func, 1, user_data);
}
