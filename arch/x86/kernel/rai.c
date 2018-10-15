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
