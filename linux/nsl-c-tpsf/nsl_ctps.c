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

#include "nsl_ctps.h"

#define nsl_name  "nsl_ctps5.5"


typedef struct {
	float NSL_Data[3];
}ppg_data_t;

typedef struct {
	struct delayed_work x_work;
	struct device *nsl_device;
	struct input_dev *nsl_input_dev;
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
		schedule_delayed_work(&nsldata.x_work, msecs_to_jiffies(100));
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
	
	//cat rw_reg	
	printk("%s (%d) \n", __func__, __LINE__);

	return 0; 
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

static void nsl_report_data(void)
{
	if(nsldata.run_ppg)	
	{
		printf(">>>%s (%d)\n",__func__,__LINE__);
		nsldata.ppg_data.NSL_Data[0] = 12.31;
		nsldata.ppg_data.NSL_Data[1] = 1121;
		nsldata.ppg_data.NSL_Data[2] = 55.66;
		input_report_abs(nsldata.nsl_input_dev, ABS_X, *(uint32_t *)(nsldata.ppg_data.NSL_Data));
		input_report_abs(nsldata.nsl_input_dev, ABS_Y, *(uint32_t *)(nsldata.ppg_data.NSL_Data + 1));
		input_report_abs(nsldata.nsl_input_dev, ABS_Z, *(uint32_t *)(nsldata.ppg_data.NSL_Data + 2));
		input_sync(nsldata.nsl_input_dev);		
		
	}
}



static void nsl_x_work_func(struct work_struct *work)
{
	printk(">>>%s (%d)\n", __func__, __LINE__);
//	while(nsldata.run_ppg)
	{
		nsl_report_data();
	}	
}


static int nsl_input_open(struct input_dev *dev)
{
	printk(">>> %s (%d) \n", __func__, __LINE__);
	return 0;
}

static void nsl_input_close(struct input_dev *dev)
{
	printk(">>> %s (%d) \n", __func__, __LINE__);
}


static int nsl_init_input_data(void)
{
	int ret = 0;

	printk("%s (%d) : initialize data\n", __func__, __LINE__);
	
	nsldata.nsl_input_dev = input_allocate_device();
	
	if (!nsldata.nsl_input_dev) {
		printk("%s (%d) : could not allocate mouse input device\n", __func__, __LINE__);
		return -ENOMEM;
	}
	nsldata.nsl_input_dev->evbit[0] = BIT_MASK(EV_ABS);
	nsldata.nsl_input_dev->absbit[0] = BIT_MASK(ABS_X) | BIT_MASK(ABS_Y)| BIT_MASK(ABS_Z)| BIT_MASK(ABS_RX) | BIT_MASK(ABS_RY);

	input_abs_set_max(nsldata.nsl_input_dev, ABS_X, 0xffffffff);
	input_abs_set_max(nsldata.nsl_input_dev, ABS_Y, 0xffffffff);
	input_abs_set_max(nsldata.nsl_input_dev, ABS_Z, 0xffffffff);
	input_abs_set_max(nsldata.nsl_input_dev, ABS_RX, 0xffffffff);
	input_abs_set_max(nsldata.nsl_input_dev, ABS_RY, 0xffffffff);

	input_set_drvdata(nsldata.nsl_input_dev, &nsldata);
	nsldata.nsl_input_dev->name = "NSL ctps5.5";	

	nsldata.nsl_input_dev->open = nsl_input_open;
	nsldata.nsl_input_dev->close = nsl_input_close;
	
	ret = input_register_device(nsldata.nsl_input_dev);
	if (ret < 0) {
		input_free_device(nsldata.nsl_input_dev);
		printk("%s (%d) : could not register input device\n", __func__, __LINE__);	
		return ret;
	}
	
	return 0;	
}




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

	INIT_DELAYED_WORK(&nsldata.x_work, nsl_x_work_func);

	err = nsl_init_input_data();
	if (err < 0) {
		return err;
	}

	nsldata.run_ppg = false;

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

 
