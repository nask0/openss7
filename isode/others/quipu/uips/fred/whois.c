/*****************************************************************************

 @(#) $RCSfile$ $Name$($Revision$) $Date$

 -----------------------------------------------------------------------------

 Copyright (c) 2001-2007  OpenSS7 Corporation <http://www.openss7.com/>
 Copyright (c) 1997-2000  Brian F. G. Bidulock <bidulock@openss7.org>

 All Rights Reserved.

 This program is free software: you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free Software
 Foundation, version 3 of the license.

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

#ident "@(#) $RCSfile$ $Name$($Revision$) $Date$"

static char const ident[] = "$RCSfile$ $Name$($Revision$) $Date$";

/* whois.c - fred whois function */

#ifndef	lint
static char *rcsid =
    "Header: /xtel/isode/isode/others/quipu/uips/fred/RCS/whois.c,v 9.0 1992/06/16 12:44:30 isode Rel";
#endif

/* 
 * Header: /xtel/isode/isode/others/quipu/uips/fred/RCS/whois.c,v 9.0 1992/06/16 12:44:30 isode Rel
 *
 *
 * Log: whois.c,v
 * Revision 9.0  1992/06/16  12:44:30  isode
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

#include <ctype.h>
#include <signal.h>
#include "fred.h"

/*    DATA */

struct whois {
	char *w_input;
	int w_inputype;
#define	W_NULL		0x00
#define	W_HANDLE	0x01
#define	W_MAILBOX	0x02
#define	W_NAME		0x03

	int w_nametype;
#define	W_COMMONAME	0x01
#define	W_SURNAME	0x02

	int w_record;			/* same values as ag_record */

	char *w_title;

	char *w_area;
	int w_areatype;
	char *w_geography;

	int w_output;
#define	W_EXPAND	0x01
#define	W_FULL		0x02
#define	W_SUMMARY	0x04
#define	W_SUBDISPLAY	0x08
};

char *eqstr(), *limits();
FILE *capture();

/*  */

char *whois_help[] = {
	"whois input-field [record-type] [area-designator] [output-control]",
	"    input-field is one of:",
	"\tname NAME\t\te.g., surname \"smith\", or fullname \"john smith\"",
	"\thandle HANDLE\t\te.g., handle @c=US@cn=Manager",
	"\tmailbox LOCAL@DOMAIN\te.g., mailbox postmaster@nisc.psi.net",
	"    record-type is one of:",
	"\tperson or -title NAME\te.g., -title scientist",
	"\torganization",
	"\tunit (a division under an organization)",
	"\trole (a role within an organization)",
	"\tlocality",
	"\tdsa (white pages server)",
	"    area-designator is one of:",
	"\t-org NAME\t\te.g., -org psi",
	"\t-unit NAME\t\te.g., -unit engineering",
	"\t-locality NAME\t\te.g., -locality rensselaer",
	"\t-area HANDLE\t\te.g., -area \"@c=US@o=Performance Systems...\"",
	"\t    and may be optionally followed by -geo HANDLE, e.g., -geo @c=GB",
	"    output-control is any of:",
	"\texpand\t   - give a detailed listing, followed by children",
	"\tsubdisplay - give a one-listing listing, followed by children",
	"\tfull\t   - give a detailed listing, even on ambiguous matches",
	"\tsummary\t   - give a one-line listing, even on unique matches",

	NULL
};

static int f_whois_aux();
static whois_aux();
static int test_arg();
static int f_ufn();

/*    WHOIS */

int
f_whois(vec)
	char **vec;
{
	if (strcmp(*vec, "whois") == 0)
		vec++;

	return (nametype > 1 ? f_ufn(vec) : f_whois_aux(vec));
}

/*  */

