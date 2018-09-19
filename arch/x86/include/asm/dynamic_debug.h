/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_X86_DYNAMIC_DEBUG_H
#define _ASM_X86_DYNAMIC_DEBUG_H

#include <asm-generic/jump_label.h>

#ifndef __ASSEMBLY__

#ifdef DEBUG
#define _DPRINTK_INIT_TRUE 1
#else
#define _DPRINTK_INIT_TRUE 0
#endif

#define DEFINE_DYNAMIC_DEBUG_METADATA(name, fmt)			\
	extern struct _ddebug name;					\
	asm volatile("DYNAMIC_DEBUG_METADATA"				\
		     " ident="__stringify(name)				\
		     " modname=%c0"					\
		     " func=%c1"					\
		     " file=%c2"					\
		     " fmt=%c3"						\
		     " flag_line=%c4"					\
		     " init_true=%c5"					\
		     " cnt=%c6"						\
		     : : "i" (KBUILD_MODNAME), "i" (__func__),		\
		       "i" (__FILE__), "i" (fmt),			\
		       "i" (_DPRINTK_FLAGS_LINENO_INIT),		\
		       "i" (_DPRINTK_INIT_TRUE),			\
		       "i" (__COUNTER__)				\
		)

#else	/* __ASSEMBLY__ */

.macro DYNAMIC_DEBUG_METADATA ident modname func file fmt flag_line init_true cnt
.ifndef \ident
	.pushsection __verbose,"aw"
\ident :
	.long \modname - \ident
	.long \func    - \ident
	.long \file    - \ident
	.long \fmt     - \ident
	.long \flag_line
	.long 0
#ifdef HAVE_JUMP_LABEL
	.if \init_true
	STATIC_KEY_INIT_TRUE
	.else
	STATIC_KEY_INIT_FALSE
	.endif
#endif
	.popsection
	.set \ident\().ddebug.once, \cnt
.elseif \ident\().ddebug.once - \cnt
	.error "'\ident' used as _ddebug identifier more than once"
.endif
.endm

#endif	/* __ASSEMBLY__ */

#endif /* _ASM_X86_DYNAMIC_DEBUG_H */

