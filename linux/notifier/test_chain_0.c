#include <linux/notifier.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>


#define TESTCHAIN_INIT					0X52U
static RAW_NOTIFIER_HEAD(test_chain);


/*define our own notifier_call_chain*/
static int call_test_notifiers(unsigned long val, void *v)
{
	return raw_notifier_call_chain(&test_chain, val, v);
}
EXPORT_SYMBOL(call_test_notifiers);


/*define our own notifier_chain_register func*/
static int register_test_notifier(struct notifier_block *nb)
{
	int err;
	
	err = raw_notifier_chain_register(&test_chain, nb);  //原始通知链
	//blocking_notifier_chain_register  //可阻塞通知链
	//atomic_notifier_chain_register  //原子通知链
	//srcu_notifier_chain_register  //SRCU 通知链（ SRCU notifier chains ）：可阻塞通知链的一种变体
	if(err)
	  goto out;

out:
	return err;
}
EXPORT_SYMBOL(register_test_notifier);

static int __init test_chain_0_init(void)
{
	printk(KERN_DEBUG "Im in test_chain_0\n");

	return 0;
}

static void __exit test_chain_0_exit(void)
{
	printk(KERN_DEBUG "Goodbye to test_chain_0\n");
}

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("zac");

module_init(test_chain_0_init);
module_exit(test_chain_0_exit);