static int
f_whois_aux(vec)
	char **vec;
{
	int c, mailbox, multiple, result;
	register char *bp, *cp, *dp;
	char buffer[BUFSIZ], orgname[BUFSIZ];
	register struct area_guide *ag;
	struct whois ws;
	register struct whois *w = &ws;
	FILE *fp;

	bzero((char *) w, sizeof *w);

	while (cp = *vec++) {
	      postscan:;

		switch (*cp) {
		case '.':
			if (w->w_inputype != W_NULL) {
			      too_many_fields:;
				advise(NULLCP, "only one of NAME, HANDLE, or MAILBOX allowed");
				goto you_really_lose;
			}
			if (*++cp == NULL) {
				advise(NULLCP, "expecting NAME after \".\"");
				goto you_really_lose;
			}
			w->w_inputype = W_NAME;
			w->w_input = cp;
			break;

		case '!':
			if (w->w_inputype != W_NULL)
				goto too_many_fields;
			if (*++cp == NULL) {
				advise(NULLCP, "expecting HANDLE after \"!\"");
				goto you_really_lose;
			}
			goto got_handle;

		case '*':
			if (cp[1] == '*') {
				cp++;
				goto name_or_something;
			}
			w->w_output |= W_EXPAND;
			if (*++cp)
				goto postscan;
			break;

		case '~':
			w->w_output &= ~W_EXPAND;
			if (*++cp)
				goto postscan;
			break;

		case '|':
			w->w_output |= W_FULL;
			if (*++cp)
				goto postscan;
			break;

		case '$':
			w->w_output |= W_SUMMARY;
			if (*++cp)
				goto postscan;
			break;

		case '%':
			w->w_output |= W_SUBDISPLAY;
			if (*++cp)
				goto postscan;
			break;

		case '-':
			if (test_arg(cp, "-area", 4)) {
				result = W_NULL;

			      stuff_area:;
				if (w->w_area != NULL) {
					advise(NULLCP, "only one AREA specification allowed");
					goto you_really_lose;
				}
				if (*vec == NULL) {
					advise(NULLCP, "expecting %s after \"%s\"",
					       result != W_NULL ? "NAME" : "HANDLE", cp);
					goto you_really_lose;
				}
				if (*(dp = *vec) == '!')
					result = W_NULL;
				else {
					for (; *dp; dp++)
						if (!isdigit(*dp))
							break;
					if (!*dp)
						result = NULL;
				}
				w->w_area = *vec++, w->w_areatype = result;
				break;

			}
			if (test_arg(cp, "-expand", 4)) {
				w->w_output |= W_EXPAND;
				break;
			}
			if (test_arg(cp, "-full", 4)) {
				w->w_output |= W_FULL;
				break;
			}
			if (test_arg(cp, "-geography", 4)) {
				if (*vec == NULL) {
					advise(NULLCP, "expecting location after \"-geography\"");
					goto you_really_lose;
				}
				w->w_geography = *vec++;
				break;
			}
			if (test_arg(cp, "-help", 5))
				goto print_help;
			if (test_arg(cp, "-locality", 4)) {
				result = W_LOCALITY;
				goto stuff_area;
			}
			if (test_arg(cp, "-organization", 4)) {
				result = W_ORGANIZATION;
				goto stuff_area;
			}
			if (test_arg(cp, "-summary", 4)) {
				w->w_output |= W_SUMMARY;
				break;
			}
			if (test_arg(cp, "-subdisplay", 4)) {
				w->w_output |= W_SUBDISPLAY;
				break;
			}
			if (test_arg(cp, "-title", 4)) {
				if (*vec == NULL) {
					advise(NULLCP, "expecting something after \"-title\"");
					goto you_really_lose;
				}
				w->w_title = *vec++;
				break;
			}
			if (test_arg(cp, "-unit", 4)) {
				result = W_UNIT;
				goto stuff_area;
			}

			advise(NULLCP, "unknown switch: %s", cp);
		      you_really_lose:;
			if (mail) {
				(void) fprintf(stdfp, "\n\n");
				goto print_help;
			}
			return NOTOK;

		case 'd':
			if (strcmp(cp, "dsa"))
				goto name_or_something;
			if (w->w_record != W_NULL)
				goto too_many_fields;
			w->w_record = W_DSA;
			break;

		case 'e':
			if (strcmp(cp, "expand"))
				goto name_or_something;
			w->w_output |= W_EXPAND;
			break;

		case 'f':
			if (strcmp(cp, "full") == 0) {
				w->w_output |= W_FULL;
				break;
			}
			if (strcmp(cp, "fullname") == 0) {
				w->w_nametype = W_COMMONAME;
				goto name;
			}
			goto name_or_something;

		case 'h':
			if (strcmp(cp, "handle"))
				goto name_or_something;
			if (w->w_inputype != W_NULL)
				goto too_many_fields;
			if ((cp = *vec++) == NULL) {
				advise(NULLCP, "expecting HANDLE after \"handle\"");
				goto you_really_lose;
			}
		      got_handle:;
			if (!mail && strcmp(cp, "me") == 0) {
				if (mydn == NULL) {
					advise(NULLCP,
					       "who are you?  use the \"thisis\" command first...");
					return NOTOK;
				}
				cp = mydn;
			}
			w->w_inputype = W_HANDLE;
			w->w_input = cp;
			break;

		case 'l':
			if (strcmp(cp, "locality"))
				goto name_or_something;
			if (w->w_record != W_NULL)
				goto too_many_fields;
			w->w_record = W_LOCALITY;
			break;

		case 'm':
			if (strcmp(cp, "mailbox"))
				goto name_or_something;
			if (w->w_inputype != W_NULL)
				goto too_many_fields;
			if ((cp = *vec++) == NULL) {
				advise(NULLCP, "expecting MAILBOX after \"mailbox\"");
				goto you_really_lose;
			}
			w->w_inputype = W_MAILBOX;
			w->w_input = cp;
			break;

		case 'n':
			if (strcmp(cp, "name"))
				goto name_or_something;
		      name:;
			if (w->w_inputype != W_NULL)
				goto too_many_fields;
			if ((cp = *vec++) == NULL) {
				advise(NULLCP, "expecting NAME after \"%sname\"",
				       w->w_nametype == W_COMMONAME ? "full"
				       : w->w_nametype == W_SURNAME ? "sur" : "");
				goto you_really_lose;
			}
			w->w_inputype = W_NAME;
			w->w_input = cp;
			break;

		case 'o':
			if (strcmp(cp, "organization") && strcmp(cp, "org"))
				goto name_or_something;
			if (w->w_record != W_NULL)
				goto too_many_fields;
			w->w_record = W_ORGANIZATION;
			break;

		case 'p':
			if (strcmp(cp, "person"))
				goto name_or_something;
			if (w->w_record != W_NULL)
				goto too_many_fields;
			w->w_record = W_PERSON;
			break;

		case 'r':
			if (strcmp(cp, "role"))
				goto name_or_something;
			if (w->w_record != W_NULL)
				goto too_many_fields;
			w->w_record = W_ROLE;
			break;

		case 's':
			if (strcmp(cp, "subdisplay") == 0) {
				w->w_output |= W_SUBDISPLAY;
				break;
			}
			if (strcmp(cp, "summary") == 0) {
				w->w_output |= W_SUMMARY;
				break;
			}
			if (strcmp(cp, "surname") == 0) {
				w->w_nametype = W_SURNAME;
				goto name;
			}
			goto name_or_something;

		case 'u':
			if (strcmp(cp, "unit"))
				goto name_or_something;
			if (w->w_record != W_NULL)
				goto too_many_fields;
			w->w_record = W_UNIT;
			break;

		default:
		      name_or_something:;
			if (w->w_inputype != W_NULL)
				goto too_many_fields;
			for (dp = cp; *dp; dp++)
				if (!isdigit(*dp))
					break;
			if (!*dp)
				goto got_handle;
			if ((dp = index(cp, '@')) == NULL) {
				w->w_inputype = W_NAME;
				w->w_input = cp;
				break;
			}
			if (index(dp + 1, '@') || index(dp + 1, '=')) {
				w->w_inputype = W_HANDLE;
				w->w_input = cp;
				break;
			}
			if (!index(dp + 1, '.')) {
				if (w->w_area != W_NULL) {
					advise(NULLCP, "only one AREA specification allowed");
					goto you_really_lose;
				}

				*dp++ = NULL;

				w->w_inputype = W_NAME;
				w->w_input = cp;

				w->w_area = dp, w->w_areatype = W_ORGANIZATION;
				break;
			}
			w->w_inputype = W_MAILBOX;
			w->w_input = cp;
			break;
		}
	}

	if (w->w_nametype != W_NULL)
		switch (w->w_record) {
		default:
			advise(NULLCP, "record-type ignored with \"%sname\"",
			       w->w_nametype == W_COMMONAME ? "full" : "sur");
			/* and fall... */

		case W_NULL:
			w->w_record = W_PERSON;
			/* and fall... */

		case W_PERSON:
			break;
		}

	if (w->w_input && strcmp(w->w_input, "*") == 0)
		w->w_input = NULL, w->w_inputype = W_NULL;
	if (w->w_title && strcmp(w->w_title, "*") == 0)
		w->w_title = NULL;
	if ((w->w_input || w->w_title)
	    && w->w_area && strcmp(w->w_area, "*") == 0)
		w->w_area = NULL, w->w_areatype = W_NULL;

	if (w->w_title) {
		if (w->w_inputype == W_NULL)
			w->w_inputype = W_NAME;
		if (w->w_inputype != W_NAME) {
			advise(NULLCP, "title specification ignored with %s",
			       w->w_inputype == W_HANDLE ? "HANDLE" : "MAILBOX");
			w->w_title = NULL;
		} else if (w->w_record == W_NULL)
			w->w_record = W_PERSON;
	}

	if (w->w_inputype == W_HANDLE && w->w_geography) {
		advise(NULLCP, "geography specification ignored with HANDLE");
		w->w_geography = NULL;
	}
	if (w->w_geography)
		if (w->w_area == NULL) {
			w->w_area = w->w_geography, w->w_areatype = W_LOCALITY;
			w->w_geography = NULL;
		} else if (*w->w_geography == '!')
			w->w_geography++;

	if (w->w_inputype == W_HANDLE && w->w_area) {
		advise(NULLCP, "area specification ignored with HANDLE");
		w->w_area = NULL, w->w_areatype = W_NULL;
	}
	if (w->w_area)
		if (w->w_inputype == W_NULL) {
			if (*w->w_area != '!') {
				for (dp = w->w_area; *dp; dp++)
					if (!isdigit(*dp))
						break;
				w->w_inputype = *dp ? W_NAME : W_HANDLE, w->w_input = w->w_area;
			} else
				w->w_inputype = W_HANDLE, w->w_input = w->w_area + 1;
			w->w_record = w->w_areatype;
			w->w_area = NULL, w->w_areatype = W_NULL;
		} else if (*w->w_area == '!')
			w->w_area++;

	if (w->w_inputype == W_NULL) {
		register char **ap;

		if (w->w_record != W_NULL || w->w_output != W_NULL) {
			advise(NULLCP, "input-field missing");
			return NOTOK;
		}

	      print_help:;
		for (ap = whois_help; *ap; ap++)
			(void) fprintf(stdfp, "%s%s", *ap, EOLN);

		return OK;
	} else if (w->w_inputype == W_NAME && w->w_nametype == W_NULL)
		w->w_nametype = nametype ? W_SURNAME : W_COMMONAME;

	bp = buffer;

	mailbox = 0;
	if (w->w_areatype == W_NULL) {
		if (w->w_inputype == W_MAILBOX && w->w_area == NULL && (cp = index(w->w_input, '@'))
		    && !index(++cp, '*')) {
			(void) sprintf(bp, "fred -dm2dn %s", cp);
			bp += strlen(bp);
			mailbox = 1;

			goto multiple_searching;
		}

		return whois_aux(w);
	}

	(void) sprintf(bp,
		       "search %s -norelative -singlelevel -dontdereference -sequence default -types aliasedObjectName -value -nosearchalias ",
		       limits(-1));
	bp += strlen(bp);

	for (ag = areas; ag->ag_record; ag++)
		if (ag->ag_record == w->w_areatype)
			break;
	if (w->w_geography) {
		(void) sprintf(bp, "\"%s\" ", w->w_geography);
		bp += strlen(bp);

		w->w_geography = NULL;
	} else if (ag->ag_area) {
		(void) sprintf(bp, "\"%s\" ", ag->ag_area);
		bp += strlen(bp);
	}

	(void) sprintf(bp, "-filter \"objectClass=%s & %s%s\"",
		       ag->ag_class, ag->ag_rdn, eqstr(w->w_area, 0));
	bp += strlen(bp);

      multiple_searching:;
	if ((fp = capture(buffer)) == NULL)
		return NOTOK;

	if (interrupted) {
	      you_lose:;
		(void) fclose(fp);
		return NOTOK;
	}

	w->w_area = orgname, w->w_areatype = W_NULL;

	multiple = 0;
	while (fgets(buffer, sizeof buffer, fp) && !interrupted) {
		if ((cp = index(buffer, '\n')) == NULL) {
			advise(NULLCP, "internal error(1)");
			goto you_lose;
		}
		*cp = NULL;

		if (!isdigit(*buffer)) {
			(void) fprintf(stderr, "%s\n", buffer);
			continue;
		}

		if ((cp = index(buffer, ' ')) == NULL) {
			advise(NULLCP, "internal error(2)");
			goto you_lose;
		}
		*cp++ = NULL;
		while (*cp == ' ')
			cp++;

		if (multiple == 0 && (c = getc(fp)) != EOF) {
			(void) ungetc(c, fp);
			multiple = 1;
		}

		if (mailbox && !index(cp, '@')) {
			advise(NULLCP, "Unable to resolve domain beyond national level.");
			continue;
		}
		if (multiple && query && !network)
			switch (ask("try %s [y/n] ? ", cp)) {
			case NOTOK:
				continue;

			case OK:
			default:
				break;

			case DONE:
				goto out;
		} else if (verbose) {
			(void) fprintf(stdfp, "Trying @%s ...\n", cp);
			(void) fflush(stdfp);
		}

		(void) sprintf(orgname, "@%s", cp);
		(void) whois_aux(w);

		(void) fprintf(stdfp, EOLN);
		(void) fflush(stdfp);
	}

      out:;
	(void) fclose(fp);

	return OK;
}

