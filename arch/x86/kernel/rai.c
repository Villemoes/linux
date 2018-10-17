#include <linux/memory.h>
#include <linux/mutex.h>
#include <linux/rai.h>
#include <asm/text-patching.h>

extern struct rai_entry __start_rai_data[];
extern struct rai_entry __stop_rai_data[];

static void
rai_patch_one(const struct rai_entry *r)
{
	u8 *instr = (u8*)&r->instr_offset + r->instr_offset;
	u8 *templ = (u8*)&r->templ_offset + r->templ_offset;
	u8 *thunk = (u8*)&r->thunk_offset + r->thunk_offset;

	switch (r->type) {
	case RAI_LOAD_4: {
		const u32 *imm = r->load.addr;
		/*
		 * The immediate is the last 4 bytes of the template,
		 * regardless of the operand encoding.
		 */
		memcpy(templ + r->templ_len - sizeof(*imm), imm, sizeof(*imm));
		break;
	}
	case RAI_LOAD_8: {
		const u64 *imm = r->load.addr;
		/*
		 * The immediate is the last 8 bytes of the template,
		 * regardless of the operand encoding.
		 */
		memcpy(templ + r->templ_len - sizeof(*imm), imm, sizeof(*imm));
		break;
	}
	case RAI_BUCKET_SHIFT_8_4_4: {
		const u32 *shiftp = r->bucket_shift.shift_addr;
		const u64 *basep = r->bucket_shift.base_addr;
		/*
		 * This should be made more robust. For now, assume we
		 * have a 10-byte movabs followed by a 3-byte shr. And
		 * while *shiftp is 4 bytes wide, we just need the
		 * LSB.
		 */
		memcpy(templ + 2, basep, sizeof(*basep));
		memcpy(templ + 12, shiftp, 1);
		break;
	}
	default:
		WARN_ONCE(1, "unhandled RAI type %d\n", r->type);
		return;
	}
	text_poke_bp(instr, templ, r->templ_len, thunk);
}

static void
rai_patch(const struct rai_entry *start, const struct rai_entry *stop)
{
	const struct rai_entry *r;

	for (r = start; r < stop; ++r)
		rai_patch_one(r);
}

void
update_rai_access(void)
{
	mutex_lock(&text_mutex);
	rai_patch(__start_rai_data, __stop_rai_data);
	mutex_unlock(&text_mutex);
}

#if 1
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static int one, two;
static long three;

static struct hlist_head *ht1;
static unsigned shift1;

static int
rai_proc_show(struct seq_file *m, void *v) {
	unsigned hash = 0xdeadbeef;

	seq_printf(m, "one: %d, two: %d, three: %ld\n",
		   rai_load(one), rai_load(two), rai_load(three));
	seq_printf(m, "ht1: %016lx, bucket 0x%08x: %016lx\n",
		   (long)rai_load(ht1), hash, (long)rai_bucket_shift(ht1, shift1, hash));

	one = two = three = -1;
	ht1 = NULL;
	shift1 = 2;

	return 0;
}

static int
rai_proc_open(struct inode *inode, struct  file *file) {
	return single_open(file, rai_proc_show, NULL);
}

static const struct file_operations rai_proc_fops = {
	.owner = THIS_MODULE,
	.open = rai_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init rai_proc_init(void) {
	one = 1;
	two = 2;
	three = 3;
	ht1 = (void*)0xffffffffabcd0000UL;
	shift1 = 26;

	proc_create("rai", 0, NULL, &rai_proc_fops);
	return 0;
}
late_initcall(rai_proc_init);
#endif
