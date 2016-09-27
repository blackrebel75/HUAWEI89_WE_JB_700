#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/time.h>
#include "kd_flashlight.h"
#include <asm/io.h>
#include <asm/uaccess.h>
#include "kd_camera_hw.h"

#define USE_UNLOCKED_IOCTL

//s_add new flashlight driver here
//export funtions
MUINT32 defaultFlashlightInit(PFLASHLIGHT_FUNCTION_STRUCT *pfFunc);
MUINT32 dummyFlashlightInit(PFLASHLIGHT_FUNCTION_STRUCT *pfFunc);
MUINT32 peakFlashlightInit(PFLASHLIGHT_FUNCTION_STRUCT *pfFunc);
MUINT32 torchFlashlightInit(PFLASHLIGHT_FUNCTION_STRUCT *pfFunc);
MUINT32 constantFlashlightInit(PFLASHLIGHT_FUNCTION_STRUCT *pfFunc);


KD_FLASHLIGHT_INIT_FUNCTION_STRUCT kdFlashlightList[] =
{
    {KD_DEFAULT_FLASHLIGHT_ID, defaultFlashlightInit},
#if defined(DUMMY_FLASHLIGHT)
	{KD_DUMMY_FLASHLIGHT_ID, dummyFlashlightInit},
#endif
#if defined(PEAK_FLASHLIGHT)
	{KD_PEAK_FLASHLIGHT_ID, peakFlashlightInit},
#endif
#if defined(TORCH_FLASHLIGHT)
	{KD_TORCH_FLASHLIGHT_ID, torchFlashlightInit},
#endif
#if defined(CONSTANT_FLASHLIGHT)
	{KD_CONSTANT_FLASHLIGHT_ID, constantFlashlightInit},
#endif


/*  ADD flashlight driver before this line */
    {0,NULL}, //end of list
};
//e_add new flashlight driver here
/******************************************************************************
 * Definition
******************************************************************************/
#ifndef TRUE
#define TRUE KAL_TRUE
#endif
#ifndef FALSE
#define FALSE KAL_FALSE
#endif

/* device name and major number */
#define FLASHLIGHT_DEVNAME            "kd_camera_flashlight"

