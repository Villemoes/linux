#ifdef __ASSEMBLY__

.macro rai_entry type instr instr_end templ templ_end thunk
	.long \type
	.long \instr - .
	.long \instr_end - \instr
	.long \templ - .
	.long \templ_end - \templ
	.long \thunk - .
.endm

.macro rai_entry_pad start end
	.ifgt STRUCT_RAI_ENTRY_SIZE-(\end-\start)
	.skip STRUCT_RAI_ENTRY_SIZE-(\end-\start), 0x00
	.endif
.endm

.macro rai_load dst, var, type
	.pushsection .rai_templ, "aw"
10:
	.ifeq \type - RAI_LOAD_8
	movabs $0x1234567812345678, \dst
	.else
	mov $0x12345678, \dst
	.endif
11:
	.popsection

	/* Even if the mov \var, \dst is short enough to fit in the
	 * space we reserve in .text, we still need the thunk for when
	 * we do the immediate patching. */
	.pushsection .text.rai_thunk, "ax"
20:
	mov \var(%rip), \dst
	jmp 32f
21:
	.popsection

	/* The part that goes into .text */
30:
	/* silence objtool by actually using the thunk for now */
	jmp 20b
	/* mov \var(%rip), \dst */
31:
	.skip -(((11b - 10b)-(31b - 30b)) > 0)*((11b - 10b)-(31b - 30b)), 0x90
32:

	.pushsection .rai_data, "a"
40:
	rai_entry \type 30b 32b 10b 11b 20b
	.quad \var   /* .load.addr */
41:
	rai_entry_pad 40b 41b
	.popsection
.endm /* rai_load */


#endif
