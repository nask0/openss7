/*****************************************************************************

 @(#) $Id$

 -----------------------------------------------------------------------------

 Copyright (c) 2001-2007  OpenSS7 Corporation <http://www.openss7.com/>
 Copyright (c) 1997-2001  Brian F. G. Bidulock <bidulock@openss7.org>

 All Rights Reserved.

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free Software
 Foundation; version 3 of the License.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 details.

 You should have received a copy of the GNU General Public License along with
 this program.  If not, see <http://www.gnu.org/licenses/>, or write to the
 Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 -----------------------------------------------------------------------------

 U.S. GOVERNMENT RESTRICTED RIGHTS.  If you are licensing this Software on
 behalf of the U.S. Government ("Government"), the following provisions apply
 to you.  If the Software is supplied by the Department of Defense ("DoD"), it
 is classified as "Commercial Computer Software" under paragraph 252.227-7014
 of the DoD Supplement to the Federal Acquisition Regulations ("DFARS") (or any
 successor regulations) and the Government is acquiring only the license rights
 granted herein (the license rights customarily provided to non-Government
 users).  If the Software is supplied to any unit or agency of the Government
 other than DoD, it is classified as "Restricted Computer Software" and the
 Government's rights in the Software are defined in paragraph 52.227-19 of the
 Federal Acquisition Regulations ("FAR") (or any successor regulations) or, in
 the cases of NASA, in paragraph 18.52.227-86 of the NASA Supplement to the FAR
 (or any successor regulations).

 -----------------------------------------------------------------------------

 Commercial licensing and support of this software is available from OpenSS7
 Corporation at a fee.  See http://www.openss7.com/

 -----------------------------------------------------------------------------

 Last Modified $Date$ by $Author$

 -----------------------------------------------------------------------------

 $Log$
 *****************************************************************************/

#ifndef __LOOKUP_RYINITIATOR_H__
#define __LOOKUP_RYINITIATOR_H__

#ident "@(#) $RCSfile$ $Name$($Revision$) Copyright (c) 2001-2007 OpenSS7 Corporation."

/* ryinitiator.h - include file for the generic interactive initiator */

/* 
 * Header: /xtel/isode/isode/others/lookup/RCS/ryinitiator.h,v 9.0 1992/06/16 12:42:23 isode Rel
 *
 *
 * Log: ryinitiator.h,v
 * Revision 9.0  1992/06/16  12:42:23  isode
 * Release 8.0
 *
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */

#include "rosy.h"

static struct dispatch {
	char *ds_name;
	int ds_operation;

	IFP ds_argument;
	modtyp *ds_mod;			/* pointer to module table */
	int ds_ind;			/* Index into module table */

	IFP ds_result;
	IFP ds_error;

	char *ds_help;
};

void adios(), advise();
void acs_adios(), acs_advise();
void ros_adios(), ros_advise();

int ryinitiator();

#endif				/* __LOOKUP_RYINITIATOR_H__ */