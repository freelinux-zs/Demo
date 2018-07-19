#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#define GLOBALMEM_SIZE		0X1000  /*全局内存大小：4KB*/
#define MEM_CLEAR			0x1     /*清零全局内存*/
#define GLOBALMEM_MAJOR     250		/*预设的globalmem的主设备号*/


static int globalmem_major = 0;//GLOBALMEM_MAJOR;


/*globalmem 设备结构体*/
struct globalmem_dev {
	struct cdev cdev;
	struct class *test_class;
	struct device *test_nor_device;
	struct device *accdet_nor_device;
	unsigned char mem[GLOBALMEM_SIZE];
	struct semaphore sem; /*并发控制信号量*/
};

struct globalmem_dev *globalmem_devp;  /*设备结构体实列*/

static int globalmem_open(struct inode *inode, struct file *filp)
{
	struct globalmem_dev *dev;

	dev = container_of(inode->i_cdev, struct globalmem_dev, cdev);

	filp->private_data = dev;

	return 0;
}

static int globalmem_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count  = size;
	int ret = 0;

	struct globalmem_dev *dev = filp->private_data;  /*获取设备结构体指针*/
	/*分析和获取数据有效长度*/
	if (p >= GLOBALMEM_SIZE)
		return 0;
	if (count > GLOBALMEM_SIZE - p) //要读的字节数太大
		count = GLOBALMEM_SIZE - p;

	if (down_interruptible(&dev->sem)) /*获取信号量*/
		return  - ERESTARTSYS;

	/*内核空间---->用户空间*/
	if (copy_to_user(buf, (void *)(dev->mem +p), count))
		ret = - EFAULT;
	else{
		*ppos += count;
		ret = count;

		printk(KERN_INFO "read %d byte(s) form %ld\n",count, p);
	}

	up(&dev->sem);

	return ret;
}

static ssize_t globalmem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct globalmem_dev *dev = filp->private_data;
	/*分析和获取有效读数据*/
	if (p >= GLOBALMEM_SIZE)
		return 0;
	if (count > GLOBALMEM_SIZE - p)
		return 0;
	if (count > GLOBALMEM_SIZE - p)
		count = GLOBALMEM_SIZE - p;

	if (down_interruptible(&dev->sem)) /*获取信号量*/
		return  - ERESTARTSYS; 
	/*用户空间----->内核空间*/
	if (copy_from_user(dev->mem + p, buf, count))
		ret = - EFAULT;
	else{
		*ppos += count;
		ret = count;

		printk(KERN_INFO "Written %d byte(s) from %ld\n",count, p);
	}

	up(&dev->sem); 

	return ret;
}

static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig)
{
	loff_t ret;
	switch (orig) {
		case 0:
			if (offset < 0) {
				ret = - EINVAL;
				break;
			}

			if ((unsigned int)offset > GLOBALMEM_SIZE) {
				ret = - EINVAL;
				break;
			}

			filp->f_pos = (unsigned int)offset;
			ret = filp->f_pos;
			break;
		
		case 1:
			if ((filp->f_pos + offset) > GLOBALMEM_SIZE) {
				ret = - EINVAL;
				break;
			}

			if ((filp->f_pos + offset) < 0) {
				ret = - EINVAL;
				break;
			}

			filp->f_pos += offset;
			ret = filp->f_pos;
			break;
		default:
			ret = - EINVAL;
	}

	return ret;
}

//static int globalmem_ioctl(struct inode *inodep, struct file *filp, unsigned int cmd, unsigned long arg)
static long globalmem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct globalmem_dev *dev = filp->private_data;

	switch (cmd) {
		case MEM_CLEAR:

			if (down_interruptible(&dev->sem)) /*获得信号量*/
				return - ERESTARTSYS;

			memset(dev->mem, 0, GLOBALMEM_SIZE);

			up(&dev->sem); /*释放信号量*/

			printk(KERN_INFO "globalmem is set to ZERO\n");
			break;

		default:
			return  - EINVAL;
	}

	return 0;
}

static const struct file_operations globalmem_fops = {
	.owner = THIS_MODULE,
	.open = globalmem_open,
	.release = globalmem_release,
	.llseek = globalmem_llseek,
	.read = globalmem_read,
	.write = globalmem_write,
	//.ioctl = globalmem_ioctl,
	.unlocked_ioctl = globalmem_ioctl,
};

/*初始化并贴加cdev结构体*/
static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
	int err, devno = MKDEV(globalmem_major, 0);

	cdev_init(&dev->cdev, &globalmem_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding globalmem",err);

	globalmem_devp->test_class = class_create(THIS_MODULE, "globalmem02");  
	if(IS_ERR(globalmem_devp->test_class)) {
		printk("Err: failed in creating class./n");
	}

	globalmem_devp->test_nor_device = device_create(globalmem_devp->test_class, NULL, devno, NULL, "globalmem01");
}


/*globalmem设备驱动模块加载函数*/
static int __init globalmem_init(void)
{
	int result;

	dev_t devno = MKDEV(globalmem_major, 0);

	/*申请字符设备驱动区域*/
	if(globalmem_major)
		result = register_chrdev_region(devno, 2, "globalmem");
	else{
		/*动态获得主设备号*/
		result = alloc_chrdev_region(&devno, 0, 2, "globalmem");
		globalmem_major = MAJOR(devno);
	}

	if (result < 0)
		return result;

	/*动态申请设备结构体内存*/
	globalmem_devp = kmalloc(2 * sizeof(struct globalmem_dev), GFP_KERNEL);

	if (!globalmem_devp) {/*申请失败*/
		goto fail_malloc;
	}


	memset(globalmem_devp, 0, 2 * sizeof(struct globalmem_dev));

	globalmem_setup_cdev(&globalmem_devp[0], 0);
	globalmem_setup_cdev(&globalmem_devp[1], 1);
//	init_MUTEX(&globalmem_devp->sem); /*初始化信号量*/
	sema_init(&globalmem_devp->sem, 1);
	return 0;

fail_malloc:
	unregister_chrdev_region(devno, 1);
	return result;
}

static void __exit globalmem_exit(void)
{
	//cdev_del(&globalmem_devp->cdev); //删除cdev 结构
	device_del(globalmem_devp->test_nor_device);
	class_destroy(globalmem_devp->test_class);
	cdev_del(&(globalmem_devp[0].cdev));
	cdev_del(&(globalmem_devp[1].cdev));
	
	kfree(globalmem_devp);
	unregister_chrdev_region(MKDEV(globalmem_major, 0), 2); //注销设备区域
	printk("globalmem_exit\n");
}

module_init(globalmem_init);
module_exit(globalmem_exit);


MODULE_AUTHOR("Zac <zhoubin.xu@gmail.com>");
MODULE_LICENSE("Dual BSD/GPL");
