// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include "ubifs.h"

/* Normal UBIFS messages */
void ubifs_msg(const struct ubifs_info *c, const char *fmt, ...)
{
	struct va_format vaf;

	vaf.fmt = fmt;
	va_start(vaf.va, fmt);

	pr_notice("UBIFS (ubi%d:%d): %pV\n",
		  c->vi.ubi_num, c->vi.vol_id, &vaf);

	va_end(vaf.va);
}								    \

/* UBIFS error messages */
void ubifs_err(const struct ubifs_info *c, const char *fmt, ...)
{
	struct va_format vaf;

	vaf.fmt = fmt;
	va_start(vaf.va, fmt);

	pr_err("UBIFS error (ubi%d:%d pid %d): %ps: %pV\n",
	       c->vi.ubi_num, c->vi.vol_id, current->pid,
	       __builtin_return_address(0),
	       &vaf);

	va_end(vaf.va);
}								    \

/* UBIFS warning messages */
void ubifs_warn(const struct ubifs_info *c, const char *fmt, ...)
{
	struct va_format vaf;

	vaf.fmt = fmt;
	va_start(vaf.va, fmt);

	pr_warn("UBIFS warning (ubi%d:%d pid %d): %ps: %pV\n",
		c->vi.ubi_num, c->vi.vol_id, current->pid,
		__builtin_return_address(0),
		&vaf);

	va_end(vaf.va);
}
