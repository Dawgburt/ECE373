/*
Phil Nevins (pnevins@pdx.edu)
Version: 0.1
This program is a device framework build for a kernel module that
creates and registers a single character device.
This sets up the char driver and allows read / write functionality

Template from example5.c used
Code written by Nathan Mai
Professor for PSU ECE-373
*/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVCNT 5
#define DEVNAME "my_chardev"

static struct mydev_dev {
	struct cdev my_cdev;
	int syscall_val;
} mydev;

static dev_t mydev_node;

/* this shows up under /sys/modules/my_chardev/parameters */
static int exam = 15;
module_param(exam, int, S_IRUSR | S_IWUSR);

/* this doesn't appear in /sys/modules */
static int exam_nosysfs = 25;
module_param(exam_nosysfs, int, 0);

//Open function
static int my_chardev_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "successfully opened!\n");

	mydev.syscall_val = 42;

	return 0;
}

//Read funciton
static ssize_t my_chardev_read(struct file *file, char __user *buf,
                             size_t len, loff_t *offset)
{
	/* Get a local kernel buffer set aside */
	int ret;

	/* Make sure our user wasn't bad... */
	if (!buf) {
		ret = -EINVAL;
		goto out;
	}

	if (copy_to_user(buf, &mydev.syscall_val, sizeof(int))) {
		ret = -EFAULT;
		goto out;
	}
	ret = sizeof(int);
	*offset += sizeof(int);

	/* Good to go, so printk the thingy */
	printk(KERN_INFO "User got from us %d\n", mydev.syscall_val);

out:
	return ret;
}

//Write function
static ssize_t my_chardev_write(struct file *file, const char __user *buf,
                              size_t len, loff_t *offset)
{
	/* Have local kernel memory ready */
	char *kern_buf;
	int ret;

	/* Make sure our user isn't bad... */
	if (!buf) {
		ret = -EINVAL;
		goto out;
	}

	/* Get some memory to copy into... */
	kern_buf = kmalloc(len, GFP_KERNEL);

	/* ...and make sure it's good to go */
	if (!kern_buf) {
		ret = -ENOMEM;
		goto out;
	}

	/* Copy from the user-provided buffer */
	if (copy_from_user(kern_buf, buf, len)) {
		/* uh-oh... */
		ret = -EFAULT;
		goto mem_out;
	}
	
	//Update syscall_val with user input value
	memcpy(&mydev.syscall_val, kern_buf, sizeof(int));

	ret = len;

	/* print what userspace gave us */
	printk(KERN_INFO "Userspace wrote \"%s\" to us\n", kern_buf);

mem_out:
	kfree(kern_buf);
out:
	return ret;
}

/* File operations for our device */
static struct file_operations mydev_fops = {
	.owner = THIS_MODULE,
	.open = my_chardev_open,
	.read = my_chardev_read,
	.write = my_chardev_write,
};

static int __init my_chardev_init(void)
{
	printk(KERN_INFO "my_chardev module loading... exam=%d, exam_nosysfs: %d\n", exam, exam_nosysfs);

	if (alloc_chrdev_region(&mydev_node, 0, DEVCNT, DEVNAME)) {
		printk(KERN_ERR "alloc_chrdev_region() failed!\n");
		return -1;
	}

	printk(KERN_INFO "Allocated %d devices at major: %d\n", DEVCNT,
	       MAJOR(mydev_node));

	/* Initialize the character device and add it to the kernel */
	cdev_init(&mydev.my_cdev, &mydev_fops);
	mydev.my_cdev.owner = THIS_MODULE;

	if (cdev_add(&mydev.my_cdev, mydev_node, DEVCNT)) {
		printk(KERN_ERR "cdev_add() failed!\n");
		/* clean up chrdev allocation */
		unregister_chrdev_region(mydev_node, DEVCNT);

		return -1;
	}

	return 0;
}

static void __exit my_chardev_exit(void)
{
	/* destroy the cdev */
	cdev_del(&mydev.my_cdev);

	/* clean up the devices */
	unregister_chrdev_region(mydev_node, DEVCNT);

	printk(KERN_INFO "my_chardev module unloaded!\n");
}

MODULE_AUTHOR("Phil Nevins");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
module_init(my_chardev_init);
module_exit(my_chardev_exit);
