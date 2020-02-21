#include <linux/errno.h>
#include <linux/limits.h>
#include <linux/parse_integer.h>
#include <linux/string.h>
#include <linux/types.h>

static const char*
fixup_base(const char *s, unsigned *base)
{
	if (*base == 0) {
		if (s[0] == '0') {
			*base = ((s[1] | 0x20) == 'x') ? 16 : 8;
		} else {
			*base = 10;
		}
	}
	if (*base == 16 && s[0] == '0' && (s[1] | 0x20) == 'x')
		s += 2;
	return s;
}

static unsigned
digit_value(char d)
{
	if ('0' <= d && d <= '9')
		return d - '0';
	d |= 0x20;
	if ('a' <= d && d <= 'f')
		return d - 'a' + 10;
	return UINT_MAX;
}

/*
 * Parse as much as possible from the given string, and return the
 * (possibly wrapped/overflowed) result in *dst. Return -EINVAL if no
 * digits are found. Note: If the buffer begins with "0x" but no hex
 * digits follow, this is an error if base is 0 or 16, but results in
 * one consumed digit (and a result of 0) if base is 8 or 10.
 */
static int
do_parse_integer(const char *buf, unsigned base, u64 *dst, bool *overflow)
{
	u64 acc = 0, max_acc;
	const char *s, *s0;
	bool of = false;
#if 1
	static const u64 max_acc_table[] = {
		[0] = U64_MAX/8,
		[1] = U64_MAX/10,
		[4] = U64_MAX/16,
	};
#endif

	s = fixup_base(buf, &base);
	s0 = s;

	/*
	 * The product acc*base overflows if and only if the
	 * accumulator is greater than max_acc == U64_MAX/base. For
	 * now, avoid the division at the cost of a 40-byte lookup
	 * table and some simply arithmetic on base - alternatively,
	 * one could use a few ternary operators.
	 *
	 * Perhaps one could instead just rely on the builtin overflow
	 * checkers provided by gcc/clang. Or one could create a
	 * helper for each possible value of base.
	 */
#if 1
	max_acc = max_acc_table[(base-8)/2];
#elif 0
	max_acc = U64_MAX / base;
#elif 0
	max_acc =
		(base == 16) ? U64_MAX/16 :
		(base == 10) ? U64_MAX/16 :
		U64_MAX/8;
#endif

	while (1) {
		unsigned d = digit_value(*s);

		if (d >= base)
			break;
		/*
		 * This should really be "of ||= (acc > max_acc);",
		 * but that operator doesn't exist. |= works just as
		 * well.
		 */
		of |= (acc > max_acc);
		acc *= base;
		of |= (acc + d < acc);
		acc += d;
		++s;
	}
	if (s == s0)
		return -EINVAL;
	*dst = acc;
	*overflow = of;
	return s - buf;
}

#define BASE_MASK 0x000000ffU
#define TYPE_MASK 0x0000ff00U
#define FLAG_MASK 0xffff0000U
#define OVERFLOW_FLAGS (PARSE_INTEGER_SATURATE | PARSE_INTEGER_WRAP)

#define VALID_FLAGS (				\
	PARSE_INTEGER_LEAD_WS    |		\
	PARSE_INTEGER_TRAIL_WS   |		\
	PARSE_INTEGER_NEWLINE    |		\
	PARSE_INTEGER_ZERO_ON_OK |		\
	PARSE_INTEGER_ALL        |		\
	PARSE_INTEGER_SATURATE   |		\
	PARSE_INTEGER_WRAP       |		\
	0)

/*
 * Checking for overflow and doing the saturation is easier with a few
 * lookup tables. If there's no minus sign, we compare the parsed
 * value to a number from max_positive_values, otherwise to one from
 * max_negative_values.
 */
static const u64 max_positive_values[] = {
	[_PARSE_INTEGER_U8]  = U8_MAX,
	[_PARSE_INTEGER_U16] = U16_MAX,
	[_PARSE_INTEGER_U32] = U32_MAX,
	[_PARSE_INTEGER_U64] = U64_MAX,
	[_PARSE_INTEGER_S8]  = S8_MAX,
	[_PARSE_INTEGER_S16] = S16_MAX,
	[_PARSE_INTEGER_S32] = S32_MAX,
	[_PARSE_INTEGER_S64] = S64_MAX,
};
static const u64 max_negative_values[] = {
	[_PARSE_INTEGER_U8]  = 0,
	[_PARSE_INTEGER_U16] = 0,
	[_PARSE_INTEGER_U32] = 0,
	[_PARSE_INTEGER_U64] = 0,
	[_PARSE_INTEGER_S8]  = ((u64)1) << 7,
	[_PARSE_INTEGER_S16] = ((u64)1) << 15,
	[_PARSE_INTEGER_S32] = ((u64)1) << 31, /* can't do -INT32_MIN */
	[_PARSE_INTEGER_S64] = ((u64)1) << 63,
};

/*
 * The appropriate bound can also be determined with a little
 * computation, avoiding 128 bytes of lookup tables (but at the
 * expense of a little extra code). It also relies on the width and
 * signedness of the corresponding type being easily expressible in
 * terms of the _PARSE_INTEGER_* constants. Currently, the rules are
 * that is_signed_type(T) == _PARSE_INTEGER_T & 1 and sizeof(T) == 1
 * << (_PARSE_INTEGER_T >> 1) (add 3 to the shift for the size in
 * bits).
 */

