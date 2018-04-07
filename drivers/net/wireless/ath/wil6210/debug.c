/*
 * Copyright (c) 2013,2016 Qualcomm Atheros, Inc.
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "wil6210.h"
#include "trace.h"

void __wil_err(struct wil6210_priv *wil, const char *fmt, ...)
{
	struct va_format vaf;
	vaf.fmt = fmt;
	va_start(vaf.va, fmt);
	netdev_err(wil->main_ndev, "%pV", &vaf);
	trace_wil6210_log_err(&vaf);
	va_end(vaf.va);
}

void __wil_err_ratelimited(struct wil6210_priv *wil, const char *fmt, ...)
{
	struct va_format vaf;

	if (!net_ratelimit())
		return;

	vaf.fmt = fmt;
	va_start(vaf.va, fmt);
	netdev_err(wil->main_ndev, "%pV", &vaf);
	trace_wil6210_log_err(&vaf);
	va_end(vaf.va);
}

void wil_dbg_ratelimited(const struct wil6210_priv *wil, const char *fmt, ...)
{
	struct va_format vaf;

	if (!net_ratelimit())
		return;

	vaf.fmt = fmt;
	va_start(vaf.va, fmt);
	netdev_dbg(wil->main_ndev, "%pV", &vaf);
	trace_wil6210_log_dbg(&vaf);
	va_end(vaf.va);
}

void __wil_info(struct wil6210_priv *wil, const char *fmt, ...)
{
	struct va_format vaf;
	vaf.fmt = fmt;
	va_start(vaf.va, fmt);
	netdev_info(wil->main_ndev, "%pV", &vaf);
	trace_wil6210_log_info(&vaf);
	va_end(vaf.va);
}

void wil_dbg_trace(struct wil6210_priv *wil, const char *fmt, ...)
{
	struct va_format vaf;
	vaf.fmt = fmt;
	va_start(vaf.va, fmt);
	trace_wil6210_log_dbg(&vaf);
	va_end(vaf.va);
}
