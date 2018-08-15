// SPDX-License-Identifier: GPL-2.0

#include <linux/stdarg.h>
#include <linux/build_bug.h>
#include <linux/compiler.h> /* __alias */
#include <linux/export.h>   /* EXPORT_SYMBOL */

int va_arg_int(va_list ap)
{
	BUILD_BUG_ON(!__same_type(va_list, typeof(*ap)[1]));
	return __builtin_va_arg(ap, int);
}
EXPORT_SYMBOL(va_arg_int);

long long va_arg_longlong(va_list ap)
{
	return __builtin_va_arg(ap, long long);
}
EXPORT_SYMBOL(va_arg_longlong);

#if BITS_PER_LONG == 64
long va_arg_long(va_list ap) __alias(va_arg_longlong);
#else
long va_arg_long(va_list ap) __alias(va_arg_int);
#endif
EXPORT_SYMBOL(va_arg_long);
