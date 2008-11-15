/* Copyright (C) 2000 MySQL AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include "mysys_priv.h"
#include "mysys_err.h"
#include <mystrings/m_string.h>

	/* My memory allocator */

void *my_malloc(size_t size, myf my_flags)
{
  void* point;

  if (!size)
    size=1;					/* Safety */
  if ((point = malloc(size)) == NULL)
  {
    my_errno=errno;
    if (my_flags & MY_FAE)
      error_handler_hook=fatal_error_handler_hook;
    if (my_flags & (MY_FAE+MY_WME))
      my_error(EE_OUTOFMEMORY, MYF(ME_BELL+ME_WAITTANG+ME_NOREFRESH),size);
    if (my_flags & MY_FAE)
      exit(1);
  }
  else if (my_flags & MY_ZEROFILL)
    memset(point, 0, size);
  return((void*) point);
} /* my_malloc */


	/* malloc and copy */

void* my_memdup(const void *from, size_t length, myf my_flags)
{
  void *ptr;
  if ((ptr= my_malloc(length,my_flags)) != 0)
    memcpy(ptr, from, length);
  return(ptr);
}


char *my_strdup(const char *from, myf my_flags)
{
  char *ptr;
  size_t length= strlen(from)+1;
  if ((ptr= (char*) my_malloc(length, my_flags)))
    memcpy(ptr, from, length);
  return(ptr);
}


char *my_strndup(const char *from, size_t length, myf my_flags)
{
  char *ptr;
  if ((ptr= (char*) my_malloc(length+1,my_flags)) != 0)
  {
    memcpy(ptr, from, length);
    ptr[length]=0;
  }
  return((char*) ptr);
}