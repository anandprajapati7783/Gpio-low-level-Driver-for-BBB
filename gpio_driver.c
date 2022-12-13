/*     Creating a device driver for gpio using physical adress of gpio registers */
/*                              develope by -Anand                               */


#include <linux/kernel.h> 
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                
#include<linux/uaccess.h>              
#include <linux/ioctl.h> 
#include<asm/io.h>


/* macros  */
#define GPIO_SETDATAOUT 0X194
#define GPIO_CLRDATAOUT 0X190
#define GPIO_0     0x44E07000
#define GPIO_1     0x4804C000
#define GPIO_OE    0x134
#define PIN        (1<<28)       
#define GPIO1_SIZE 0x1000 

volatile unsigned int *gpio_addr = NULL;
volatile unsigned int *gpio_oe_addr = NULL;
volatile unsigned int *gpio_setdataout_addr = NULL;
volatile unsigned int *gpio_cleardataout_addr = NULL;

static void gpio_pin_on(void);
static void gpio_pin_off(void);

static struct class *dev_class;
static struct cdev my_cdev;
dev_t dev = 0;

static ssize_t  my_write(struct file *filp, const char *buf, size_t len, loff_t * off);

static void gpio_pin_on(void) //for Led on
{

        gpio_oe_addr = (unsigned int *)((char*)gpio_addr + GPIO_OE);
        gpio_setdataout_addr = (unsigned int *)((char*)gpio_addr + GPIO_SETDATAOUT);
        *gpio_oe_addr = (0xFFFFFFFF - PIN);
        *gpio_setdataout_addr= PIN;
	printk("led is on\n");
return;
}
static void gpio_pin_off(void)//for led off
{
        gpio_cleardataout_addr = (unsigned int *)((char*)gpio_addr + GPIO_CLRDATAOUT);
        *gpio_cleardataout_addr=PIN;
	printk("led is off\n");
        return;
}


static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .write          = my_write,
        
};

static ssize_t my_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{       static int c;
        static char databuff[10];
        memset(databuff, 0x0, sizeof(databuff));
        if (copy_from_user(databuff, buf, len))
                return 0;
        if (sscanf(databuff,"%d", &c) != 1)
                {
                        printk("Inproper data format submitted\n");
                        return len;
                }

        if(c==1)
                gpio_pin_on();
        else if(c==0)
                gpio_pin_off();
        else
                printk("Wrong input\n");
        
        return len;       
}


static int __init gpio_driver_init(void)
{
        printk("Welcome to my driver!\n");
        
        gpio_addr = (int*)ioremap(GPIO_1, PAGE_SIZE);
        if (gpio_addr == NULL)
        {
                printk("Failed to map GPIO memory to driver\n");
                return -1;
        }
        
        printk("Successfully mapped in GPIO memory\n");
        if((alloc_chrdev_region(&dev, 0, 1, "gpio_Dev")) <0){
                pr_err("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        cdev_init(&my_cdev,&fops);
 
        if((cdev_add(&my_cdev,dev,1)) < 0){
            pr_err("Cannot add the device to the system\n");
            goto r_class;
        }
 

        if((dev_class = class_create(THIS_MODULE,"my_class")) == NULL){
            pr_err("Cannot create the struct class\n");
            goto r_class;
        }
 
        if((device_create(dev_class,NULL,dev,NULL,"gpio_device")) == NULL){
            pr_err("Cannot create the Device 1\n");
            goto r_device;
        }
       return 0;

r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}

static void __exit gpio_driver_exit(void)
{
        iounmap(gpio_addr);
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
        return;
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ANAND");
MODULE_DESCRIPTION("Test of writing drivers for bbb");
MODULE_VERSION("1.0");  
