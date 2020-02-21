#ifndef LINUX_PARSE_INTEGER_H
#define LINUX_PARSE_INTEGER_H

#include <linux/build_bug.h>
#include <linux/compiler.h>
#include <linux/overflow.h> /* is_signed_type */

/**
 * int parse_integer(const char *buf, unsigned base, T *dst);
 * int parse_integer(const char *buf, unsigned base, T *dst, unsigned flags);
 *
 * Parse an ascii representation of an integer and place the result in
 * @dst.
 *
 * Returns: Number of characters consumed, or a negative errno value
 * in case of error. In the latter case, nothing is written to *@dst.
 *
 * @buf: nul-terminated string to be parsed
 *
 * @base: the base to use for interpreting @buf. Valid values are 8,
 * 10, 16 or 0; the latter means auto-detect (with the usual
 * convention: leading 0x means hex, leading 0 means octal, otherwise
 * decimal). In case of hexadecimal, both upper- and lowercase letters
 * are allowed (both for the digits and the 0x prefix).
 *
 * @dst: pointer to destination variable. T must be a signed or
 * unsigned integer type of size 1, 2, 4 or 8 (and not _Bool). Passing
 * a wrong type will likely manifest itself as an obscure compiler
 * error.
 *
 * The behaviour can be modified by a few flags, which can either be
 * OR'ed with the base argument or, if you think that's cleaner,
 * supplied as an optional fourth argument. [We use a little macro
 * magic to make the flags argument optional - the generated code
 * should be the same in either case.]
 *
 * These flags can be used to make the parsing as strict or lax as the
 * situation calls for, and can also help eliminate boilerplate
 * (whitespace-skipping, for example). You should probably create your
 * own helper(s) on top of parse_integer with the semantics you want,
 * instead of cluttering your source with weird flag combinations.
 *
 * %PARSE_INTEGER_LEAD_WS         allow and consume leading whitespace
 * %PARSE_INTEGER_TRAIL_WS        allow and consume trailing whitespace
 * %PARSE_INTEGER_ZERO_ON_OK      return 0 when ok instead of #characters consumed
 * %PARSE_INTEGER_ALL             require that the entire string is consumed
 * %PARSE_INTEGER_SATURATE        allow over/underflow, saturate to min or max for the type in question
 * %PARSE_INTEGER_WRAP            allow over/underflow, just do mod 2^N wrapping
 *
 * %PARSE_INTEGER_SATURATE and %PARSE_INTEGER_WRAP are obviously
 * mutually exclusive.
 *
 * If %PARSE_INTEGER_ALL is given, the trailing \0 is included in the
 * count of characters consumed.
 *
 * Possible errors:
 *
 * -EINVAL: base was not 0, 8, 10 or 16.
 * -EINVAL: an unknown flag was given.
 * -EINVAL: both _SATURATE and _WRAP were given.
 * -EINVAL: _ALL was given, but we did not consume the entire string.
 * -EINVAL: no valid integer found.
 * -ERANGE: the value is not representable in the given type.
 *
 * Based on original idea by Alexey Dobriyan.
 */
#define parse_integer(buf, base, dst...)				\
	_parse_integer1(buf, base, dst, 0)

/*
 * parse_integer and the PARSE_INTEGER_* flags are the public
 * interfaces. The leading underscore versions (_parse_integer* and
 * _PARSE_INTEGER_*) are for internal use.
 */

/*
 * This is the magic that allows the flags argument to be optional;
 * strip of a fifth argument if present.
 */
#define _parse_integer1(buf, base, dst, flags, ...)	\
	_parse_integer2(buf, base, dst, flags)


