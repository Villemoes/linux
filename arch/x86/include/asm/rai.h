#ifndef _ASM_X86_RAI_H
#define _ASM_X86_RAI_H

#define RAI_LOAD_4 0
#define RAI_LOAD_8 1

#define STRUCT_RAI_ENTRY_SIZE 32

/* Put the asm macros in a separate file for easier editing. */
#include <asm/rai.S>

#ifndef __ASSEMBLY__

struct rai_entry {
	int type;         /* RAI_xxx constant */
	s32 instr_offset; /* member-relative offset to instructions-to-be-patched */
	s32 instr_len;    /* size of area, >= templ_len */
	s32 templ_offset; /* member-relative offset to template */
	s32 templ_len;    /* length of template */
	s32 thunk_offset; /* member-relative offset to ool thunk */
	/* type-specific data follows */
	union {
		struct {
			void *addr;
		} load;
	};
};
_Static_assert(sizeof(struct rai_entry) == STRUCT_RAI_ENTRY_SIZE,
	       "please update STRUCT_RAI_ENTRY_SIZE");

#define _rai_load(var) ({						\
		typeof(var) ret__;					\
		switch(sizeof(var)) {					\
		case 4:							\
			asm("rai_load %0, %c1, %c2"			\
			    : "=r" (ret__)				\
			    : "i" (&(var)), "i" (RAI_LOAD_4));		\
			break;						\
		case 8:							\
			asm("rai_load %0, %c1, %c2"			\
			    : "=r" (ret__)				\
			    : "i" (&(var)), "i" (RAI_LOAD_8));		\
			break;						\
		default:						\
			ret__ = _rai_load_fallback(var);		\
			break;						\
		}							\
		ret__;							\
	})

#endif /* !__ASSEMBLY */

#endif /* _ASM_X86_RAI_H */
