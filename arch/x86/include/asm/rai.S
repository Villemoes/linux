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

	/*
	 * For convenience, and because it should not cause that much
	 * worse code gen, we tie the hash to an output register, to
	 * avoid it being given in the same register where we must
	 * place the actual output. Since the hash output is unused,
	 * gcc is free to pick that register for anything immediately
	 * afterwards.
	 */
.macro rai_bucket_shift dst, hash, hashq, base, shift
	.pushsection .rai_templ, "aw"
10:	movabs $0x1234567812345678, \dst
	/*
	 * Actually, the hash output contains the shifted hash
	 * value. But I don't think there's a way to inform gcc about
	 * that, and I don't know how useful it would be anyway. So in
	 * the thunk below, we don't do anything to have the same
	 * property, though it would be doable.
	 */
	shr $6, \hash
	lea (\dst, \hashq, 8), \dst
11:
	.popsection

	.pushsection .text.rai_thunk, "ax"
20:	/* dst and hash are registers, we can clobber hash */
	push %rdx
	push %rcx
	mov \hash, %edx
	mov \shift(%rip), %ecx
	shr %cl,%edx
	/* move the shifted value into \hash, so the below works regardless of whether \dst is %rdx or not */
	mov %edx, \hash
	mov \base(%rip), \dst
	lea (\dst, \hashq, 8), \dst
	/* We have our final value. */
	push \dst
	/* Now restore %rdx and %rcx, then finally restore \dst and adjust the stack pointer */
	mov 0x8(%rsp), %rcx
	mov 0x10(%rsp), %rdx
	pop \dst
	add $0x10, %rsp
	jmp 32f
21:
	.popsection
	/* The part that goes into .text */
30:	jmp 20b
31:	.skip -(((11b - 10b)-(31b - 30b)) > 0)*((11b - 10b)-(31b - 30b)), 0x90
32:

	.pushsection .rai_data, "a"
40:
	rai_entry RAI_BUCKET_SHIFT_8_4_4 30b 32b 10b 11b 20b
	.quad \base   /* .bucket_shift.base_addr */
	.quad \shift  /* .bucket_shift.shift_addr */
41:
	rai_entry_pad 40b 41b
	.popsection
.endm /* rai_bucket_shift */


#endif