#if 0
#define compute_bound(neg, type) ({			\
	u64 sign = type & _PARSE_INTEGER_SIGN;		\
	int width = 1 << (3 + (type >> 1));		\
	neg ?						\
		(((u64)1) << (width - 1)) & (-sign) :	\
		(~(u64)0) >> (64 - width + sign);	\
})
#endif

static void
assign_value(void *dst, u64 val, unsigned type)
{
	/*
	 * FIXME: Is there some odd platform with sizeof(long) == 8,
	 * alignof(long) == 4 and alignof(long long) == 8, so we can
	 * risk being passed a long* that is only 4-byte aligned and
	 * then access it as a long long that requires 8-byte
	 * alignment?
	 */
	switch (type) {
	case _PARSE_INTEGER_U8:
		*(u8*)dst = val;
		break;
	case _PARSE_INTEGER_U16:
		*(u16*)dst = val;
		break;
	case _PARSE_INTEGER_U32:
		*(u32*)dst = val;
		break;
	case _PARSE_INTEGER_U64:
		*(u64*)dst = val;
		break;
	case _PARSE_INTEGER_S8:
		*(s8*)dst = val;
		break;
	case _PARSE_INTEGER_S16:
		*(s16*)dst = val;
		break;
	case _PARSE_INTEGER_S32:
		*(s32*)dst = val;
		break;
	case _PARSE_INTEGER_S64:
		*(s64*)dst = val;
		break;
	}
}

static int
check_base(unsigned base)
{
	switch (base) {
	case 0:
	case 8:
	case 10:
	case 16:
		return 0;
	default:
		return -EINVAL;
	}
}

static int
check_type(unsigned type)
{
	switch (type) {
	case _PARSE_INTEGER_U8:
	case _PARSE_INTEGER_U16:
	case _PARSE_INTEGER_U32:
	case _PARSE_INTEGER_U64:
	case _PARSE_INTEGER_S8:
	case _PARSE_INTEGER_S16:
	case _PARSE_INTEGER_S32:
	case _PARSE_INTEGER_S64:
		return 0;
	default:
		return -EINVAL;
	}
}

static int
check_flags(unsigned flags)
{
	if (flags & ~VALID_FLAGS)
		return -EINVAL;
	if ((flags & OVERFLOW_FLAGS) == OVERFLOW_FLAGS)
		return -EINVAL;
	return 0;
}

int __parse_integer(const char *buf, unsigned b_t_f, void *dst)
{
	unsigned base = b_t_f & BASE_MASK;
	unsigned type = (b_t_f & TYPE_MASK) >> _PARSE_INTEGER_TYPE_SHIFT;
	unsigned flags = b_t_f & FLAG_MASK;
	const char *s = buf;
	bool neg = false;
	int rv;
	u64 val, bound;
	bool overflow;

	/* Basic input validation. */
	rv = check_base(base);
	if (rv)
		return rv;

	rv = check_type(type);
	if (rv)
		return rv;

	rv = check_flags(flags);
	if (rv)
		return rv;

	/* Now start parsing. */
	if (flags & PARSE_INTEGER_LEAD_WS)
		s = skip_spaces(s);

	/*
	 * Deal with an optional leading sign. -0 is ok also for
	 * unsigned destinations, and -1 may also be ok if _WRAP or
	 * _SATURATE was given. So for now just record whether a sign
	 * was given.
	 */
	if (*s == '-') {
		neg = true;
		++s;
	} else if (*s == '+') {
		++s;
	}

	/* Now do the actual parsing. */
	rv = do_parse_integer(s, base, &val, &overflow);
	if (rv < 0)
		return rv;
	s += rv;

	bound = (neg ? max_negative_values : max_positive_values)[type];
	/*
	 * If we overflowed a u64 we're certainly out of range for
	 * whatever type the caller passed. Otherwise, the
	 * mathematical integer represented by the given string lies
	 * in -U64_MAX .. U64_MAX, so compare the absolute value
	 * (which we still have in val) to the appropriate bound.
	 */
	if (overflow || val > bound) {
		/* If the user hasn't indicated how to handle overflow, return an error. */
		if (!(flags & OVERFLOW_FLAGS)) {
			return -ERANGE;
		} else if (flags & PARSE_INTEGER_SATURATE) {
			val = bound;
		} else {
			/* PARSE_INTEGER_WRAP, nothing to do. */
		}
	}

	if (neg)
		val = -val;

	if (flags & PARSE_INTEGER_NEWLINE && *s == '\n')
		++s;
	else if (flags & PARSE_INTEGER_TRAIL_WS)
		s = skip_spaces(s);

	/* For PARSE_INTEGER_ALL, the trailing \0 should also count as 'consumed'. */
	if (flags & PARSE_INTEGER_ALL && *s++ != '\0')
		return -EINVAL;

	/*
	 * OK, we've done all appropriate range checking/clamping and
	 * validation according to the user's wishes. The appropriate
	 * low-order bits of val contain the value to be stored.
	 */
	assign_value(dst, val, type);

	if (flags & PARSE_INTEGER_ZERO_ON_OK)
		return 0;
	return s - buf;
}
