#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/mm.h>
#include <linux/io.h>	


#define MY_DEVNAME "char_dev"
#define MAX_SIZE   1024

//#define WR_VALUE _IOW('a','a',int *)
//#define RD_VALUE _IOR('a','b',int *)
#define SET_BAUDRATE _IOW('a','c',int *)
#define GET_BAUDRATE _IOR('a','d',int *)
#define GET_DATA _IOR('a','e',char *)



unsigned int value = 9600;


dev_t devno;
static struct class *dev_class;
static char *kmalloc_area;


static int chardev_open(struct inode *inode, struct file *file);
static int chardev_close(struct inode *inode, struct file *file);
static ssize_t chardev_write(struct file *file,const char __user *buf,size_t size,loff_t *offset);
static ssize_t chardev_read(struct file *file,char __user *buf,size_t size,loff_t *offset);
static long chardev_ioctl(struct file *file,unsigned int cmd, unsigned long arg);
static int my_mmap(struct file *file , struct vm_area_struct *vm);

/* driver specific structure*/
struct driver_info
{
	char *buf_addr;
	struct cdev mycdev;
};

struct driver_info mydev;



struct file_operations mydev_fops =
{
	.owner = THIS_MODULE,
	.open = chardev_open,
	.release = chardev_close,
	.write = chardev_write,
	.read = chardev_read,
	.unlocked_ioctl = chardev_ioctl,
	.mmap = my_mmap,
};

static int my_mmap(struct file *file,struct vm_area_struct *vma)
{
	unsigned long pfn;
	size_t size = vma->vm_end - vma->vm_start;

	printk(KERN_INFO "mmap driver called\n");
	if( (kmalloc_area = kmalloc(MAX_SIZE, GFP_KERNEL)) == NULL)
		return -1;

	pfn = virt_to_phys((void *)kmalloc_area)>>PAGE_SHIFT;



	/* Remap-pfn-range will mark the range VM_IO */
	if (remap_pfn_range(vma, vma->vm_start,
				pfn, size,
				vma->vm_page_prot))
		return -EAGAIN;

	return 0;




}


static int chardev_open(struct inode *inode, struct file *file)
{
	if((mydev.buf_addr = kmalloc(MAX_SIZE, GFP_KERNEL)) == NULL)
	{
		printk(KERN_INFO " cannot allocate memory in kernel \n");
		return -1;
	}
	printk(KERN_INFO "device file opened \n");
	return 0;
}

static ssize_t chardev_write(struct file *file,const char __user *buf,size_t size,loff_t *offset)
{
	printk(KERN_INFO "write driver method is invoked...\n");

	if(copy_from_user(mydev.buf_addr,buf,size))
	{
		printk(KERN_INFO " write driver failed \n");
		return -1;
	}

	printk(KERN_INFO "data = %s \n",mydev.buf_addr);


	printk(KERN_INFO "Data Write ...Done\n");
	return size;

}



static ssize_t chardev_read(struct file *file,char __user *buf,size_t size,loff_t *offset)
{
	printk(KERN_INFO " read driver is called \n");

	if(copy_to_user(buf,mydev.buf_addr,size))
	{
		printk(KERN_INFO "read driver failed \n");
		return -1;
	}
	printk(KERN_INFO " data read...done \n");
	return size;
}



static int chardev_close(struct inode *inode, struct file *file)
{
	kfree(kmalloc_area);
	kfree(mydev.buf_addr);
	printk(KERN_INFO "device closed...\n");
	return 0;
}

static long chardev_ioctl(struct file *file,unsigned int cmd, unsigned long arg)
{
	printk(KERN_INFO " driver ioctl called ...\n");
	switch(cmd)
	{
#if 0
		case WR_VALUE:
			if(copy_from_user(&value,(int *)arg,sizeof(value)))
			{
				printk(KERN_INFO "ioctl write failed ...\n");
				return -1;
			}
			printk(KERN_INFO"value = %d\n",value);
			break;


		case RD_VALUE:
			if(copy_to_user((int *)arg,&value,sizeof(value)))
			{
				printk(KERN_INFO "ioctl read failed ...\n");
				return -1;
			}

			break;
#endif
		case SET_BAUDRATE:
			if(copy_from_user(&value,(int *)arg,sizeof(value)))
			{
				printk(KERN_INFO "ioctl write failed ...\n");
				return -1;
			}
			printk(KERN_INFO" baudrate changed = %d\n", value);
			break;


		case GET_BAUDRATE:
			if(copy_to_user((int *)arg,&value,sizeof(value)))
			{
				printk(KERN_INFO "ioctl read failed ...\n");
				return -1;
			}
			break;
		
		case GET_DATA:
			if(copy_to_user((char *)arg,kmalloc_area,1024))
			{
				printk(KERN_INFO "ioctl read failed ...\n");
				return -1;
			}
			printk(KERN_INFO " %s\n",kmalloc_area);
			break;
		default:
			printk(KERN_INFO"No %d ioctl commad\n", cmd);
			break;
	}
	return 0;
}
static int __init mydriver_init(void)
{
	int ret;

	ret=alloc_chrdev_region(&devno,0,2,MY_DEVNAME);
	if(ret < 0)
	{
		printk(KERN_INFO " allocating major number failed\n");
		return ret;
	}

	printk(KERN_INFO "Major = %d  Minor = %d\n",MAJOR(devno),MINOR(devno));

	cdev_init(&mydev.mycdev,&mydev_fops);
	ret=cdev_add(&mydev.mycdev,devno,2);
	if(ret<0)
	{
		printk(KERN_INFO "can not add the device to kernel\n");
		unregister_chrdev_region(devno,1);
		return ret;
	}


	if((dev_class = class_create(THIS_MODULE,"my_class")) == NULL)
	{
		printk("can not create the struct class for device\n");
		goto class_error;
	}
	if((device_create(dev_class,NULL,devno,NULL,"my_device")) == NULL )
	{
		printk("can not create the device\n");
		goto device_error;
	}

	printk(KERN_INFO"driver inserted.....\n");
	return 0;





device_error:
	class_destroy(dev_class);

class_error:
	unregister_chrdev_region(devno,1);
	return -1;
}


static void __exit mydriver_exit(void)
{
	device_destroy(dev_class,devno);
	class_destroy(dev_class);
	cdev_del(&mydev.mycdev);
	unregister_chrdev_region(devno,1);
	printk(KERN_INFO " driver remove .... done \n");
}

module_init(mydriver_init);
module_exit(mydriver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("suresh");
MODULE_DESCRIPTION("charcter driver");


