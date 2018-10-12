#ifndef _LINUX_RAI_H
#define _LINUX_RAI_H

/*
 * These document the behaviour any arch implementation of _rai_*
 * should have, and can be used by those in cases the arch does not
 * want to handle (e.g. _rai_load of a 2-byte quantity).
 */
#define _rai_load_fallback(var)                       (var)
#define _rai_bucket_shift_fallback(base, shift, hash) (&(base)[(hash) >> (shift)])
#define _rai_bucket_mask_fallback(base, mask, hash)   (&(base)[(hash) & (mask)])

#ifdef CONFIG_ARCH_HAS_RAI
#include <asm/rai.h>
void update_rai_access(void);
#else
static inline void update_rai_access(void) {}
#endif

#ifdef MODULE /* don't bother with modules for now */
#undef _rai_load
#undef _rai_bucket_shift
#undef _rai_bucket_mask
#endif

/* Make sure all _rai_* are defined. */
#ifndef _rai_load
#define _rai_load _rai_load_fallback
#endif
#ifndef _rai_bucket_shift
#define _rai_bucket_shift _rai_bucket_shift_fallback
#endif
#ifndef _rai_bucket_mask
#define _rai_bucket_mask _rai_bucket_mask_fallback
#endif


/* 
 * The non-underscored rai_* are property of this header, so that it
 * can do tricks like defining debugging versions. Usually, it just
 * defines rai_foo as _rai_foo, with the latter being guaranteed to be
 * defined by the above logic.
 */
#if defined(CONFIG_RAI_DEBUG)

#include <bug.h>

#define rai_warn(what, expect, got)					\
	WARN_ONCE(expect != got,					\
		  "%s:%d: %s() returned %*phN, expected %*phN\n",	\
		  __FILE__, __LINE__, what,				\
		  (int)sizeof(got), &(got),				\
		  (int)sizeof(expect), &(expect))

#define rai_load(var) ({						\
		typeof(var) v1 = _rai_load_fallback(var);		\
		typeof(var) v2 = _rai_load(var);			\
		rai_warn("rai_load", v1, v2);				\
		(v1); /* chicken */					\
	})

#define rai_bucket_shift(base, shift, hash) ({				\
		typeof(hash) h = (hash);				\
		typeof(base) b1 = _rai_bucket_shift_fallback(base, shift, h); \
		typeof(base) b2 = _rai_bucket_shift(base, shift, h);	\
		rai_warn("rai_bucket_shift", b1, b2);			\
		(b1);							\
	})

#define rai_bucket_mask(base, mask, hash) ({				\
		typeof(hash) h = (hash);				\
		typeof(base) b1 = _rai_bucket_mask_fallback(base, mask, h); \
		typeof(base) b2 = _rai_bucket_mask(base, mask, h);	\
		rai_warn("rai_bucket_mask", b1, b2);			\
		(b1);							\
	})
#else
#define rai_load         _rai_load
#define rai_bucket_shift _rai_bucket_shift
#define rai_bucket_mask  _rai_bucket_mask
#endif

#endif /* _LINUX_RAI_H */
