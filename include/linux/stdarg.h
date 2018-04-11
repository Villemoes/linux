/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_STDARG_H
#define _LINUX_STDARG_H

#if __INCLUDE_LEVEL__ > 1
#error "linux/stdarg.h must not be indirectly included"
#endif

#include <stdarg.h>

#ifdef CONFIG_OOL_VA_ARG

#include <linux/compiler_types.h> /* __same_type */

int va_arg_int(va_list ap);
long long va_arg_longlong(va_list ap);
long va_arg_long(va_list ap);

#undef va_arg

#define va_arg(ap, t) ((t)						\
	__builtin_choose_expr(__same_type(t, int),                va_arg_int(ap), \
	__builtin_choose_expr(__same_type(t, unsigned int),       va_arg_int(ap), \
	__builtin_choose_expr(__same_type(t, long),               va_arg_long(ap), \
	__builtin_choose_expr(__same_type(t, unsigned long),      va_arg_long(ap), \
	__builtin_choose_expr(__same_type(t, long long),          va_arg_longlong(ap), \
	__builtin_choose_expr(__same_type(t, unsigned long long), va_arg_longlong(ap), \
	__builtin_choose_expr(sizeof(t) == sizeof(void *),        va_arg_long(ap), \
			      __builtin_va_arg(ap, t) \
		))))))))

#endif /* CONFIG_OOL_VA_ARG */

#endif /* _LINUX_STDARG_H */