#define DELAY_MS(ms) {mdelay(ms);}//unit: ms(10^-3)
#define DELAY_US(us) {mdelay(us);}//unit: us(10^-6)
#define DELAY_NS(ns) {mdelay(ns);}//unit: ns(10^-9)
/*
    non-busy dealy(/kernel/timer.c)(CANNOT be used in interrupt context):
        ssleep(sec)
        msleep(msec)
        msleep_interruptible(msec)

    kernel timer


*/
/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[KD_CAMERA_FLASHLIGHT]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    printk(KERN_INFO PFX "%s: " fmt, __FUNCTION__ ,##arg)

#define PK_WARN(fmt, arg...)        printk(KERN_WARNING PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_NOTICE(fmt, arg...)      printk(KERN_NOTICE PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_INFO(fmt, arg...)        printk(KERN_INFO PFX "%s: " fmt, __FUNCTION__ ,##arg)
#define PK_TRC_FUNC(f)              printk(PFX "<%s>\n", __FUNCTION__);
#define PK_TRC_VERBOSE(fmt, arg...) printk(PFX fmt, ##arg)

#define DEBUG_KD_CAMERA_FLASHLIGHT
#ifdef DEBUG_KD_CAMERA_FLASHLIGHT
#define PK_DBG PK_DBG_FUNC
#define PK_TRC PK_DBG_NONE //PK_TRC_FUNC
#define PK_VER PK_DBG_NONE //PK_TRC_VERBOSE
#define PK_ERR(fmt, arg...)         printk(KERN_ERR PFX "%s: " fmt, __FUNCTION__ ,##arg)
#else
#define PK_DBG(a,...)
#define PK_TRC(a,...)
#define PK_VER(a,...)
#define PK_ERR(a,...)
#endif
/*****************************************************************************

*****************************************************************************/
static FLASHLIGHT_FUNCTION_STRUCT *g_pFlashlightFunc = NULL;

/* read and write functions.*/
static ssize_t torch_set_brightness(struct device *dev, struct device_attribute *attr,
    const char *buf, size_t count);
static ssize_t torch_get_brightness(struct device *dev, struct device_attribute *attr,char *buf);
/* Node for led brightness control */
static struct kobject *android_flashlight_kobj;
static DEVICE_ATTR(led_lightness, 0664, torch_get_brightness, torch_set_brightness);

static char torch_lightness='0';//Used to store the previous state.
static DEFINE_SPINLOCK(g_TorchStatusSMPLock);
static FLASHLIGHT_FUNCTION_STRUCT *g_pTorchFunc = NULL;//Fuc pointer to flashlight
//Torch light levels
//Step:0~7, Duty:0~31
#define CURRENT_BRIGHT_LOW_DUTY    6
#define CURRENT_BRIGHT_MEDIUM_DUTY 15
#define CURRENT_BRIGHT_HIGH_DUTY   20
#define CURRENT_BRIGHT_LOW_STEP 5
#define CURRENT_BRIGHT_MEDIUM_STEP  5
#define CURRENT_BRIGHT_HIGH_STEP   5


/*****************************************************************************

*****************************************************************************/
MINT32 default_flashlight_open(void *pArg) {
    PK_DBG("[default_flashlight_open] E\n");
    return 0;
}
MINT32 default_flashlight_release(void *pArg) {
    PK_DBG("[default_flashlight_release] E\n");
    return 0;
}
MINT32 default_flashlight_ioctl(MUINT32 cmd, MUINT32 arg) {
    int i4RetValue = 0;
    int iFlashType = (int)FLASHLIGHT_NONE;

    switch(cmd)
    {
        case FLASHLIGHTIOC_G_FLASHTYPE:
            iFlashType = FLASHLIGHT_NONE;
            if(copy_to_user((void __user *) arg , (void*)&iFlashType , _IOC_SIZE(cmd)))
            {
                PK_DBG("[default_flashlight_ioctl] ioctl copy to user failed\n");
                return -EFAULT;
            }
            break;
    	default :
    		PK_DBG("[default_flashlight_ioctl]\n");
    		break;
    }
    return i4RetValue;
}

FLASHLIGHT_FUNCTION_STRUCT	defaultFlashlightFunc=
{
	default_flashlight_open,
	default_flashlight_release,
	default_flashlight_ioctl,
};

UINT32 defaultFlashlightInit(PFLASHLIGHT_FUNCTION_STRUCT *pfFunc) { 
    if (pfFunc!=NULL) {
        *pfFunc=&defaultFlashlightFunc;
    }
    return 0;
}
/*******************************************************************************
* kdSetDriver
********************************************************************************/
int kdSetFlashlightDrv(unsigned int *pFlashlightIdx)
{
unsigned int flashlightIdx = *pFlashlightIdx;
    PK_DBG("[kdSetFlashlightDrv] flashlightIdx: %d \n",flashlightIdx);
    
    if (NULL != kdFlashlightList[flashlightIdx].flashlightInit) {
        kdFlashlightList[flashlightIdx].flashlightInit(&g_pFlashlightFunc);
        if (NULL == g_pFlashlightFunc) {
            PK_DBG("[kdSetFlashlightDrv] flashlightIdx init fail\n");
            if (NULL != kdFlashlightList[KD_DEFAULT_FLASHLIGHT_INDEX].flashlightInit) {
                kdFlashlightList[KD_DEFAULT_FLASHLIGHT_INDEX].flashlightInit(&g_pFlashlightFunc);
                if (NULL == g_pFlashlightFunc) {
                    PK_DBG("[kdSetFlashlightDrv] KD_DEFAULT_FLASHLIGHT_INDEX init fail\n");
                    return -EIO;
                }
            }
        }
    }

    //open flashlight driver
    if (g_pFlashlightFunc) {
       g_pFlashlightFunc->flashlight_open(0);
    }

	return 0;
}

/******************************************************************************
Function:       // kdSetTorchtDrv
Description:    // Get the specific flashlight driver function pointer,
                // and open the driver.
Input:          // Flash driver index, for constant flash, must be 1.
Output:         //
Return:         //Zero if no error happened.
Others:         //
******************************************************************************/
int kdSetTorchtDrv(unsigned int *pFlashlightIdx)
{
    unsigned int flashlightIdx = *pFlashlightIdx;
    PK_DBG("[Torch] kdSetTorchtDrv flashlightIdx: %d \n",flashlightIdx);

    if (NULL != kdFlashlightList[flashlightIdx].flashlightInit) {
        kdFlashlightList[flashlightIdx].flashlightInit(&g_pTorchFunc);
        if (NULL == g_pTorchFunc) {
            PK_DBG("[Torch]kdSetFlashlightDrv flashlightIdx init fail\n");
            if (NULL != kdFlashlightList[KD_DEFAULT_FLASHLIGHT_INDEX].flashlightInit) {
                kdFlashlightList[KD_DEFAULT_FLASHLIGHT_INDEX].flashlightInit(&g_pTorchFunc);
                if (NULL == g_pTorchFunc) {
                    PK_DBG("[Torch]kdSetFlashlightDrv KD_DEFAULT_FLASHLIGHT_INDEX init fail\n");
                    return -EIO;
                }
            }
        }
    }

    //open flashlight driver
    if (g_pTorchFunc) {
       g_pTorchFunc->flashlight_open(0);
    }

    return 0;
}


/******************************************************************************
Function:       // torch_get_brightness
Description:    // Get the  current flash state.
Input:          // buf:buffer to store current state.
Output:         //
Return:         // Length of flash state.
Others:         //
******************************************************************************/
static ssize_t torch_get_brightness(struct device *dev, struct device_attribute *attr,
        char *buf)
{
    return sprintf(buf, "%c\n", torch_lightness);
}
/******************************************************************************
Function:       // torch_set_brightness
Description:    // Set the  flash state according to the value passed.
Input:          // buf:buffer stored the flash state.
Output:         //
Return:         // 0 if error.
Others:         //
******************************************************************************/
static ssize_t torch_set_brightness(struct device *dev, struct device_attribute *attr,
        const char *buf, size_t count)
{
    int ret, error, drv_num=1;

    PK_DBG("[Torch]torch_set_brightness:%c, count:%d\n",buf[0], count);

    //If previous state is the same to current state, we will ignore this command.
    //if(torch_lightness == buf[0])
    //{
    //    PK_DBG("[Torch]torch_set_brightness: the same state to previously :%c!!\n",buf[0]);
    //    return count;
    //}

    if (NULL ==g_pTorchFunc)
    {
        PK_DBG("[Torch] g_pTorchFunc is NULL, re-set\n ");
        if (kdSetTorchtDrv(&drv_num))//Get flash driver and open it.
        {
            PK_ERR("[Torch] Cannot set torch driver!\n");
            return 0;
        }
    }

    if ('0' == buf[0] )//close torch
    {
        if (g_pTorchFunc)
        {
            PK_DBG("[Torch] buf[0] = 0, begin to disable the torch!\n ");  
            g_pTorchFunc->flashlight_ioctl(FLASH_IOC_SET_ONOFF,0);//turn off
            if (!g_pTorchFunc->flashlight_release(0))
            {
                 PK_DBG("[Torch] Every handle has been released!\n ");
            }
            g_pTorchFunc=NULL;

        }
        else
        {
            PK_ERR("[Torch] Fail to close torch due to NULL poiner,buf[0]:%d\n",buf[0]);
        }

     }
    else
    {

        if (g_pTorchFunc)
        {
            g_pTorchFunc->flashlight_ioctl(FLASH_IOC_SET_TIME_OUT_TIME_MS, 0);
            if ((buf[0] == '1'))
            {
                g_pTorchFunc->flashlight_ioctl(FLASH_IOC_SET_DUTY, CURRENT_BRIGHT_LOW_DUTY);
                g_pTorchFunc->flashlight_ioctl(FLASH_IOC_SET_STEP, CURRENT_BRIGHT_LOW_STEP);
            }
            else if (buf[0] == '2')
            {
                g_pTorchFunc->flashlight_ioctl(FLASH_IOC_SET_DUTY, CURRENT_BRIGHT_MEDIUM_DUTY);
                g_pTorchFunc->flashlight_ioctl(FLASH_IOC_SET_STEP, CURRENT_BRIGHT_MEDIUM_STEP);
            }
            else
            {
                g_pTorchFunc->flashlight_ioctl(FLASH_IOC_SET_DUTY, CURRENT_BRIGHT_HIGH_DUTY);
                g_pTorchFunc->flashlight_ioctl(FLASH_IOC_SET_STEP, CURRENT_BRIGHT_HIGH_STEP);
            }
            /*Turn on the flash.*/
            g_pTorchFunc->flashlight_ioctl(FLASH_IOC_SET_ONOFF,1);
        }
        else
        {
            PK_ERR("[Torch] Fail to open torch due to NULL poiner,buf[0]:%d\n",buf[0]);
        }

    }
    spin_lock_irq(&g_TorchStatusSMPLock);
    torch_lightness = buf[0];/*Store the previous state.*/
    spin_unlock_irq(&g_TorchStatusSMPLock);

    return count;
}


/*****************************************************************************

*****************************************************************************/
#ifdef USE_UNLOCKED_IOCTL
static long flashlight_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int flashlight_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
    int i4RetValue = 0;

    //PK_DBG("%x, %x \n",cmd,arg);

    switch(cmd)
    {
        case FLASHLIGHTIOC_X_SET_DRIVER:
            i4RetValue = kdSetFlashlightDrv((unsigned int*)&arg);
            break;
    	default :
    	    if (g_pFlashlightFunc) {
    	        i4RetValue = g_pFlashlightFunc->flashlight_ioctl(cmd,arg);
    	    }
    		break;
    }

    return i4RetValue;
}