enum {
	/* allow and consume leading whitespace */
	PARSE_INTEGER_LEAD_WS    = 0x010000,
	 /* allow and consume trailing whitespace */
	PARSE_INTEGER_TRAIL_WS   = 0x020000,
	/* for convenience */
	PARSE_INTEGER_WS         = PARSE_INTEGER_LEAD_WS | PARSE_INTEGER_TRAIL_WS,
	/* allow and consume a single trailing newline */
	PARSE_INTEGER_NEWLINE    = 0x040000,
	/* return 0 when ok instead of #characters consumed */
	PARSE_INTEGER_ZERO_ON_OK = 0x080000,
	/* require that the entire string is consumed */
	PARSE_INTEGER_ALL        = 0x100000,
	/* allow over/underflow, saturate to min or max for the type in question */
	PARSE_INTEGER_SATURATE   = 0x200000,
	/* allow over/underflow, just do mod 2^N wrapping */
	PARSE_INTEGER_WRAP       = 0x400000,
};

/*
 * Implementation details follow.
 *
 * Only the above is part of the public interface.
 */

enum {
	_PARSE_INTEGER_TYPE_SHIFT = 8,
	_PARSE_INTEGER_SIGN       = 1,

	_PARSE_INTEGER_U8         = 0,
	_PARSE_INTEGER_S8         = _PARSE_INTEGER_U8  + _PARSE_INTEGER_SIGN,
	_PARSE_INTEGER_U16        = 2,
	_PARSE_INTEGER_S16        = _PARSE_INTEGER_U16 + _PARSE_INTEGER_SIGN,
	_PARSE_INTEGER_U32        = 4,
	_PARSE_INTEGER_S32        = _PARSE_INTEGER_U32 + _PARSE_INTEGER_SIGN,
	_PARSE_INTEGER_U64        = 6,
	_PARSE_INTEGER_S64        = _PARSE_INTEGER_U64 + _PARSE_INTEGER_SIGN,

	/*
	 * This should leave room for expanding to u128/s128, but I
	 * doubt that'll ever be relevant.
	 */
};

#define _PARSE_INTEGER_TYPE(T) ((	    				\
	(is_signed_type(T) ? _PARSE_INTEGER_SIGN : 0) +			\
	(sizeof(T) == 1 ? _PARSE_INTEGER_U8 :				\
	sizeof(T) == 2 ? _PARSE_INTEGER_U16 :				\
	sizeof(T) == 4 ? _PARSE_INTEGER_U32 :				\
	sizeof(T) == 8 ? _PARSE_INTEGER_U64 :				\
	 0)								\
) << _PARSE_INTEGER_TYPE_SHIFT)

/*
 * _Bool is not allowed because that's just crazy, and (plain) char is
 * not allowed because its signedness is implementation-defined.
 */
#define _parse_integer2(buf, base, dst, flags) ({			\
	typeof(&((dst)[0])) __dst = (dst);				\
									\
	static_assert(							\
		__same_type(*__dst, signed char) ||			\
		__same_type(*__dst, unsigned char) ||			\
		__same_type(*__dst, signed short) ||			\
		__same_type(*__dst, unsigned short) ||			\
		__same_type(*__dst, signed int) ||			\
		__same_type(*__dst, unsigned int) ||			\
		__same_type(*__dst, signed long) ||			\
		__same_type(*__dst, unsigned long) ||			\
		__same_type(*__dst, signed long long) ||		\
		__same_type(*__dst, unsigned long long),		\
		"the destination must have standard integer type (not _Bool, char)");	\
	static_assert(							\
		sizeof(*__dst) == 1 ||					\
		sizeof(*__dst) == 2 ||					\
		sizeof(*__dst) == 4 ||					\
		sizeof(*__dst) == 8,					\
		"the destination must have size 1, 2, 4 or 8");		\
									\
	unsigned __b_t_f = (base) | (flags) |				\
		_PARSE_INTEGER_TYPE(typeof(*__dst));			\
	__parse_integer(buf, __b_t_f, __dst);				\
})


/* The internal work-horse. Don't call directly. */
int __parse_integer(const char *buf, unsigned b_t_f, void *dst);

#endif /* LINUX_PARSE_INTEGER_H */