/*  */

static
whois_aux(w)
	register struct whois *w;
{
	register char *bp, *cp;
	char *handle, buffer[BUFSIZ];
	register struct area_guide *ag;

	for (ag = areas; ag->ag_record; ag++)
		if (ag->ag_record == w->w_record)
			break;

	bp = buffer;
	switch (w->w_inputype) {
	case W_NULL:
	default:
		advise(NULLCP, "internal error(8)");
		return NOTOK;

	case W_HANDLE:
		(void) sprintf(bp, "showentry \"%s\" %s -fred -dontdereference",
			       w->w_input, limits(0));
		bp += strlen(bp);
		goto options;

	case W_MAILBOX:
		(void) sprintf(bp, "search %s -fred ", limits(1));
		bp += strlen(bp);

		if (w->w_area) {
			(void) sprintf(bp, "\"%s\" ", w->w_area);
			bp += strlen(bp);
		}

		(void) sprintf(bp, "-subtree -filter \"");
		bp += strlen(bp);

		cp = w->w_input;
		if (*cp == '@')
			(void) sprintf(bp, "mail=*%s", cp);
		else if (*(cp + strlen(cp) - 1) == '@' || !index(cp, '@'))
			(void) sprintf(bp, "mail=%s*", cp);
		else if (index(cp, '*'))
			(void) sprintf(bp, "mail=%s", cp);
		else
			(void) sprintf(bp, "(mail=%s | otherMailbox=internet$%s)", cp, cp);
		bp += strlen(bp);
		break;

	case W_NAME:
		if (cp = w->w_input) {
			cp += strlen(cp) - 1;
			if (*cp == '.')
				*cp = '*';
		}

		(void) sprintf(bp, "search %s -%s ", limits(1), kflag ? "show" : "fred");
		bp += strlen(bp);

		if (w->w_area) {
			(void) sprintf(bp, "\"%s\" -subtree ", w->w_area);
			bp += strlen(bp);
		} else if (w->w_geography) {
			(void) sprintf(bp, "\"%s\" ", w->w_geography);
			bp += strlen(bp);
		} else if (ag->ag_area) {
			(void) sprintf(bp, "\"%s\" %s ", ag->ag_area, ag->ag_search);
			bp += strlen(bp);
		} else {
			(void) sprintf(bp, "-subtree ");
			bp += strlen(bp);
		}
		(void) sprintf(bp, "-filter \"");
		bp += strlen(bp);

		handle = eqstr(w->w_input, 0);
		switch (w->w_record) {
		case W_NULL:
		case W_PERSON:
			if (handle) {
				if (w->w_title
				    && (w->w_record == W_NULL || w->w_nametype == W_SURNAME)) {
					(void) sprintf(bp, "( ");
					bp += strlen(bp);
				}
				if (w->w_record == W_NULL) {
					(void) sprintf(bp, "o%s | ou%s | l%s | ",
						       handle, handle, handle, handle);
					bp += strlen(bp);
				}

				if (w->w_nametype == W_SURNAME) {
					if (w->w_record == W_PERSON && !w->w_title) {
						(void) sprintf(bp, "( ");
						bp += strlen(bp);
					}
					(void) sprintf(bp, "surname%s | mail=%s@* ",
						       eqstr(w->w_input, 1), w->w_input);
					bp += strlen(bp);
					if (w->w_record == W_PERSON && !w->w_title) {
						(void) sprintf(bp, ") ");
						bp += strlen(bp);
					}
				} else {
					(void) sprintf(bp, "cn%s ", handle);
					bp += strlen(bp);
				}

				if (w->w_title
				    && (w->w_record == W_NULL || w->w_nametype == W_SURNAME)) {
					(void) sprintf(bp, ") ");
					bp += strlen(bp);
				}
			}
			if (w->w_title) {
				(void) sprintf(bp, "%stitle%s",
					       handle ? "& " : "", eqstr(w->w_title, 1));
				bp += strlen(bp);
			}
			break;

		default:
			if (strcmp(handle, "=*")) {
				(void) sprintf(bp, "%s%s", ag->ag_rdn, handle);
				bp += strlen(bp);
			}
			break;
		}
		break;
	}

	if (w->w_record == W_PERSON || (w->w_record != W_NULL && strcmp(handle, "=*"))) {
		(void) sprintf(bp, " & ");
		bp += strlen(bp);
	}

	if (w->w_record != W_NULL) {
		(void) sprintf(bp, "objectClass=%s", ag->ag_class);
		bp += strlen(bp);
	}

	(void) sprintf(bp, "\"");
	bp += strlen(bp);

      options:;
	if (w->w_output & W_EXPAND) {
		(void) sprintf(bp, " -expand");
		bp += strlen(bp);
	}
	if (w->w_output & W_FULL) {
		(void) sprintf(bp, " -full");
		bp += strlen(bp);
	}
	if (w->w_output & W_SUMMARY) {
		(void) sprintf(bp, " -summary");
		bp += strlen(bp);
	}
	if (w->w_output & W_SUBDISPLAY) {
		(void) sprintf(bp, " -subdisplay");
		bp += strlen(bp);
	}

	(void) sprintf(bp, " -sequence %s", "default");
	bp += strlen(bp);

	return dish(buffer, 0);
}

