#include <linux/notifier.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>


extern int call_test_notifiers(unsigned long val, void *v);
#define TESTCHAIN_INIT   0X52U


static int __init test_chain_2_init(void)
{
	printk(KERN_DEBUG "I am in test_chain_2\n");
	call_test_notifiers(TESTCHAIN_INIT, "no_use");

	return 0;
}

static void __exit test_chain_2_exit(void)
{
	printk(KERN_DEBUG"Goodbye to test_chain_2\n");
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("zac");

module_init(test_chain_2_init);
module_exit(test_chain_2_exit);
