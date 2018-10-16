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

static int
rai_proc_show(struct seq_file *m, void *v) {
	seq_printf(m, "one: %d, two: %d, three: %ld\n",
		   rai_load(one), rai_load(two), rai_load(three));
	one = two = three = -1;

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

	proc_create("rai", 0, NULL, &rai_proc_fops);
	return 0;
}
late_initcall(rai_proc_init);
#endif