/*  */

static int
test_arg(user, full, minlen)
	char *user, *full;
	int minlen;
{
	int i;

	if ((i = strlen(user)) < minlen || i > (int) strlen(full)
	    || strncmp(user, full, i))
		return 0;

	return 1;
}

/*  */

static char *
eqstr(s, exact)
	char *s;
	int exact;
{
	static char buffer[BUFSIZ];

	if (s == NULL)
		return NULL;

	if (index(s, '*'))
		(void) sprintf(buffer, "=%s", s);
	else if (soundex)
		(void) sprintf(buffer, "~=%s", s);
	else if (exact)
		(void) sprintf(buffer, "=%s", s);
	else
		(void) sprintf(buffer, "=*%s*", s);

	return buffer;
}

/*  */

static char *
limits(isearch)
	int isearch;
{
	register char *bp;
	static char buffer[100];

	bp = buffer;

	if (phone) {
		(void) strcpy(bp, "-phone ");
		bp += strlen(bp);
	}
#ifdef	notdef			/* don't do this! */
	if (isearch) {
		(void) strcpy(bp, "-searchalias ");
		bp += strlen(bp);
	}
#endif

	(void) strcpy(bp, "-nosizelimit ");
	bp += strlen(bp);

	if (timelimit > 0)
		(void) sprintf(bp, "-timelimit %d", timelimit);
	else
		(void) strcpy(bp, "-notimelimit");
	bp += strlen(bp);

	if (isearch >= 0 && (network || oneshot)) {
		(void) strcpy(bp, " -nofredseq");
		bp += strlen(bp);
	}

	return buffer;
}

