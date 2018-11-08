#ifndef __ASM_GENERIC_JUMP_LABEL_H
#define __ASM_GENERIC_JUMP_LABEL_H

#if defined(CC_HAVE_ASM_GOTO) && defined(CONFIG_JUMP_LABEL)
# define HAVE_JUMP_LABEL
#endif

#ifdef HAVE_JUMP_LABEL
#include <linux/const.h>

#define JUMP_TYPE_FALSE		UL(0)
#define JUMP_TYPE_TRUE		UL(1)
#define JUMP_TYPE_LINKED	UL(2)
#define JUMP_TYPE_MASK		UL(3)
#endif

#endif /* __ASM_GENERIC_JUMP_LABEL_H */
