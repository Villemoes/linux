#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/parse_integer.h>
#include <linux/stringify.h>
#include <linux/types.h>

static unsigned total_tests __initdata;
static unsigned failed_tests __initdata;

#define LEAD_WS    PARSE_INTEGER_LEAD_WS
#define TRAIL_WS   PARSE_INTEGER_TRAIL_WS
#define WS         PARSE_INTEGER_WS
#define ZERO_ON_OK PARSE_INTEGER_ZERO_ON_OK
#define ALL        PARSE_INTEGER_ALL
#define SATURATE   PARSE_INTEGER_SATURATE
#define WRAP       PARSE_INTEGER_WRAP

#define POISON 123

#define TYPE s8
#define FMT "%d"
#include "test_parse_integer_skeleton.c"

#define TYPE u8
#define FMT "%d"
#include "test_parse_integer_skeleton.c"

#define TYPE s64
#define FMT "%lld"
#include "test_parse_integer_skeleton.c"

static const struct s8_test s8_tests[] = {
	{.s = "0", .ret = 1, .val = 0},
	{.s = "077", .ret = 3, .val = 63},
	{.s = "128", .ret = -ERANGE},
	{.s = "-128", .ret = 4, .val = -128},
	{.s = "1234", .ret = 4, .flags = SATURATE, .val = 127 },
	{.s = "1234", .ret = 4, .flags = WRAP, .val = -46 },
	{.s = NULL},
};

static const struct u8_test u8_tests[] = {
	{.s = "0", .ret = 1, .val = 0},
	{.s = "077", .ret = 3, .val = 63},
	{.s = "128", .ret = 3, .val = 128},
	{.s = "-128", .ret = -ERANGE},
	{.s = "-128", .ret = 4, .flags = SATURATE, .val = 0},
	{.s = "-128", .ret = 4, .flags = WRAP, .val = 128},
	{.s = "1234", .ret = 4, .flags = SATURATE, .val = 255 },
	{.s = "1234", .ret = 4, .flags = WRAP, .val = 210 },
	{.s = NULL},
};

static const struct s64_test s64_tests[] = {
	{.s = "0", .ret = 1, .val = 0},
	{.s = "0x1234567812345678", .ret = 18, .val = 0x1234567812345678ll},
	{.s = "0x", .ret = -EINVAL},
	{.s = "0x", .ret = -EINVAL, .base = 16},
	{.s = "0x", .ret = 1, .base = 10, .val = 0},
	{.s = "0x", .ret = 1, .base = 8, .val = 0},
	{.s = NULL},
};


static int __init test_parse_integer_init(void)
{
	int err = 0;

	do_s8_tests();
	do_u8_tests();
	do_s64_tests();

	if (failed_tests) {
		pr_warn("Failed %d out of %d tests\n", failed_tests, total_tests);
		err = -EINVAL;
	} else {
		pr_info("all %d tests passed\n", total_tests);
	}
	return err;

}
module_init(test_parse_integer_init);