/*  */

static FILE *
capture(command)
	char *command;
{
	int savnet, savpage;
	char tmpfil[BUFSIZ];
	FILE *fp, *savfp;

	(void) strcpy(tmpfil, "/tmp/fredXXXXXX");
	(void) unlink(mktemp(tmpfil));

	if ((fp = fopen(tmpfil, "w+")) == NULL) {
		advise(tmpfil, "unable to create");
		return NULL;
	}
	(void) unlink(tmpfil);

	savfp = stdfp, stdfp = fp;
	savnet = network, network = 0;
	savpage = dontpage, dontpage = 1;

	(void) dish(command, 0);

	dontpage = savpage;
	network = savnet;
	stdfp = savfp;

	rewind(fp);

	return fp;
}

/*  */

static int
f_ufn(vec)
	char **vec;
{
	char *cp, buffer[BUFSIZ];
	static int lastq = -1;

	if ((cp = vec[0]) == NULL || strcmp(cp, "-help") == 0) {
		(void) fprintf(stdfp, "whois name...\n");
		(void) fprintf(stdfp, "    find something in the white pages\n");

		return OK;
	}

	if (!test_ufn(cp)) {
		(void) strcpy(buffer, cp);
		(void) str2vec(buffer, vec);
		return f_whois_aux(vec);
	}

	(void) sprintf(buffer, "fred -ufn %s-options %x,%s",
		       phone ? "-phone," : "", ufn_options, cp);

	if (lastq != area_quantum) {
		(void) sync_ufnrc();

		lastq = area_quantum;
	}

	return dish(buffer, 0);
}

/*  */

int
test_ufn(cp)
	char *cp;
{
	register char *dp;

	if (*(dp = cp) == '!')
		dp++;
	for (; *dp; dp++)
		if (!isdigit(*dp))
			break;
	if (!*dp || *dp == '-')	/* a handle, or oldstyle */
		return 0;

	if (!index(dp = cp, ',')) {
		while (dp = index(dp, ' '))
			if (*++dp == '-')
				return 0;
		if (index(cp, '@'))
			return 0;
	}

	if (strcmp(cp, "!me") == 0)
		return 0;

	return 1;
}