/****************************************************************************
 *                 Basic manipulation of XStringSet objects                 *
 *                           Author: Herve Pages                            *
 ****************************************************************************/
#include "Biostrings.h"
#include "IRanges_interface.h"

static int debug = 0;

SEXP debug_XStringSet_class()
{
#ifdef DEBUG_BIOSTRINGS
	debug = !debug;
	Rprintf("Debug mode turned %s in file %s\n",
		debug ? "on" : "off", __FILE__);
#else
	Rprintf("Debug mode not available in file %s\n", __FILE__);
#endif
	return R_NilValue;
}


/****************************************************************************
 * C-level slot getters.
 *
 * Be careful that these functions do NOT duplicate the returned slot.
 * Thus they cannot be made .Call() entry points!
 */

/* Not strict "slot getters" but very much like. */

int _get_XStringSet_length(SEXP x)
{
	return get_XVectorList_length(x);
}

SEXP _get_XStringSet_width(SEXP x)
{
	return get_XVectorList_width(x);
}

const char *_get_XStringSet_xsbaseclassname(SEXP x)
{
	return get_List_elementType(x);
}


/****************************************************************************
 * C-level abstract getters.
 */

cachedXStringSet _cache_XStringSet(SEXP x)
{
	return cache_XVectorList(x);
}

int _get_cachedXStringSet_length(const cachedXStringSet *cached_x)
{
	return get_cachedXVectorList_length(cached_x);
}

cachedCharSeq _get_cachedXStringSet_elt(const cachedXStringSet *cached_x, int i)
{
	return get_cachedXRawList_elt(cached_x, i);
}


/****************************************************************************
 * C-level slot setters.
 *
 */

/* WARNING: x@ranges@NAMES is modified in-place! */
void _set_XStringSet_names(SEXP x, SEXP names)
{
	set_XVectorList_names(x, names);
	return;
}


/****************************************************************************
 * From CHARACTER to XStringSet and vice-versa.
 */

/* --- .Call ENTRY POINT --- */
SEXP new_XStringSet_from_CHARACTER(SEXP classname, SEXP element_type,
		SEXP x, SEXP start, SEXP width, SEXP lkup)
{
	SEXP ans, x_elt;
	cachedXVectorList cached_ans;
	const int *lkup0;
	int ans_length, lkup_length, i;
	cachedCharSeq cached_ans_elt;

	PROTECT(ans = alloc_XRawList(CHAR(STRING_ELT(classname, 0)),
				     CHAR(STRING_ELT(element_type, 0)),
				     width));
	cached_ans = cache_XVectorList(ans);
	ans_length = get_cachedXVectorList_length(&cached_ans);
	if (lkup == R_NilValue) {
		lkup0 = NULL;
	} else {
		lkup0 = INTEGER(lkup);
		lkup_length = LENGTH(lkup);
	}
	for (i = 0; i < ans_length; i++) {
		cached_ans_elt = get_cachedXRawList_elt(&cached_ans, i);
		x_elt = STRING_ELT(x, i);
		if (x_elt == NA_STRING) {
			UNPROTECT(1);
			error("input sequence %d is NA", i + 1);
		}
		_copy_CHARSXP_to_cachedCharSeq(&cached_ans_elt, x_elt,
				INTEGER(start)[i], lkup0, lkup_length);
	}
	UNPROTECT(1);
	return ans;
}

/* --- .Call ENTRY POINT --- */
SEXP new_CHARACTER_from_XStringSet(SEXP x, SEXP lkup)
{
	cachedXStringSet cached_x;
	int x_length, i;
	SEXP ans, ans_elt;
	cachedCharSeq cached_x_elt;

	cached_x = cache_XVectorList(x);
	x_length = get_cachedXVectorList_length(&cached_x);
	PROTECT(ans = NEW_CHARACTER(x_length));
	for (i = 0; i < x_length; i++) {
		cached_x_elt = get_cachedXRawList_elt(&cached_x, i);
		PROTECT(ans_elt = _new_CHARSXP_from_cachedCharSeq(
					&cached_x_elt, lkup));
		SET_STRING_ELT(ans, i, ans_elt);
		UNPROTECT(1);
	}
	UNPROTECT(1);
	return ans;
}


/****************************************************************************
 * Creating a set of sequences (RoSeqs struct) from an XStringSet object.
 */

RoSeqs _new_RoSeqs_from_XStringSet(int nelt, SEXP x)
{
	RoSeqs seqs;
	cachedXStringSet cached_x;
	cachedCharSeq *elt1;
	int i;

	if (nelt > _get_XStringSet_length(x))
		error("_new_RoSeqs_from_XStringSet(): "
		      "'nelt' must be <= '_get_XStringSet_length(x)'");
	seqs = _alloc_RoSeqs(nelt);
	cached_x = _cache_XStringSet(x);
	for (i = 0, elt1 = seqs.elts; i < nelt; i++, elt1++)
		*elt1 = _get_cachedXStringSet_elt(&cached_x, i);
	return seqs;
}


/****************************************************************************
 * unlist().
 */

/* Note that XStringSet_unlist() is VERY similar to XString_xscat().
   Maybe both could be unified under a fast c() for XRaw objects. */
SEXP XStringSet_unlist(SEXP x)
{
	SEXP ans_tag, ans;
	int x_length, ans_length, tag_offset, i;
	cachedXStringSet cached_x;
	cachedCharSeq xx;

	cached_x = _cache_XStringSet(x);
	x_length = _get_cachedXStringSet_length(&cached_x);

	/* 1st pass: determine 'ans_length' */
	ans_length = 0;
	for (i = 0; i < x_length; i++) {
		xx = _get_cachedXStringSet_elt(&cached_x, i);
		ans_length += xx.length;
	}
	PROTECT(ans_tag = NEW_RAW(ans_length));

	/* 2nd pass: fill 'ans' */
	tag_offset = 0;
	for (i = 0; i < x_length; i++) {
		xx = _get_cachedXStringSet_elt(&cached_x, i);
		Ocopy_bytes_to_i1i2_with_lkup(tag_offset,
				tag_offset + xx.length - 1,
				(char *) RAW(ans_tag), LENGTH(ans_tag),
				xx.seq, xx.length,
				NULL, 0);
		tag_offset += xx.length;
	}

	/* Make 'ans' */
	PROTECT(ans = new_XRaw_from_tag(_get_XStringSet_xsbaseclassname(x),
					ans_tag));
	UNPROTECT(2);
	return ans;
}

