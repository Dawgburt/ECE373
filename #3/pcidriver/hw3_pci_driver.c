/*
Phil Nevins (pnevins@pdx.edu)
Version: 0.1
driver for the 1000e network device that will enable blinking the LEDs

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
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#define DEVCNT 5
#define DEVNAME "hw3"
char pci_driver_name[] = "hw3_pci_driver";

/*given struct*/
static struct mydev_dev {
    struct cdev my_cdev;
    dev_t mydev_node;
    long ledval;
} mydev;

/*pci struct*/
static struct my_pci{
    struct pci_dev* pdev;
    void* hw_addr;
} my_pci;


static const struct pci_device_id pci_id_table[] = {
        { PCI_DEVICE(0x8086, 0x100E) }, //intel vendor device
        {0,0}, /* terminate list */

};


static int homework3_open(struct inode *inode, struct file *file){

    printk(KERN_INFO "Sucessfully opened!\n");

    return 0;
}

static ssize_t homework3_read(struct file *file, char __user *buf, size_t len, loff_t *offset){
int ret;
int value;
value = ioread32(my_pci.hw_addr + 0xE00);

if(*offset >= sizeof(int)){
return 0;

}

/* Make sure our user wasn't bad... */
if(!buf){

ret = -EINVAL;
goto out;
}

if(copy_to_user(buf, &value, sizeof(int))) {

ret = -EFAULT;
goto out;

}
ret = sizeof(int);
*offset += sizeof(int);

/* Good to go, so printk the thingy */
printk(KERN_INFO "User got from us %d\n", value);
out:
return ret;
}

static ssize_t homework3_write(struct file *file, const char __user *buf, size_t len, loff_t *offset){
int value;
int ret;
/* Make sure our user wasn't bad... */
if (!buf) {
ret = -EINVAL;

goto out;
}

/* Copy from the user-provided buffer */
if (copy_from_user(&value, buf, len)) {
/* uh-oh... */
ret = -EFAULT;
goto mem_out;

}

/* print what userspace gave us */
printk(KERN_INFO "Userspace wrote \"%d\" to us\n", &value);

/* writing to PCI driver */
iowrite32(value, my_pci.hw_addr + 0xE00);

/* showing what got sent to the PCI driver */
printk(KERN_INFO "Sent \"%d\" to device\n", &value);
ret = len;

mem_out:

out:
return ret;
}
static int e1000_probe(struct pci_dev* pdev, const struct pci_device_id* ent){
    int err = pci_enable_device_mem(pdev);
    if (err)
        return err;

    resource_size_t mmio_start, mmio_len;
    unsigned long bars;

    bars = pci_select_bars(pdev, IORESOURCE_MEM);

    if(pci_request_selected_regions(pdev, bars, pci_driver_name)){

        printk(KERN_ERR "Pci request region failed.\n");

        pci_release_selected_regions(pdev, pci_select_bars(pdev, IORESOURCE_MEM));
    }


    mmio_start = pci_resource_start(pdev, 0); //region 3
    printk(KERN_INFO "Mem-map starts at %lx\n", (unsigned long) mmio_start);
    mmio_len = pci_resource_len(pdev, 0);
    printk(KERN_INFO "Mem-map length at %lx\n", (unsigned long) mmio_len);

    /* this is where we know we can start reading and writing */
    /* getting the IO memory hardware adress */
    if(!(my_pci.hw_addr = ioremap(mmio_start, mmio_len))){
        err = -EIO;
        printk(KERN_ERR "IORESOURCE_MEM failed.\n");
        iounmap(my_pci.hw_addr);
        pci_release_selected_regions(pdev, pci_select_bars(pdev, IORESOURCE_MEM));

    }

    mydev.ledval = ioread32(my_pci.hw_addr + 0xE00);
    printk(KERN_INFO "SUCCESSFULLY probing");
    printk(KERN_INFO "The value to turn on and off: %lx\n", mydev.ledval);

    return 0;

}

static void e1000_remove(struct pci_dev* pdev){
    iounmap(my_pci.hw_addr);
    kfree(pdev);
    /* releasing from pci request selected regions.
    this will release referecnces in the kernel*/
    pci_release_selected_regions(pdev, pci_select_bars(pdev, IORESOURCE_MEM));
    printk(KERN_INFO "Homework 3 PCI driver is sucessfully removed.\n");
    pci_disable_device(pdev);
}

/* File operations for our devices*/
static struct file_operations mydev_fops = {
        .owner = THIS_MODULE,
        .open = homework3_open,
        .read = homework3_read,
        .write = homework3_write,
};

static struct pci_driver pci_led_driver = {
        .name = pci_driver_name,
        .id_table = pci_id_table,
        .probe = e1000_probe,
        .remove = e1000_remove,

};

int __init homework3_init(void){

    printk(KERN_INFO "Initialize Homework 3 driver module!\n");


    if(alloc_chrdev_region(&mydev.mydev_node, 0, DEVCNT, "DEVNAME")){
        printk(KERN_ERR "alloc_chrdev_region() failed!\n");
        return -1;

    }


    printk(KERN_INFO "Allocated %d devices at major: %d\n", DEVCNT, MAJOR(mydev.mydev_node));

    /*Initlize the character device and add it to the kernel*/
    cdev_init(&mydev.my_cdev, &mydev_fops);
    mydev.my_cdev.owner = THIS_MODULE;

    if(cdev_add(&mydev.my_cdev, mydev.mydev_node, DEVCNT)){
        printk(KERN_ERR "cdev_add() failed!\n");
        /* clean up chrdev allocation */
        unregister_chrdev_region(mydev.mydev_node, DEVCNT);

        return -1;
    }

    if(pci_register_driver(&pci_led_driver)){
        printk(KERN_ERR "Registering PCI driver failed.\n");
        pci_unregister_driver(&pci_led_driver);
        unregister_chrdev_region(mydev.mydev_node, DEVCNT);
        return -1;
    }

    return 0;
}


void __exit homework3_exit(void){

    /* cleaning up  pci drivers*/
    pci_unregister_driver(&pci_led_driver);
    /* destroy the cdev */
    cdev_del(&mydev.my_cdev);
    /* clean up the devices */
    unregister_chrdev_region(mydev.mydev_node, DEVCNT);

    printk(KERN_INFO "Homework 3 module unloaded!\n");

}




MODULE_AUTHOR("Phil Nevins");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.2");
module_init(homework3_init);
module_exit(homework3_exit);
