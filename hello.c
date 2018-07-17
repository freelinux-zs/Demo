#include <linux/init.h>
#include <linux/module.h>



static int __init hello_init(void)
{
	printk("hello world enter\r\n");
	return 0;
}

static void __exit hello_exit(void)
{
	printk("hello world exit\n");
}


module_init(hello_init);
module_exit(hello_exit);


MODULE_AUTHOR("Zac <zhoubin.xu@gmail.com>");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("A simple Hello world Module");
MODULE_ALIAS("a simplest module");
