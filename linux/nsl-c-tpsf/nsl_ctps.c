#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/input.h>
//#include <linux/wakelock.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/syscalls.h> 
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>

#include "nsl_ctps.h"

#define nsl_name  "nsl_ctps5.5"


typedef struct {
	float NSL_Data[3];
}ppg_data_t;

typedef struct {
	struct delayed_work x_work;
	struct device *nsl_device;
	struct input_dev *nsl_input_dev;
	struct mutex lock;
	bool run_ppg;
	ppg_data_t ppg_data;
}nsl_data_t;

static nsl_data_t nsldata;


static ssize_t nsl_enable_store(struct device* dev, 
                                   struct device_attribute *attr, const char *buf, size_t count)
{
	printk("%s (%d) :\n", __func__, __LINE__);
	if((buf != NULL) && ( (1 == buf[0]) || ('1' == buf[0]) ))
	{
		printk("Enable!!\n");		
		nsldata.run_ppg = true;
//		schedule_delayed_work(&nsldata.x_work, msecs_to_jiffies(100));
	}
	else
	{
		printk("Disable!!\n");	
		nsldata.run_ppg = false ;
	}
	return count;
}                       

static ssize_t nsl_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int rawdata[2] = {0};
	mutex_lock(&nsldata.lock);
	rawdata[0] = 4;
	rawdata[1] = 55;
	mutex_unlock(&nsldata.lock);
	//cat rw_reg	
	printk("%s (%d) \n", __func__, __LINE__);
	
	return sprintf(buf,"%d,%d",rawdata[0],rawdata[1]);

}



static DEVICE_ATTR(enable, 0664, nsl_enable_show, nsl_enable_store);
static struct device_attribute *nsl_attr_list[] =
{
	&dev_attr_enable,
};

/***********************************************/
/*----------------------------------------------------------------------------*/
static int nsl_create_attr(struct device *dev) 
{
	int idx, err = 0;
	int num = (int)(sizeof(nsl_attr_list)/sizeof(nsl_attr_list[0]));
	if(!dev)
	{
		return -EINVAL;
	}	

	for(idx = 0; idx < num; idx++)
	{
		if((err = device_create_file(dev, nsl_attr_list[idx])))
		{            
			printk("device_create_file (%s) = %d\n", nsl_attr_list[idx]->attr.name, err);        
			break;
		}
	}

	return err;
}
/*----------------------------------------------------------------------------*/
static int nsl_delete_attr(struct device *dev)
{
	
	int idx ,err = 0;
	int num = (int)(sizeof(nsl_attr_list)/sizeof(nsl_attr_list[0]));
	if (!dev)
	{
		return -EINVAL;
	}
	

	for (idx = 0; idx < num; idx++)
	{
		device_remove_file(dev, nsl_attr_list[idx]);
	}	

	return err;
} 

/*--------------------------------------------*/

/********************************************/
static ssize_t nsl_read(struct file *filp, char *buf, size_t count, loff_t *l)
{
	printk(">>>%s (%d) \n",__func__, __LINE__);
	return 0;
}

static ssize_t nsl_write(struct file *filp, const char *buf, size_t count, loff_t *f_ops)
{
	printk(">>>%s (%d) \n", __func__, __LINE__);
	return count;
}

static long nsl_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	printk(">>>%s (%d)\n", __func__, __LINE__);
	return 0;
}

static int nsl_open(struct inode *inode, struct file *filp)
{
	printk(">>>%s (%d)\n", __func__, __LINE__);
	return 0;
}

static int nsl_release(struct inode *inode, struct file *filp)
{
	printk(">>> %s (%d) \n", __func__, __LINE__);
	return 0;
}


/*------------------------------------------*/
static struct file_operations nsl_fops = {
	.owner =  THIS_MODULE,
	.read = nsl_read,
	.write = nsl_write,
	.unlocked_ioctl = nsl_ioctl,
	.open = nsl_open,
	.release = nsl_release, 
};

struct miscdevice nsl_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = nsl_name,
	.fops = &nsl_fops,
};

static int __init nslctps_init(void)
{
	int err = 0;
	printk("%s (%d) :init module\n", __func__,__LINE__);
	
	if((err = misc_register(&nsl_device))){
		printk("nsl_driver register failed\n");
		return err;
	}

	nsldata.nsl_device = nsl_device.this_device;
	if( (err = nsl_create_attr(nsldata.nsl_device)) ){
		printk("create attribute err = %d\n", err);
		return err;
	}  


	nsldata.run_ppg = false;
	mutex_init(&nsldata.lock);
	return 0;
}

static void __exit nslctps_exit(void)
{
	printk("%s (%d):exit module\n", __func__, __LINE__);
	nsl_delete_attr(nsldata.nsl_device);	
	misc_deregister(&nsl_device);
}


module_init(nslctps_init);
module_exit(nslctps_exit);
MODULE_AUTHOR("YIWANG");
MODULE_DESCRIPTION("nsl ctps5.5 driver");
MODULE_LICENSE("Dual BSD/GPL");

 
