#include <linux/notifier.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/fs.h>

extern int register_test_notifier(struct notifier_block *nb);
#define TESTCHAIN_INIT			0X52U


/*realize the notifier_call func*/
int test_init_event(struct notifier_block *nb, unsigned long event, void *v)
{
	switch(event){
		case TESTCHAIN_INIT:
			printk(KERN_DEBUG "I got the chain event: test_chain_2 is on the way of init\n");
			break;
		default:
			break;
	
	}

	return NOTIFY_DONE;
}

/*define a notifier_block*/
static struct notifier_block test_init_notifier = {
	.notifier_call = test_init_event,
};

static int __init test_chain_1_init(void)
{
	printk(KERN_DEBUG "I am in test_chain_1\n");
	register_test_notifier(&test_init_notifier);
	return 0;
}

static void __exit test_chain_1_exit(void)
{
	printk(KERN_DEBUG "Goodbye to test_clain_1\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zac");


module_init(test_chain_1_init);
module_exit(test_chain_1_exit);
