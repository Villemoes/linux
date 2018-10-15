#ifndef _ASM_X86_RAI_H
#define _ASM_X86_RAI_H

#define STRUCT_RAI_ENTRY_SIZE 24

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
};
_Static_assert(sizeof(struct rai_entry) == STRUCT_RAI_ENTRY_SIZE,
	       "please update STRUCT_RAI_ENTRY_SIZE");

#endif /* !__ASSEMBLY */

#endif /* _ASM_X86_RAI_H */
