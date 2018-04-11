// SPDX-License-Identifier: GPL-2.0

#include <linux/stdarg.h>
#include <linux/compiler.h> /* __alias */
#include <linux/build_bug.h>

int va_arg_int(va_list ap)
{
	BUILD_BUG_ON(!__same_type(va_list, typeof(*ap)[1]));
	return __builtin_va_arg(ap, int);
}

long long va_arg_longlong(va_list ap)
{
	return __builtin_va_arg(ap, long long);
}

#if BITS_PER_LONG == 64
long va_arg_long(va_list ap) __alias(va_arg_longlong);
#else
long va_arg_long(va_list ap) __alias(va_arg_int);
#endif