static int flashlight_open(struct inode *inode, struct file *file)
{
    int i4RetValue = 0;
    PK_DBG("[flashlight_open] E\n");
    return i4RetValue;
}

static int flashlight_release(struct inode *inode, struct file *file)
{
    PK_DBG("[flashlight_release] E\n");

    if (g_pFlashlightFunc) {
        g_pFlashlightFunc->flashlight_release(0);
        g_pFlashlightFunc = NULL;
    }

    return 0;
}

/*****************************************************************************/
/* Kernel interface */
static struct file_operations flashlight_fops = {
    .owner      = THIS_MODULE,
#ifdef USE_UNLOCKED_IOCTL
    .unlocked_ioctl      = flashlight_ioctl,
#else
    .ioctl      = flashlight_ioctl,
#endif
    .open       = flashlight_open,
    .release    = flashlight_release,
};

/*****************************************************************************
Driver interface
*****************************************************************************/
struct flashlight_data{
    spinlock_t lock;
    wait_queue_head_t read_wait;
    struct semaphore sem;
};
static struct class *flashlight_class = NULL;
static struct device *flashlight_device = NULL;
static struct flashlight_data flashlight_private;
static dev_t flashlight_devno;
static struct cdev flashlight_cdev;
/****************************************************************************/
#define ALLOC_DEVNO
static int flashlight_probe(struct platform_device *dev)
{
    int ret = 0, err = 0;

	PK_DBG("[flashlight_probe] start\n");

#ifdef ALLOC_DEVNO
    ret = alloc_chrdev_region(&flashlight_devno, 0, 1, FLASHLIGHT_DEVNAME);
    if (ret) {
        PK_ERR("[flashlight_probe] alloc_chrdev_region fail: %d\n", ret);
        goto flashlight_probe_error;
    } else {
        PK_DBG("[flashlight_probe] major: %d, minor: %d\n", MAJOR(flashlight_devno), MINOR(flashlight_devno));
    }
    cdev_init(&flashlight_cdev, &flashlight_fops);
    flashlight_cdev.owner = THIS_MODULE;
    err = cdev_add(&flashlight_cdev, flashlight_devno, 1);
    if (err) {
        PK_ERR("[flashlight_probe] cdev_add fail: %d\n", err);
        goto flashlight_probe_error;
    }
#else
    #define FLASHLIGHT_MAJOR 242
    ret = register_chrdev(FLASHLIGHT_MAJOR, FLASHLIGHT_DEVNAME, &flashlight_fops);
    if (ret != 0) {
        PK_ERR("[flashlight_probe] Unable to register chardev on major=%d (%d)\n", FLASHLIGHT_MAJOR, ret);
        return ret;
    }
    flashlight_devno = MKDEV(FLASHLIGHT_MAJOR, 0);
#endif


    flashlight_class = class_create(THIS_MODULE, "flashlightdrv");
    if (IS_ERR(flashlight_class)) {
        PK_ERR("[flashlight_probe] Unable to create class, err = %d\n", (int)PTR_ERR(flashlight_class));
        goto flashlight_probe_error;
    }

    flashlight_device = device_create(flashlight_class, NULL, flashlight_devno, NULL, FLASHLIGHT_DEVNAME);
    if(NULL == flashlight_device){
        PK_ERR("[flashlight_probe] device_create fail\n");
        goto flashlight_probe_error;
    }

    /*initialize members*/
    spin_lock_init(&flashlight_private.lock);
    init_waitqueue_head(&flashlight_private.read_wait);
    //init_MUTEX(&flashlight_private.sem);
    sema_init(&flashlight_private.sem, 1);


    /* Create led brightness control node.*/
    android_flashlight_kobj = kobject_create_and_add("flashlight", NULL);
    if (android_flashlight_kobj == NULL)
    {
        PK_ERR("[Torch] %s: android_flashlight_kobj kobject_create_and_add failed\n", __func__);
        goto flashlight_probe_error;
    }
    if (sysfs_create_file(android_flashlight_kobj, &dev_attr_led_lightness.attr))
    {
        PK_ERR("[Torch] %s: sysfs_create_file failed\n", __func__);
        goto flashlight_probe_error;
    }

    PK_DBG("[flashlight_probe] Done\n");
    return 0;

flashlight_probe_error:
#ifdef ALLOC_DEVNO
    if (err == 0)
        cdev_del(&flashlight_cdev);
    if (ret == 0)
        unregister_chrdev_region(flashlight_devno, 1);
#else
    if (ret == 0)
        unregister_chrdev(MAJOR(flashlight_devno), FLASHLIGHT_DEVNAME);
#endif
    return -1;
}

