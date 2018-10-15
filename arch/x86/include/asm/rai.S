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

#endif
