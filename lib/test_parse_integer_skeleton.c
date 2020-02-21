#ifndef TYPE
#error "TYPE must be #defined before including this file"
#endif
#ifndef FMT
#error "FMT must be #defined before including this file"
#endif

#define TYPE_STR    __stringify(TYPE)
#define STAG        __PASTE(TYPE, _test)
#define DO_ONE_TEST __PASTE(__PASTE(do_one_, TYPE), _test)
#define DO_TESTS    __PASTE(__PASTE(do_, TYPE), _tests)
#define ARRAY_NAME  __PASTE(TYPE, _tests)

struct STAG {
	const char *s;
	unsigned   base;
	unsigned   flags;
	int        ret;
	TYPE       val;
};

static const struct STAG ARRAY_NAME[] __initconst;

static int __init
DO_ONE_TEST(const struct STAG *t)
{
	char buf[256];
	TYPE val = POISON;
	int ret;

	total_tests++;
	snprintf(buf, sizeof(buf), "p_i(\"%s\", 0x%x | %d, %s*)",
		 t->s, t->flags, t->base, TYPE_STR);
	ret = parse_integer(t->s, t->base | t->flags, &val);

	if (ret != t->ret) {
		pr_warn("%s returned %d, expected %d", buf, ret, t->ret);
		return 1;
	}

	if (ret >= 0 && val != t->val) {
		pr_warn("%s yielded "FMT", expected "FMT, buf, val, t->val);
		return 1;
	} else if (ret < 0 && val != POISON) {
		pr_warn("%s failed as expected (%d), but wrote "FMT" to the destination",
			buf, ret, val);
		return 1;
	}

	if (ret >= 0)
		pr_info("%s -> %d ("FMT")", buf, ret, val);
	else
		pr_info("%s -> %d", buf, ret);
	return 0;
}

static void __init
DO_TESTS(void)
{
	const struct STAG *t;

	for (t = ARRAY_NAME; t->s != NULL; ++t)
		failed_tests += DO_ONE_TEST(t);
}


#undef TYPE_STR
#undef STAG
#undef DO_ONE_TEST
#undef DO_TESTS
#undef ARRAY_NAME

#undef TYPE
#undef FMT