static int flashlight_remove(struct platform_device *dev)
{

    PK_DBG("[flashlight_probe] start\n");

#ifdef ALLOC_DEVNO
    cdev_del(&flashlight_cdev);
    unregister_chrdev_region(flashlight_devno, 1);
#else
    unregister_chrdev(MAJOR(flashlight_devno), FLASHLIGHT_DEVNAME);
#endif
    device_destroy(flashlight_class, flashlight_devno);
    class_destroy(flashlight_class);

    sysfs_remove_file(android_flashlight_kobj, &dev_attr_led_lightness.attr);
    kobject_del(android_flashlight_kobj);

    PK_DBG("[flashlight_probe] Done\n");
    return 0;
}


static struct platform_driver flashlight_platform_driver =
{
    .probe      = flashlight_probe,
    .remove     = flashlight_remove,
    .driver     = {
        .name = FLASHLIGHT_DEVNAME,
		.owner	= THIS_MODULE,
    },
};

static struct platform_device flashlight_platform_device = {
    .name = FLASHLIGHT_DEVNAME,
    .id = 0,
    .dev = {
    }
};

static int __init flashlight_init(void)
{
    int ret = 0;
    PK_DBG("[flashlight_probe] start\n");

	ret = platform_device_register (&flashlight_platform_device);
	if (ret) {
        PK_ERR("[flashlight_probe] platform_device_register fail\n");
        return ret;
	}

    ret = platform_driver_register(&flashlight_platform_driver);
	if(ret){
		PK_ERR("[flashlight_probe] platform_driver_register fail\n");
		return ret;
	}

	PK_DBG("[flashlight_probe] done!\n");
    return ret;
}

static void __exit flashlight_exit(void)
{
    PK_DBG("[flashlight_probe] start\n");
    platform_driver_unregister(&flashlight_platform_driver);
    //to flush work queue
    //flush_scheduled_work();
    PK_DBG("[flashlight_probe] done!\n");
}

/*****************************************************************************/
module_init(flashlight_init);
module_exit(flashlight_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jackie Su <jackie.su@mediatek.com>");
MODULE_DESCRIPTION("Flashlight control Driver");


