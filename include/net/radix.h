/******************************************************************************
*   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
*
*   This file is part of Real VMX.
*   Copyright (C) 2010 Surplus Users Ham Society
*
*   Real VMX is free software: you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation, either version 2.1 of the License, or
*   (at your option) any later version.
*
*   Real VMX is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with Real VMX.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

/* radix.h - Common routing routines header */

#ifndef _radix_h
#define _radix_h

/* Defines */
#define RNF_NORMAL		0x1			/* Normal route */
#define RNF_ROOT		0x2			/* Root leaf */
#define RNF_ACTIVE		0x4			/* Node alive */
#define RNF_BLACKHOLE		0x8			/* Not up */

#ifndef _ASMLANGUAGE

#ifdef __cplusplus
extern "C" {
#endif

/* Structs */
struct radix_node {
  struct radix_mask		*rn_mklist;		/* List of masks */
  struct radix_node		*rn_p;			/* Parent */
  short				rn_b;			/* Bit offset */
  char				rn_bmask;		/* Mask for test */
  unsigned char			rn_flags;		/* Flags */
  union {

    struct {
      void			    *rn_key;		    /* Object key */
      void			    *rn_mask;		    /* Netmask */
      struct radix_node		    *rn_dupedkey;	    /* Duplicated key */
    } rn_leaf;

    struct {
      int			    rn_off;		    /* Node data */
      struct radix_node		    *rm_l;
      struct radix_node		    *rm_r;
    } rn_node;

  } rn_u;
};

#define rn_dupedkey		rn_u.rn_leaf.rn_dupedkey
#define rn_key			rn_u.rn_leaf.rn_key
#define rn_mask			rn_u.rn_leaf.rn_mask
#define rn_off			rn_u.rn_node.rn_off
#define rn_l			rn_u.rn_node.rm_l
#define rn_r			rn_u.rn_node.rm_r

struct radix_mask {
  short				rm_b;			/* Bit offset */
  char				rm_unused;		/* Mask */
  unsigned char			rm_flags;		/* Flags */
  struct radix_mask		*rm_mklist;		/* Mask list */
  union {
    void			  *rmu_mask;		  /* Mask */
    struct radix_node		  *rmu_leaf;		  /* Leaf */
  } rm_rmu;

  int rm_refs;						/* References */
};

#define rm_mask			rm_rmu.rmu_mask
#define rm_leaf			rm_rmu.rmu_leaf

struct radix_node_head {
  struct radix_node		*rnh_treetop;		/* Top of tree */
  int				rnh_addrsize;		/* Address size */
  int				rnh_pktsize;		/* Packet size */

  FUNCPTR			rnh_addaddr;		/* Add from sockaddr */
  FUNCPTR			rnh_addpkt;		/* Add from pkt hdr */
  FUNCPTR			rnh_deladdr;		/* Rem from sockaddr */
  FUNCPTR			rnh_delpkt;		/* Rem from pkt hdr */
  FUNCPTR			rnh_matchaddr;		/* Match fr sockaddr */
  FUNCPTR			rnh_lookup;		/* Match fr sockaddr */
  FUNCPTR			rnh_matchpkt;		/* Match fr pkt hdr */
  FUNCPTR			rnh_walktree;		/* Traverse tree */
  FUNCPTR			rnh_walktree_from;	/* Traverse sub-tree */

  struct radix_node		rnh_nodes[3];		/* Empty tree */
};

/* Macros */

/*******************************************************************************
 * R_Malloc - Allocate memory for routing node
 *
 * RETURNS: N/A
 ******************************************************************************/

#define R_Malloc(p, t, n)	(p = (t) mb_alloc(n, MT_RTABLE, M_DONTWAIT))

/* Functions */

IMPORT struct radix_node* rn_search(void *v_arg, struct radix_node *head);
IMPORT struct radix_node* rn_search_m(void *v_arg,
				      struct radix_node *head,
				      void *m_arg);
IMPORT int rn_refines(void *m_arg, void *n_arg);
IMPORT struct radix_node* rn_lookup(void *v_arg,
				    void *m_arg,
				    struct radix_node_head *head);
IMPORT struct radix_node* rn_match(void *v_arg,
				   struct radix_node_head *head,
				   BOOL skipFlag);
IMPORT struct radix_node* rn_newpair(void *v, int b, struct radix_node *nodes);
IMPORT struct radix_node* rn_insert(void *v_arg,
				    struct radix_node_head *head,
				    int *dupedentry,
				    struct radix_node *nodes);
IMPORT struct radix_node* rn_addmask(void *n_arg, int search, int skip);
IMPORT struct radix_node* rn_addroute(void *v_arg,
				      void *n_arg,
				      struct radix_node_head *head,
				      struct radix_node *treenodes);
IMPORT struct radix_node* rn_delete(void *v_arg,
				    void *netmask,
				    struct radix_node_head *head);
IMPORT int rn_walktree_from(struct radix_node_head *h,
			    void *a,
			    void *m,
			    FUNCPTR f,
			    void *w);
IMPORT int rn_walktree(struct radix_node_head *head, FUNCPTR func, void *w);
IMPORT int rn_inithead(void **head, int off);
IMPORT STATUS radixLibInit(int maxKeyLen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASMLANGUAGE */

#endif /* _radix_h */

