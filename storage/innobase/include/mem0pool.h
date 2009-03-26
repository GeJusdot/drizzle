/*****************************************************************************

Copyright (c) 1994, 2009, Innobase Oy. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

*****************************************************************************/

/******************************************************
The lowest-level memory management

Created 6/9/1994 Heikki Tuuri
*******************************************************/

#ifndef mem0pool_h
#define mem0pool_h

#include "univ.i"
#include "os0file.h"
#include "ut0lst.h"

typedef struct mem_area_struct	mem_area_t;
typedef struct mem_pool_struct	mem_pool_t;

/* The common memory pool */
extern mem_pool_t*	mem_comm_pool;

/* Memory area header */

struct mem_area_struct{
	ulint		size_and_free;	/* memory area size is obtained by
					anding with ~MEM_AREA_FREE; area in
					a free list if ANDing with
					MEM_AREA_FREE results in nonzero */
	UT_LIST_NODE_T(mem_area_t)
			free_list;	/* free list node */
};

/* Each memory area takes this many extra bytes for control information */
#define MEM_AREA_EXTRA_SIZE	(ut_calc_align(sizeof(struct mem_area_struct),\
			UNIV_MEM_ALIGNMENT))

/************************************************************************
Creates a memory pool. */
UNIV_INTERN
mem_pool_t*
mem_pool_create(
/*============*/
			/* out: memory pool */
	ulint	size);	/* in: pool size in bytes */
/************************************************************************
Allocates memory from a pool. NOTE: This low-level function should only be
used in mem0mem.*! */
UNIV_INTERN
void*
mem_area_alloc(
/*===========*/
				/* out, own: allocated memory buffer */
	ulint*		psize,	/* in: requested size in bytes; for optimum
				space usage, the size should be a power of 2
				minus MEM_AREA_EXTRA_SIZE;
				out: allocated size in bytes (greater than
				or equal to the requested size) */
	mem_pool_t*	pool);	/* in: memory pool */
/************************************************************************
Frees memory to a pool. */
UNIV_INTERN
void
mem_area_free(
/*==========*/
	void*		ptr,	/* in, own: pointer to allocated memory
				buffer */
	mem_pool_t*	pool);	/* in: memory pool */
/************************************************************************
Returns the amount of reserved memory. */
UNIV_INTERN
ulint
mem_pool_get_reserved(
/*==================*/
				/* out: reserved mmeory in bytes */
	mem_pool_t*	pool);	/* in: memory pool */
/************************************************************************
Reserves the mem pool mutex. */
UNIV_INTERN
void
mem_pool_mutex_enter(void);
/*======================*/
/************************************************************************
Releases the mem pool mutex. */
UNIV_INTERN
void
mem_pool_mutex_exit(void);
/*=====================*/
/************************************************************************
Validates a memory pool. */
UNIV_INTERN
ibool
mem_pool_validate(
/*==============*/
				/* out: TRUE if ok */
	mem_pool_t*	pool);	/* in: memory pool */
/************************************************************************
Prints info of a memory pool. */
UNIV_INTERN
void
mem_pool_print_info(
/*================*/
	FILE*		outfile,/* in: output file to write to */
	mem_pool_t*	pool);	/* in: memory pool */


#ifndef UNIV_NONINL
#include "mem0pool.ic"
#endif

#endif