#include "tpd.h"
#include <linux/interrupt.h>
#include <cust_eint.h>
#include <linux/i2c.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/rtpm_prio.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include "tpd_custom_ft5316.h"

#include <mach/mt_pm_ldo.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_boot.h>

#include "cust_gpio_usage.h"

 
 
extern struct tpd_device *tpd;
 
struct i2c_client *i2c_client = NULL;
struct task_struct *thread = NULL;
 
static DECLARE_WAIT_QUEUE_HEAD(waiter);
static DEFINE_MUTEX(i2c_access);
 
 
static void tpd_eint_interrupt_handler(void);
 
#ifdef MT6575 
 extern void mt65xx_eint_unmask(unsigned int line);
 extern void mt65xx_eint_mask(unsigned int line);
 extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
 extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
 extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
									  kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
									  kal_bool auto_umask);
#endif
#ifdef MT6577
	extern void mt65xx_eint_unmask(unsigned int line);
	extern void mt65xx_eint_mask(unsigned int line);
	extern void mt65xx_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
	extern unsigned int mt65xx_eint_set_sens(unsigned int eint_num, unsigned int sens);
	extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
#endif
#ifdef MT6589
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En, kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void), kal_bool auto_umask);
#endif

/*lenovo-sw xuwen1 add for modify platform 20131023 begin*/
#ifdef MT6592
extern void mt_eint_unmask(unsigned int line);
extern void mt_eint_mask(unsigned int line);
extern void mt_eint_set_hw_debounce(unsigned int eintno, unsigned int ms);
extern void mt_eint_registration(unsigned int eintno, unsigned int Dbounce_En, void (EINT_FUNC_PTR)(void), unsigned int auto_umask);
extern void mt_eint_registration_threaded(unsigned int eint_num, unsigned int pol, void(EINT_FUNC_PTR)(void), void(threaed_isr)(void), unsigned int is_auto_umask);
#endif
/*lenovo-sw xuwen1 add for modify platform 20131023 end*/
/* lenovo-sw, zhouwl, 20130109, config for chg plug-in/out */
extern kal_bool upmu_is_chr_det(void);
kal_bool tpd_first_config_det_chg = KAL_FALSE;
/* lenovo-sw, zhouwl, 20130109, config for chg plug-in/out */

static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info);
static int __devexit tpd_remove(struct i2c_client *client);
static int touch_event_handler(void *unused);
static  void tpd_up(int x, int y,int *count);
static int ft5x0x_read_reg(u8 addr, u8 *pdata);

static int boot_mode = 0;
static int tpd_flag = 0;
static int tpd_halt=0;
static int point_num = 0;
static int p_point_num = 0;
static bool discard_resume_first_eint = KAL_FALSE;
static int tpd_state = 0;
/* Lenovo-sw yexm1 add for detect charger plug-in/out */
static int usb_cable_flag = 0;
/*Begin lenovo-sw, xuwen1, 20131121, add for slide touch */

#ifdef LENOVO_CTP_SLIDE_WAKEUP
int  get_tpd_suspend_status(void);
static suspend_flag = 0;
static void tpd_slide_handle(struct work_struct *work);
static DECLARE_WORK(tpd_slide_work, tpd_slide_handle);
#endif
/*End lenovo-sw, xuwen1, 20130515, add for slide touch */
//#define TPD_CLOSE_POWER_IN_SLEEP

#define TPD_OK 0
//register define
#define SYSFS_DEBUG
#define DEVICE_MODE 0x00
#define GEST_ID 0x01
#define TD_STATUS 0x02

#define TOUCH1_XH 0x03
#define TOUCH1_XL 0x04
#define TOUCH1_YH 0x05
#define TOUCH1_YL 0x06

#define TOUCH2_XH 0x09
#define TOUCH2_XL 0x0A
#define TOUCH2_YH 0x0B
#define TOUCH2_YL 0x0C

#define TOUCH3_XH 0x0F
#define TOUCH3_XL 0x10
#define TOUCH3_YH 0x11
#define TOUCH3_YL 0x12

#define CONFIG_SUPPORT_FTS_CTP_UPG  

//#define UPGRADE_NEW
#ifdef UPGRADE_NEW
#define    BL_VERSION_LZ4        0
#define    BL_VERSION_Z7        1
#define    BL_VERSION_GZF        2
#define FT_UPGRADE_EARSE_DELAY 1500
#define IC_FT5X36	3
#define DEVICE_IC_TYPE	IC_FT5X36
#endif

//register define
//#define ESD_CHECK
#define FTS_CTL_IIC

#ifdef FTS_CTL_IIC
#include "focaltech_ctl.h"
#endif

#ifdef SYSFS_DEBUG
#include "ft5x06_ex_fun.h"
#endif

#define TPD_RESET_ISSUE_WORKAROUND

#define TPD_MAX_RESET_COUNT 3

#ifdef ESD_CHECK
static struct delayed_work ctp_read_id_work;
static struct workqueue_struct * ctp_read_id_workqueue = NULL;
#endif

#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
static int tpd_calmat_local[8]     = TPD_CALIBRATION_MATRIX;
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif

#ifdef LENOVO_CTP_AREA_LOCK
extern int get_tpd_area_status(void);
#endif

#define VELOCITY_CUSTOM_FT5206
#ifdef VELOCITY_CUSTOM_FT5206
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

// for magnify velocity********************************************

#ifndef TPD_VELOCITY_CUSTOM_X
#define TPD_VELOCITY_CUSTOM_X 10
#endif
#ifndef TPD_VELOCITY_CUSTOM_Y
#define TPD_VELOCITY_CUSTOM_Y 10
#endif

/*lenovo-sw xuwen1 add 20140126 for read fw-version begin*/
extern struct tpd_version_info *tpd_info_t;	
extern unsigned int have_correct_setting;
static int get_tpd_info(void);
/*lenovo-sw xuwen1 add 20140126 for read fw-version end*/

#define TOUCH_IOC_MAGIC 'A'

#define TPD_GET_VELOCITY_CUSTOM_X _IO(TOUCH_IOC_MAGIC,0)
#define TPD_GET_VELOCITY_CUSTOM_Y _IO(TOUCH_IOC_MAGIC,1)

int g_v_magnify_x =TPD_VELOCITY_CUSTOM_X;
int g_v_magnify_y =TPD_VELOCITY_CUSTOM_Y;

/*lenovo-sw, xuwen1, 20131121, add for slide touch begin*/
#if defined(LENOVO_CTP_SLIDE_WAKEUP) 
static void tpd_slide_handle(struct work_struct *work)
{
	int ret;
	u8  state;
	u8 data = 0x00;
	u8 val = 0x00;
	u8 vall =0x00;
/*
	ret = i2c_read_bytes(i2c_client_point, 0x814b, &state, 1);
		//printk("*************[wj]_%s, Reg[0x814b] = 0x%02x.\n", __func__, state);
*/

		ft5x0x_read_reg(0xb8, &val);
		printk("[xw]Reg 0xb8 value is 0x%02x.\n", val);
		if(val == 0x1 )
		{
			suspend_flag = 0;
			//ret = i2c_write_bytes(i2c_client_point, 0x814b, &data, 1);
/*lenovo-sw lixh10 remove 20140429 for slide wakeup slowly  begin*/
			/*
			input_report_key(tpd->dev, KEY_POWER, 1);
			input_sync(tpd->dev);
			input_report_key(tpd->dev, KEY_POWER, 0);
			input_sync(tpd->dev);
			*/
/*lenovo-sw lixh10 remove 20140429 for slide wakeup slowly  begin*/
			input_report_key(tpd->dev, KEY_SLIDE, 1);
			input_sync(tpd->dev);
			input_report_key(tpd->dev, KEY_SLIDE, 0);
			input_sync(tpd->dev);		
		}
}
#endif
/*lenovo-sw, xuwen1, 20131121, add for slide touch end */
int ft5x0x_i2c_Read(struct i2c_client *client, char *writebuf,
		    int writelen, char *readbuf, int readlen)
{
	int ret;

	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
			 },
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret < 0)
			dev_err(&client->dev, "f%s: i2c read error.\n",
				__func__);
	} else {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 1);
		if (ret < 0)
			dev_err(&client->dev, "%s:i2c read error.\n", __func__);
	}
	return ret;
}
/*write data by i2c*/
int ft5x0x_i2c_Write(struct i2c_client *client, char *writebuf, int writelen)
{
	int ret;

	struct i2c_msg msg[] = {
		{
		 .addr = client->addr,
		 .flags = 0,
		 .len = writelen,
		 .buf = writebuf,
		 },
	};

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret < 0)
		dev_err(&client->dev, "%s i2c write error.\n", __func__);

	return ret;
}
static int tpd_misc_open(struct inode *inode, struct file *file)
{
/*
	file->private_data = adxl345_i2c_client;

	if(file->private_data == NULL)
	{
		printk("tpd: null pointer!!\n");
		return -EINVAL;
	}
	*/
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int tpd_misc_release(struct inode *inode, struct file *file)
{
	//file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
//static int adxl345_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
//       unsigned long arg)
static long tpd_unlocked_ioctl(struct file *file, unsigned int cmd,
       unsigned long arg)
{
	//struct i2c_client *client = (struct i2c_client*)file->private_data;
	//struct adxl345_i2c_data *obj = (struct adxl345_i2c_data*)i2c_get_clientdata(client);	
	//char strbuf[256];
	void __user *data;
	
	long err = 0;
	
	if(_IOC_DIR(cmd) & _IOC_READ)
	{
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}

	if(err)
	{
		printk("tpd: access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch(cmd)
	{
		case TPD_GET_VELOCITY_CUSTOM_X:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}			
			
			if(copy_to_user(data, &g_v_magnify_x, sizeof(g_v_magnify_x)))
			{
				err = -EFAULT;
				break;
			}				 
			break;

	   case TPD_GET_VELOCITY_CUSTOM_Y:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;	  
			}			
			
			if(copy_to_user(data, &g_v_magnify_y, sizeof(g_v_magnify_y)))
			{
				err = -EFAULT;
				break;
			}				 
			break;


		default:
			printk("tpd: unknown IOCTL: 0x%08x\n", cmd);
			err = -ENOIOCTLCMD;
			break;
			
	}

	return err;
}


static struct file_operations tpd_fops = {
//	.owner = THIS_MODULE,
	.open = tpd_misc_open,
	.release = tpd_misc_release,
	.unlocked_ioctl = tpd_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice tpd_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "touch",
	.fops = &tpd_fops,
};

//**********************************************
#endif

struct touch_info {
    int y[5];
    int x[5];
    int p[5];
    int finger_id[5];
    int count;
	#if defined(LENOVO_AREA_TOUCH)//lenovo jixu modify
    int size[5];
	#endif
};
 
 static const struct i2c_device_id ft5206_tpd_id[] = {{"ft5206",0},{}};
 //unsigned short force[] = {0,0x70,I2C_CLIENT_END,I2C_CLIENT_END}; 
 //static const unsigned short * const forces[] = { force, NULL };
 //static struct i2c_client_address_data addr_data = { .forces = forces, };
 static struct i2c_board_info __initdata ft5206_i2c_tpd={ I2C_BOARD_INFO("ft5206", (0x70>>1))};
 
 
 static struct i2c_driver tpd_i2c_driver = {
  .driver = {
	 .name = "ft5206",//.name = TPD_DEVICE,
//	 .owner = THIS_MODULE,
  },
  .probe = tpd_probe,
  .remove = __devexit_p(tpd_remove),
  .id_table = ft5206_tpd_id,
  .detect = tpd_detect,
//  .address_data = &addr_data,
 };
 
 #ifdef CONFIG_SUPPORT_FTS_CTP_UPG
static u8 *CTPI2CDMABuf_va = NULL;
static u32 CTPI2CDMABuf_pa = NULL;
typedef enum
{
    ERR_OK,
    ERR_MODE,
    ERR_READID,
    ERR_ERASE,
    ERR_STATUS,
    ERR_ECC,
    ERR_DL_ERASE_FAIL,
    ERR_DL_PROGRAM_FAIL,
    ERR_DL_VERIFY_FAIL
}E_UPGRADE_ERR_TYPE;

typedef unsigned char         FTS_BYTE;     //8 bit
typedef unsigned short        FTS_WORD;    //16 bit
typedef unsigned int          FTS_DWRD;    //16 bit
typedef unsigned char         FTS_BOOL;    //8 bit

#define FTS_NULL                0x0
#define FTS_TRUE                0x01
#define FTS_FALSE              0x0

#define I2C_CTPM_ADDRESS       0x70

/***********************************************************************************************
Name	:	ft5x0x_i2c_rxdata 

Input	:	*rxdata
                     *length

Output	:	ret

function	:	

***********************************************************************************************/
static int ft5x0x_i2c_rxdata(char *rxdata, int length)
{
	int ret;

	struct i2c_msg msgs[] = {
		{
			.addr	= i2c_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rxdata,
		},
		{
			.addr	= i2c_client->addr,
			.flags	= I2C_M_RD,
			.len	= length,
			.buf	= rxdata,
		},
	};

    //msleep(1);
	ret = i2c_transfer(i2c_client->adapter, msgs, 2);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);
	
	return ret;
}
/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static int ft5x0x_i2c_txdata(char *txdata, int length)
{
	int ret;

	struct i2c_msg msg[] = {
		{
			.addr	= i2c_client->addr,
			.flags	= 0,
			.len	= length,
			.buf	= txdata,
		},
	};

   	//msleep(1);
	ret = i2c_transfer(i2c_client->adapter, msg, 1);
	if (ret < 0)
		pr_err("%s i2c write error: %d\n", __func__, ret);

	return ret;
}
/***********************************************************************************************
Name	:	 ft5x0x_write_reg

Input	:	addr -- address
                     para -- parameter

Output	:	

function	:	write register of ft5x0x

***********************************************************************************************/
static int ft5x0x_write_reg(u8 addr, u8 para)
{
    u8 buf[3];
    int ret = -1;

    buf[0] = addr;
    buf[1] = para;
    ret = ft5x0x_i2c_txdata(buf, 2);
    if (ret < 0) {
        pr_err("write reg failed! %#x ret: %d", buf[0], ret);
        return -1;
    }
    
    return 0;
}


/***********************************************************************************************
Name	:	ft5x0x_read_reg 

Input	:	addr
                     pdata

Output	:	

function	:	read register of ft5x0x

***********************************************************************************************/
static int ft5x0x_read_reg(u8 addr, u8 *pdata)
{
	int ret;
	u8 buf[2] = {0};

	buf[0] = addr;
	struct i2c_msg msgs[] = {
		{
			.addr	= i2c_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= buf,
		},
		{
			.addr	= i2c_client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= buf,
		},
	};

    //msleep(1);
	ret = i2c_transfer(i2c_client->adapter, msgs, 2);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);

	*pdata = buf[0];
	return ret;
  
}


/***********************************************************************************************
Name	:	 ft5x0x_read_fw_ver

Input	:	 void
                     

Output	:	 firmware version 	

function	:	 read TP firmware version

***********************************************************************************************/
static unsigned char ft5x0x_read_fw_ver(void)
{
	unsigned char ver;
	ft5x0x_read_reg(0xa6, &ver);
	printk("[TSP]%s, version = %x\n", __func__, ver);
	return(ver);
}
static unsigned char ft5x0x_read_ID_ver(void)
{
	unsigned char ver;
	ft5x0x_read_reg(0xa8, &ver);
	printk("[TSP]%s, version = %x\n", __func__, ver);
	return(ver);
}


static void delay_qt_ms(unsigned long  w_ms)
{
    unsigned long i;
    unsigned long j;

    for (i = 0; i < w_ms; i++)
    {
        for (j = 0; j < 1000; j++)
        {
            udelay(1);
        }
    }
}
/*lenovo-sw xuwen1 add 20131218 for get FW begin*/
unsigned int getVersion(void)
{
	int i = 0;	
	unsigned int version=0;	
	do	
	{	
	i ++;	
	version =ft5x0x_read_fw_ver();	
	delay_qt_ms(2);
	}while( i < 5 );	
	printk("[xw]version is 0x%08x.\n", version);	
	return version;	
}	
EXPORT_SYMBOL(getVersion);

unsigned int tpd_read_id_version(void)
	{
	int i = 0;	
	unsigned int version=0;	
	do	
	{	
	i ++;	
	version = ft5x0x_read_ID_ver();	
	delay_qt_ms(2);
	}while( i < 5 );	
	printk("[xw]version is 0x%08x.\n", version);	
	return version;	
}	
EXPORT_SYMBOL(tpd_read_id_version);


static int get_tpd_info(void)
{
char *ic_name = "FocalTech";
tpd_info_t ->name = ic_name;
tpd_info_t ->fw_num = getVersion();
tpd_info_t ->types = tpd_read_id_version();

have_correct_setting = 1;
}
/*lenovo-sw xuwen1 add 20131218 for get FW end*/

/*
[function]: 
    callback: read data from ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[out]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
FTS_BOOL i2c_read_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;
    
    ret=i2c_master_recv(i2c_client, pbt_buf, dw_lenth);

    if(ret<=0)
    {
        printk("[TSP]i2c_read_interface error\n");
        return FTS_FALSE;
    }
  
    return FTS_TRUE;
}

/*
[function]: 
    callback: write data to ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[in]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
FTS_BOOL i2c_write_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;
    ret=i2c_master_send(i2c_client, pbt_buf, dw_lenth);
    if(ret<=0)
    {
        printk("[TSP]i2c_write_interface error line = %d, ret = %d\n", __LINE__, ret);
        return FTS_FALSE;
    }

    return FTS_TRUE;
}

/*
[function]: 
    send a command to ctpm.
[parameters]:
    btcmd[in]        :command code;
    btPara1[in]    :parameter 1;    
    btPara2[in]    :parameter 2;    
    btPara3[in]    :parameter 3;    
    num[in]        :the valid input parameter numbers, if only command code needed and no parameters followed,then the num is 1;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL cmd_write(FTS_BYTE btcmd,FTS_BYTE btPara1,FTS_BYTE btPara2,FTS_BYTE btPara3,FTS_BYTE num)
{
    FTS_BYTE write_cmd[4] = {0};

    write_cmd[0] = btcmd;
    write_cmd[1] = btPara1;
    write_cmd[2] = btPara2;
    write_cmd[3] = btPara3;
    return i2c_write_interface(I2C_CTPM_ADDRESS, write_cmd, num);
}

/*
[function]: 
    write data to ctpm , the destination address is 0.
[parameters]:
    pbt_buf[in]    :point to data buffer;
    bt_len[in]        :the data numbers;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL byte_write(FTS_BYTE* pbt_buf, FTS_DWRD dw_len)
{
    
    return i2c_write_interface(I2C_CTPM_ADDRESS, pbt_buf, dw_len);
}


static int CTPDMA_i2c_write(FTS_BYTE slave,FTS_BYTE* pbt_buf, FTS_DWRD dw_len)
{
    
	int i = 0;
	for(i = 0 ; i < dw_len; i++)
	{
		CTPI2CDMABuf_va[i] = pbt_buf[i];
	}

	if(dw_len <= 8)
	{
		//i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;
		//MSE_ERR("Sensor non-dma write timing is %x!\r\n", this_client->timing);
		return i2c_master_send(i2c_client, pbt_buf, dw_len);
	}
	else
	{
		i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG | I2C_DMA_FLAG;
		//MSE_ERR("Sensor dma timing is %x!\r\n", this_client->timing);
		return i2c_master_send(i2c_client, CTPI2CDMABuf_pa, dw_len);
	}    
}


static int CTPDMA_i2c_read(FTS_BYTE slave, FTS_BYTE *buf, FTS_DWRD len)
{
	int i = 0, err = 0;

	if(len < 8)
	{
		i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG;
		//MSE_ERR("Sensor non-dma read timing is %x!\r\n", this_client->timing);
		return i2c_master_recv(i2c_client, buf, len);
	}
	else
	{
		i2c_client->addr = i2c_client->addr & I2C_MASK_FLAG | I2C_DMA_FLAG;
		//MSE_ERR("Sensor dma read timing is %x!\r\n", this_client->timing);
		err = i2c_master_recv(i2c_client, CTPI2CDMABuf_pa, len);
		
	    if(err < 0)
	    {
			return err;
		}

		for(i = 0; i < len; i++)
		{
			buf[i] = CTPI2CDMABuf_va[i];
		}
	}
}


/*
[function]: 
    read out data from ctpm,the destination address is 0.
[parameters]:
    pbt_buf[out]    :point to data buffer;
    bt_len[in]        :the data numbers;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL byte_read(FTS_BYTE* pbt_buf, FTS_BYTE bt_len)
{
    return i2c_read_interface(I2C_CTPM_ADDRESS, pbt_buf, bt_len);
}


/*
[function]: 
    burn the FW to ctpm.
[parameters]:(ref. SPEC)
    pbt_buf[in]    :point to Head+FW ;
    dw_lenth[in]:the length of the FW + 6(the Head length);    
    bt_ecc[in]    :the ECC of the FW
[return]:
    ERR_OK        :no error;
    ERR_MODE    :fail to switch to UPDATE mode;
    ERR_READID    :read id fail;
    ERR_ERASE    :erase chip fail;
    ERR_STATUS    :status error;
    ERR_ECC        :ecc error.
*/
void fts_ctpm_clb(void)
{
	unsigned char uc_temp = 0x00;
	unsigned int i = 0;

	/*start auto CLB */
	msleep(200);
	ft5x0x_write_reg(0x00, 0x40); /*make sure already enter factory mode */
	msleep(100);
	ft5x0x_write_reg(0x02, 0x04); /*write command to start calibration */
	msleep(300);
#ifdef UPGRADE_NEW
	if (DEVICE_IC_TYPE == IC_FT5X36) {
		for(i=0;i<100;i++)
		{
			ft5x0x_read_reg(0x02, &uc_temp);
			if (0x02 == uc_temp ||
				0xFF == uc_temp)
			{
				/*if 0x02, then auto clb ok, else 0xff, auto clb failure*/
			    break;
			}
			msleep(20);	    
		}
	} else {
		for(i=0;i<100;i++)
		{
			ft5x0x_read_reg(0, &uc_temp);
			if (0x0 == ((uc_temp&0x70)>>4))  /*return to normal mode, calibration finish*/
			{
			    break;
			}
			msleep(20);	    
		}
	}

#else
	for (i = 0; i < 100; i++) {
		ft5x0x_read_reg(0x00, &uc_temp);
		/*return to normal mode, calibration finish */
		if (0x0 == ((uc_temp & 0x70) >> 4))
			break;
	}
#endif
	/*calibration OK */
	msleep(300);
	ft5x0x_write_reg(0x00, 0x40);
	msleep(100);	/*make sure already enter factory mode */
	ft5x0x_write_reg(0x02, 0x05);
	msleep(300);
	ft5x0x_write_reg(0x00, 0x00);	/*return to normal mode */
	msleep(300);
}

#define    FTS_PACKET_LENGTH        2

static unsigned char CTPM_FW[]=
{
#ifdef UPGRADE_NEW

//#include "Lenovo_Snoopy_DVT_01000043_20131115_app.i"
#include "Lenovo_Smart-T+_Test_05000080_20131219_app.i"

#else
   #include "Lenovo_SmartT_MP_0400000F_20130910_app.i" 
  // #include "Lenovo_SmartT_MP_05000010_20131024_app.i" 
#endif
};

E_UPGRADE_ERR_TYPE  fts_ctpm_fw_upgrade_1(FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    FTS_BYTE reg_val[2] = {0};
    FTS_DWRD i = 0;

    FTS_DWRD  packet_number;
    FTS_DWRD  j;
    FTS_DWRD  temp;
    FTS_DWRD  lenght;
    FTS_BYTE  packet_buf[FTS_PACKET_LENGTH + 6];
    FTS_BYTE  auc_i2c_write_buf[10];
    FTS_BYTE bt_ecc;
    int      i_ret;
	int ret;
	unsigned char ver;
	/*lenovo-sw xuwen1 add 20131114 for update 5336 begin*/
	#ifdef UPGRADE_NEW
	FTS_BYTE is_5336_new_bootloader = 0;
	FTS_BYTE is_5336_fwsize_30 = 0;
	int	fw_filenth = sizeof(CTPM_FW);
	//printk(KERN_WARNING "fw_filenth %d \n",fw_filenth);
	if(CTPM_FW[fw_filenth-12] == 30)
	{
		is_5336_fwsize_30 = 1;
	}
	else 
	{
		is_5336_fwsize_30 = 0;
	}
#endif
	
    /*********Step 1:Reset  CTPM *****/
    /*write 0xaa to register 0xfc*/
 #ifdef UPGRADE_NEW
   mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
    msleep(30); //original 10 
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	//msleep(10);//add this line 
    printk("[TSP] Step 1: Reset CTPM test\n");
   
    delay_qt_ms(35);   //original 50
 #else
       mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
    msleep(10); //original 10 
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	//msleep(10);//add this line 
    printk("[TSP] Step 1: Reset CTPM test\n");
   
    delay_qt_ms(50);   //original 50
   #endif
   /*lenovo-sw xuwen1 add 20131114 for update 5336 end*/
    /*********Step 2:Enter upgrade mode *****/
    auc_i2c_write_buf[0] = 0x55;
    auc_i2c_write_buf[1] = 0xaa;
    do
    {
        i ++;
        i_ret = ft5x0x_i2c_txdata(auc_i2c_write_buf, 2);
		
#ifdef UPGRADE_NEW
        delay_qt_ms(30);
#else
        delay_qt_ms(5);
#endif

    }while(i_ret <= 0 && i < 5 );

    /*********Step 3:check READ-ID***********************/        
    cmd_write(0x90,0x00,0x00,0x00,4);
	i=0;
	i_ret=0;
    do
    {
        i ++;
        i_ret = byte_read(reg_val,2);
        delay_qt_ms(10);
    }while(i_ret <= 0 && i < 5 );
        printk("[TSP] Step 2: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
  
   // if (reg_val[0] == 0x79 && reg_val[1] == 0x3) //0x03 
  //  {
        printk("[TSP] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
   // }
   /*
    else
    {
        return ERR_READID;
        //i_is_new_protocol = 1;
    }*/
/*lenovo-sw xuwen1 add 20131114 for update 5336 begin*/
   #ifdef UPGRADE_NEW //NK
    cmd_write(0xcd,0x0,0x00,0x00,1);
    byte_read(reg_val,1);
    printk("[FTS] bootloader version = 0x%x\n", reg_val[0]);

	if (reg_val[0] <= 4)
	{
		is_5336_new_bootloader = BL_VERSION_LZ4 ;
	}
	else if(reg_val[0] == 7)
	{
		is_5336_new_bootloader = BL_VERSION_Z7 ;
	}
	else if(reg_val[0] >= 0x0f)
	{
		is_5336_new_bootloader = BL_VERSION_GZF ;
	}



#endif
/*lenovo-sw xuwen1 add 20131114 for update 5336 end*/
     /*********Step 4:erase app*******************************/
#ifdef UPGRADE_NEW
	if(is_5336_fwsize_30)
	{
		printk("[FTS] Step 4:erase app 1\n");
		auc_i2c_write_buf[0] = 0x61;
		ft5x0x_i2c_Write(i2c_client, auc_i2c_write_buf, 1); /*erase app area*/	
   		 msleep(FT_UPGRADE_EARSE_DELAY); 

		 auc_i2c_write_buf[0] = 0x63;
		ft5x0x_i2c_Write(i2c_client, auc_i2c_write_buf, 1); /*erase app area*/	
   		 msleep(50);
	}
	else
	{
		printk("[FTS] Step 4:erase app 2\n");
		auc_i2c_write_buf[0] = 0x61;
		ft5x0x_i2c_Write(i2c_client, auc_i2c_write_buf, 1); /*erase app area*/	
   		msleep(FT_UPGRADE_EARSE_DELAY); 
	}


#else
    ret = cmd_write(0x61,0x00,0x00,0x00,1);
   
    delay_qt_ms(1500);
#endif
    printk("[TSP] Step 4: erase.ret=%d\n",ret);

    /*********Step 5:write firmware(FW) to ctpm flash*********/
    bt_ecc = 0;
    printk("[TSP] Step 5: start upgrade. \n");
#ifdef UPGRADE_NEW
	if(is_5336_new_bootloader == BL_VERSION_LZ4 || is_5336_new_bootloader == BL_VERSION_Z7 )
	{
		dw_lenth = dw_lenth - 8;
		printk("[TSP] Step 5: ft5336 update 1 LZ4 or Z7 \n");
	}
	else if(is_5336_new_bootloader == BL_VERSION_GZF) dw_lenth = dw_lenth - 14;

#else 

    dw_lenth = dw_lenth - 8;
#endif

    packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
    packet_buf[0] = 0xbf;
    packet_buf[1] = 0x00;
    for (j=0;j<packet_number;j++)
    {
        temp = j * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        lenght = FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(lenght>>8);
        packet_buf[5] = (FTS_BYTE)lenght;

        for (i=0;i<FTS_PACKET_LENGTH;i++)
        {
            packet_buf[6+i] = pbt_buf[j*FTS_PACKET_LENGTH + i]; 
            bt_ecc ^= packet_buf[6+i];
        }
        
        ret=CTPDMA_i2c_write(0x70, &packet_buf[0],FTS_PACKET_LENGTH + 6);
              //printk("[TSP] 111 ret 0x%x \n", ret);
        //delay_qt_ms(FTS_PACKET_LENGTH/6 + 1);
        if ((j * FTS_PACKET_LENGTH % 1024) == 0)
        {
              printk("[TSP] upgrade the 0x%x th byte.\n", ((unsigned int)j) * FTS_PACKET_LENGTH);
        }
    }

    if ((dw_lenth) % FTS_PACKET_LENGTH > 0)
    {
        temp = packet_number * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;

        temp = (dw_lenth) % FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;

        for (i=0;i<temp;i++)
        {
            packet_buf[6+i] = pbt_buf[ packet_number*FTS_PACKET_LENGTH + i]; 
            bt_ecc ^= packet_buf[6+i];
        }
              printk("[TSP]temp 0x%x \n", temp);
        ret = CTPDMA_i2c_write(0x70, &packet_buf[0],temp+6);    
              printk("[TSP] 222 ret 0x%x \n", ret);
        delay_qt_ms(20);
    }
#ifdef UPGRADE_NEW
	if(is_5336_new_bootloader == BL_VERSION_LZ4 || is_5336_new_bootloader == BL_VERSION_Z7 )
	{
		for (i = 0; i<6; i++)
		{
			if (is_5336_new_bootloader  == BL_VERSION_Z7 && DEVICE_IC_TYPE==IC_FT5X36) 
			{
				temp = 0x7bfa + i;
			}
			else if(is_5336_new_bootloader == BL_VERSION_LZ4)
			{
				temp = 0x6ffa + i;
			}
			packet_buf[2] = (FTS_BYTE)(temp>>8);
			packet_buf[3] = (FTS_BYTE)temp;
			temp =1;
			packet_buf[4] = (FTS_BYTE)(temp>>8);
			packet_buf[5] = (FTS_BYTE)temp;
			packet_buf[6] = pbt_buf[ dw_lenth + i]; 
			bt_ecc ^= packet_buf[6];
  
			CTPDMA_i2c_write(0x70,&packet_buf[0],7);
			delay_qt_ms(20);
		}
		printk("[TSP] Step 5:  ft5336 update 2nd \n");
	}
	else if(is_5336_new_bootloader == BL_VERSION_GZF)
	{
		for (i = 0; i<12; i++)
		{ 
		      printk("[TSP-xw]fwsize=%d,IC_TYPE=%d,IC_FT5X36\n",is_5336_fwsize_30,DEVICE_IC_TYPE,IC_FT5X36);
			if(1)//if (is_5336_fwsize_30 && DEVICE_IC_TYPE==IC_FT5X36) //if (1)
			{
				temp = 0x7ff4 + i;
			}
			else if (DEVICE_IC_TYPE==IC_FT5X36) 
			{
				temp = 0x7bf4 + i;
			}
//			packet_buf[0] = 0xbf;
//			packet_buf[1] = 0x00;
			packet_buf[2] = (FTS_BYTE)(temp>>8);
			packet_buf[3] = (FTS_BYTE)temp;
			temp =1;
			packet_buf[4] = (FTS_BYTE)(temp>>8);
			packet_buf[5] = (FTS_BYTE)temp;
			packet_buf[6] = pbt_buf[ dw_lenth + i]; 
			bt_ecc ^= packet_buf[6];
  
			CTPDMA_i2c_write(0x70,&packet_buf[0],7);
			delay_qt_ms(20);

		}
		printk("[TSP] Step 5:  ft5336 update 3rd \n");
	}

#else

    //send the last six byte
    for (i = 0; i<6; i++)
    {
        temp = 0x6ffa + i;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        temp =1;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;
        packet_buf[6] = pbt_buf[ dw_lenth + i]; 
        bt_ecc ^= packet_buf[6];

        CTPDMA_i2c_write(0x70,&packet_buf[0],7);  
        delay_qt_ms(20);
    }
    printk("[TSP] Step 5:  update f5406 \n");
#endif

    /*********Step 6: read out checksum***********************/
    /*send the opration head*/
    //cmd_write(0xcc,0x00,0x00,0x00,1);
    //byte_read(reg_val,1);
i2c_smbus_read_i2c_block_data(i2c_client, 0xcc, 1, &(reg_val[0]));
    printk("[TSP] Step 6:  ecc read 0x%x, new firmware 0x%x. \n", reg_val[0], bt_ecc);
    if(reg_val[0] != bt_ecc)
    {
        //return ERR_ECC;
    }

    /*********Step 7: reset the new FW***********************/
    //cmd_write(0x07,0x00,0x00,0x00,1);
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
    msleep(10);  
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
    return ERR_OK;
}


static int fts_ctpm_fw_upgrade_with_i_file(void)
{
   FTS_BYTE*     pbt_buf = FTS_NULL;
   int i_ret;
   unsigned char version=0;
    FTS_BYTE flag;
    FTS_DWRD i = 0;
    //=========FW upgrade========================*/
   pbt_buf = CTPM_FW;
#if 0	
	printk("version=%x,pbt_buf[sizeof(CTPM_FW)-2]=%d\n",version,pbt_buf[sizeof(CTPM_FW)-2]);
	if(ft5x0x_read_ID_ver() != pbt_buf[sizeof(CTPM_FW)-1])
	{
        return;
	}
	
    do
    {
        i ++;
        version =ft5x0x_read_fw_ver();
        delay_qt_ms(2);
    }while( i < 5 );
    
	//if(version==pbt_buf[sizeof(CTPM_FW)-2])
	//{
	//	return;
	//}
#else
	do
	{
		i ++;
		version =ft5x0x_read_fw_ver();
		delay_qt_ms(2);
	}while( i < 5 );
	printk("[wj]version is 0x%02x.\n", version);
	/*Lenovo huangdra 20130617 close firmware update func*/
	//if(version>=0xa)
	//{
	//	printk("[wj]version is new, no need to update.\n");
	//	return;
	//}
#endif
   /*call the upgrade function*/
   i_ret =  fts_ctpm_fw_upgrade_1(pbt_buf,sizeof(CTPM_FW));
   if (i_ret != 0)
   {
       //error handling ...
       //TBD
   }
	msleep(200);  
    /*ft5x0x_write_reg(0xfc,0x04);
	msleep(4000);
	flag=0;
	i2c_smbus_read_i2c_block_data(i2c_client, 0xFC, 1, &flag);
	printk("flag=%d\n",flag);*/
#if 0//def CONFIG_SUPPORT_FTS_CTP_UPG
	fts_ctpm_clb();
#endif
   return i_ret;
}

unsigned char fts_ctpm_get_upg_ver(void)
{
    unsigned int ui_sz;
    ui_sz = sizeof(CTPM_FW);
    if (ui_sz > 2)
    {
        return CTPM_FW[ui_sz - 2];
    }
    else
    {
        //TBD, error handling?
        return 0xff; //default value
    }
}
#endif

/*lenovo-sw xuwen1 add 20131121 for read version begin*/
unsigned int tpd_read_fw_version(void)
{
	int i = 0;
	unsigned int version=0;
	do
	{
		i ++;
		version =ft5x0x_read_fw_ver();
		delay_qt_ms(2);
	}while( i < 5 );
	printk("[wj]version is 0x%02x.\n", version);
	return version;
}
EXPORT_SYMBOL(tpd_read_fw_version);
/*lenovo-sw xuwen1 add 20131121 for read version end*/
#ifdef ESD_CHECK	
static void ESD_read_id_workqueue(struct work_struct *work)
{
	char data;
	if(tpd_halt) 
		return; 
	i2c_smbus_read_i2c_block_data(i2c_client, 0x88, 1, &data);
	TPD_DEBUG("ESD_read_id_workqueue data: %d\n", data);
	
	if((data > 5)&&(data < 10))
	{
		//add_timer();
	}
	else
	{
//lenovo -sw xuwen1 modify 20131023 for match platform
 	mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
	
		 if(tpd_state)
		 {
			 input_mt_sync(tpd->dev);
	                input_sync(tpd->dev);
			tpd_state = 0;
		 }
		msleep(5);  
	
		    //power on, need confirm with SA
		hwPowerDown(MT65XX_POWER_LDO_VGP2,"TP");
		msleep(5);  
		hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
		msleep(100);
		mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
		msleep(10);  
		mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
//lenovo-sw xuwen1 modify 20131023 for match platform 
		mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 

		msleep(200);
	}
	if(tpd_halt) 
//lenovo-sw xuwen1 modify 20131023 for match platform 
		mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM); 
	else 
		queue_delayed_work(ctp_read_id_workqueue, &ctp_read_id_work,400); //schedule a work for the first detection					

}
#endif
#if defined(LENOVO_AREA_TOUCH)//lenovo jixu modify

static void tpd_down(int x, int y, int p,int finger_id, int Xw, int Yw) {
#else
static  void tpd_down(int x, int y, int p,int finger_id) {
#endif
	// input_report_abs(tpd->dev, ABS_PRESSURE, p);
	if(x > TPD_RES_X)
	{
		TPD_DEBUG("warning: IC have sampled wrong value.\n");;
		return;
	}
	 input_report_key(tpd->dev, BTN_TOUCH, 1);
	 input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 1);
	 input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
	 input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
	#if defined(LENOVO_AREA_TOUCH)//lenovo jixu add
	input_report_abs(tpd->dev, ABS_MT_POSITION_X_W, Xw);
	input_report_abs(tpd->dev, ABS_MT_POSITION_Y_W, Yw);
	#endif
	 input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, finger_id);
	 
	 printk("D[%4d %4d %4d] ", x, y, p);
	 input_mt_sync(tpd->dev);
     if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
     {   
       tpd_button(x, y, 1);  
     }
	 if(y > TPD_RES_Y) //virtual key debounce to avoid android ANR issue
	 {
         //msleep(50);
		 printk("D virtual key \n");
	 }
	 TPD_EM_PRINT(x, y, x, y, p-1, 1);
 }
 
static  void tpd_up(int x, int y,int *count) {
	 //if(*count>0) {
		 //input_report_abs(tpd->dev, ABS_PRESSURE, 0);
		 input_report_key(tpd->dev, BTN_TOUCH, 0);
		 //input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);
		 //input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
		 //input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
		 printk("U[%4d %4d %4d] ", x, y, 0);
		 input_mt_sync(tpd->dev);
		 TPD_EM_PRINT(x, y, x, y, 0, 0);
	//	 (*count)--;
     if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())
     {   
        tpd_button(x, y, 0); 
     }   		 

 }

 static int tpd_touchinfo(struct touch_info *cinfo, struct touch_info *pinfo)
 {

	int i = 0;

	char data[40] = {0};

    u16 high_byte,low_byte;
	u8 report_rate =0;
	u8 gestureCode = 0;

	p_point_num = point_num;
    memcpy(pinfo, cinfo, sizeof(struct touch_info));
    memset(cinfo, 0, sizeof(struct touch_info));

	mutex_lock(&i2c_access);
	if (tpd_halt)
	{
		mutex_unlock(&i2c_access);
		TPD_DMESG( "tpd_touchinfo return ..\n");
		return false;
	}
	i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 8, &(data[0]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0x08, 8, &(data[8]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0x10, 8, &(data[16]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0x18, 8, &(data[24]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0xa6, 1, &(data[32]));
	i2c_smbus_read_i2c_block_data(i2c_client, 0x88, 1, &report_rate);
	//TPD_DEBUG("received raw data from touch panel as following:\n");
	//TPD_DEBUG("[data[0]=%x,data[1]= %x ,data[2]=%x ,data[3]=%x ,data[4]=%x ,data[5]=%x]\n",data[0],data[1],data[2],data[3],data[4],data[5]);
	//TPD_DEBUG("[data[9]=%x,data[10]= %x ,data[11]=%x ,data[12]=%x]\n",data[9],data[10],data[11],data[12]);
	//TPD_DEBUG("[data[15]=%x,data[16]= %x ,data[17]=%x ,data[18]=%x]\n",data[15],data[16],data[17],data[18]);

	#ifdef LENOVO_CTP_AREA_LOCK
	//if (get_tpd_area_status())
	{
		gestureCode = data[1];
		if (gestureCode == 0x80)
		{
			ft5x0x_write_reg(0xb9, 0x00);//close the area gesture
			//msleep(10);
			input_report_key(tpd->dev, KEY_POWER, 1);
			input_sync(tpd->dev);
			input_report_key(tpd->dev, KEY_POWER, 0);
			input_sync(tpd->dev);
		}
	}
	#endif
    //    
	 //we have  to re update report rate
    // TPD_DMESG("report rate =%x\n",report_rate);
/* Lenovo-sw yexm1 add for detect charger plug-in/out begin*/
/*
	if((upmu_is_chr_det() == KAL_TRUE) && (tpd_first_config_det_chg== KAL_FALSE))
	{
		ft5x0x_write_reg(0x8b, 0x01);
		tpd_first_config_det_chg = KAL_TRUE;
	}
	else if((upmu_is_chr_det() == KAL_FALSE) && (tpd_first_config_det_chg== KAL_TRUE))
	{
		ft5x0x_write_reg(0x8b, 0x00);
		tpd_first_config_det_chg = KAL_FALSE;
	}
*/
/* Lenovo-sw yexm1 add for detect charger plug-in/out out */

	 if(report_rate < 8)
	 {
	   report_rate = 0x8;
	   if((i2c_smbus_write_i2c_block_data(i2c_client, 0x88, 1, &report_rate))< 0)
	   {
		   TPD_DMESG("I2C read report rate error, line: %d\n", __LINE__);
	   }
	 }
	 
	mutex_unlock(&i2c_access);
	/* Device Mode[2:0] == 0 :Normal operating Mode*/
	if((data[0] & 0x70) != 0) return false; 

	/*get the number of the touch points*/
	point_num= data[2] & 0x0f;
	
	//TPD_DEBUG("point_num =%d\n",point_num);
	
//	if(point_num == 0) return false;

	   //TPD_DEBUG("Procss raw data...\n");

		
		for(i = 0; i < point_num; i++)
		{
			cinfo->p[i] = data[3+6*i] >> 6; //event flag 
                   cinfo->finger_id[i] = data[3+6*i+2]>>4; //touch id
	       /*get the X coordinate, 2 bytes*/
			high_byte = data[3+6*i];
			high_byte <<= 8;
			high_byte &= 0x0f00;
			low_byte = data[3+6*i + 1];
			cinfo->x[i] = high_byte |low_byte;

				//cinfo->x[i] =  cinfo->x[i] * 480 >> 11; //calibra
		
			/*get the Y coordinate, 2 bytes*/
			
			high_byte = data[3+6*i+2];
			high_byte <<= 8;
			high_byte &= 0x0f00;
			low_byte = data[3+6*i+3];
			cinfo->y[i] = high_byte |low_byte;
			#if defined(LENOVO_AREA_TOUCH)//lenovo jixu add
			cinfo->size[i] = data[8+6*i];
			#endif
			  //cinfo->y[i]=  cinfo->y[i] * 800 >> 11;
		
			cinfo->count++;
			
		}
		//TPD_DEBUG(" cinfo->x[0] = %d, cinfo->y[0] = %d, cinfo->p[0] = %d\n", cinfo->x[0], cinfo->y[0], cinfo->p[0]);	
		//TPD_DEBUG(" cinfo->x[1] = %d, cinfo->y[1] = %d, cinfo->p[1] = %d\n", cinfo->x[1], cinfo->y[1], cinfo->p[1]);		
		//TPD_DEBUG(" cinfo->x[2]= %d, cinfo->y[2]= %d, cinfo->p[2] = %d\n", cinfo->x[2], cinfo->y[2], cinfo->p[2]);	
		  
	 return true;

 };
/* Lenovo-sw yexm1 add for detect charger plug-in/out begin*/
int set_tp_protect(bool flag)
{
 if(!tpd_load_status)
  {return;}
else
  {
	if(flag)
	{
		usb_cable_flag = 1;
		ft5x0x_write_reg(0x8b, 0x01);
	}
	else
	{
		usb_cable_flag = 0;
		ft5x0x_write_reg(0x8b, 0x00);
	}
  }
}
EXPORT_SYMBOL(set_tp_protect);
/* Lenovo-sw yexm1 add for detect charger plug-in/out end*/

 static int touch_event_handler(void *unused)
 {
  
    struct touch_info cinfo, pinfo;

	 struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };
	 sched_setscheduler(current, SCHED_RR, &param);
 
	 do
	 {
	  //mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 
        set_current_state(TASK_INTERRUPTIBLE); 
	  	/*lenovo -xm zhangjiano add begin in supend avoid int wake up 11*/
        while (tpd_halt)
		{
			tpd_flag = 0; 
			msleep(20);
		}
		/*end*/
        wait_event_interruptible(waiter,tpd_flag!=0);

        tpd_flag = 0;

        set_current_state(TASK_RUNNING);

        if (tpd_touchinfo(&cinfo, &pinfo)) {
            TPD_DEBUG("point_num = %d\n",point_num);
            if(point_num > 0) {
				#if defined(LENOVO_AREA_TOUCH)//lenovo jixu modify
					tpd_down(cinfo.x[0], cinfo.y[0], 1,cinfo.finger_id[0], cinfo.size[0], cinfo.size[0]);
				#else
                tpd_down(cinfo.x[0], cinfo.y[0], 1,cinfo.finger_id[0]);
				#endif

				#ifdef TPD_HAVE_BUTTON
                if (((boot_mode == META_BOOT)||(boot_mode == RECOVERY_BOOT)||(boot_mode == FACTORY_BOOT)) && (point_num == 1)) {
					tpd_button(cinfo.x[0], cinfo.y[0], 1);	
                }
				#endif
                
                if(point_num > 1) {
					#if defined(LENOVO_AREA_TOUCH)//lenovo jixu modify
					tpd_down(cinfo.x[1], cinfo.y[1], 2,cinfo.finger_id[1], 0, 0);
					#else
	                tpd_down(cinfo.x[1], cinfo.y[1], 2,cinfo.finger_id[1]);
					#endif
                    if(point_num >2) 
						#if defined(LENOVO_AREA_TOUCH)//lenovo jixu modify
						tpd_down(cinfo.x[2], cinfo.y[2], 3,cinfo.finger_id[2], 0, 0);
						#else
                        tpd_down(cinfo.x[2], cinfo.y[2], 3,cinfo.finger_id[2]);
						#endif
            			if(point_num >3) 
						#if defined(LENOVO_AREA_TOUCH)//lenovo jixu modify
						tpd_down(cinfo.x[3], cinfo.y[3], 4,cinfo.finger_id[3], 0, 0);
						#else
                        tpd_down(cinfo.x[3], cinfo.y[3], 4,cinfo.finger_id[3]);	
						#endif
    	              	 if(point_num >4) 
						 	#if defined(LENOVO_AREA_TOUCH)//lenovo jixu modify
							tpd_down(cinfo.x[4], cinfo.y[4], 5,cinfo.finger_id[4], 0, 0);
							#else
							tpd_down(cinfo.x[4], cinfo.y[4], 5,cinfo.finger_id[4]);
							#endif
                }

            } else  {
                tpd_up(cinfo.x[0], cinfo.y[0], 0);

				#ifdef TPD_HAVE_BUTTON
				if (((boot_mode == META_BOOT)||(boot_mode == RECOVERY_BOOT)||(boot_mode == FACTORY_BOOT)) && (p_point_num == 1)) {
					tpd_button(pinfo.x[0], pinfo.y[0], 0);
				}
				#endif
				
            }
            input_sync(tpd->dev);
        }

 }while(!kthread_should_stop());
 
	 return 0;
 }
 
 static int tpd_detect (struct i2c_client *client, struct i2c_board_info *info) 
 {
	 strcpy(info->type, TPD_DEVICE);	
	  return 0;
 }
 
 static void tpd_eint_interrupt_handler(void)
 {
	 TPD_DEBUG("TPD interrupt has been triggered\n");
/*Begin lenovo-sw, xuwen1, 20131121 add for slide touch begin*/
#if defined(LENOVO_CTP_SLIDE_WAKEUP) 
	if(suspend_flag == 1)
	{
		//suspend_flag = 0;		
		schedule_work(&tpd_slide_work);
	}
#endif
/*End lenovo-sw, xuwen1, 20131121, add for slide touch end*/	
	 if(discard_resume_first_eint)
	 {
		discard_resume_first_eint = KAL_FALSE;
		return;
	 }
	 tpd_flag = 1;
	 wake_up_interruptible(&waiter);
	 
 }
 static int __devinit tpd_probe(struct i2c_client *client, const struct i2c_device_id *id)
 {	 
	int retval = TPD_OK;
#ifdef ESD_CHECK	
	int ret;
#endif
	char data;
	u8 report_rate=0;
	int err=0;
	int reset_count = 0;
     // i2c_client = client;//by xuwen1
reset_proc:   
	i2c_client = client;//by xuwen1

   
		//power on, need confirm with SA
#ifdef TPD_POWER_SOURCE_CUSTOM
	hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");
#else
	hwPowerOn(MT65XX_POWER_LDO_VGP2, VOL_2800, "TP");
#endif
#ifdef TPD_POWER_SOURCE_1800
	hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");
#endif 


#ifdef TPD_CLOSE_POWER_IN_SLEEP	 
	hwPowerDown(TPD_POWER_SOURCE,"TP");
	hwPowerOn(TPD_POWER_SOURCE,VOL_3300,"TP");
	msleep(100);
#else
	
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
	msleep(10);  
	TPD_DMESG(" ft5306 reset\n");
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
	msleep(100);
#endif

	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
    mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
    mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
    mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);
 /*lenovo-sw xuwen1 modify 20131023 for match platform begin*/
	  //mt_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM,MT_EDGE_SENSITIVE);
	 // mt_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
	//  mt_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, tpd_eint_interrupt_handler, 1); 
	//  mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
	mt_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_TYPE, tpd_eint_interrupt_handler, 1); 
 	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);  
  /*lenovo-sw xuwen1 modify 20131023 for match platform end*/

	msleep(100);
  /*lenovo-sw,xuwen1, 20131121, add for slide touch begin */
#if defined(LENOVO_CTP_SLIDE_WAKEUP) 
	input_set_capability(tpd->dev, EV_KEY, KEY_POWER);
	input_set_capability(tpd->dev, EV_KEY, KEY_SLIDE);
#endif
/* lenovo-sw,xuwen1, 20131121, add for slide touch end*/
#ifdef CONFIG_SUPPORT_FTS_CTP_UPG
	CTPI2CDMABuf_va = (u8 *)dma_alloc_coherent(NULL, 4096, &CTPI2CDMABuf_pa, GFP_KERNEL);
    	if(!CTPI2CDMABuf_va)
	{
    		printk("[TSP] dma_alloc_coherent error\n");
	}
#endif		
	if((i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 1, &data))< 0)
	{
		TPD_DMESG("I2C transfer error, line: %d\n", __LINE__);
#ifdef TPD_RESET_ISSUE_WORKAROUND
        if ( reset_count < TPD_MAX_RESET_COUNT )
        {
            reset_count++;
            goto reset_proc;
        }
#endif
		   return -1; 
	}

	//set report rate 80Hz
	report_rate = 0x8; 
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x88, 1, &report_rate))< 0)
	{
	    if((i2c_smbus_write_i2c_block_data(i2c_client, 0x88, 1, &report_rate))< 0)
	    {
		   TPD_DMESG("I2C read report rate error, line: %d\n", __LINE__);
	    }
		   
	}

	tpd_load_status = 1;
/*lenovo-sw xuwen1 delete 20131023 for not dupicate,begain*/
#if 0  
	#ifdef VELOCITY_CUSTOM_FT5206
	if((err = misc_register(&tpd_misc_device)))
	{
	      printk("mtk_tpd: tpd_misc_device register error number is %d\n",err);
		printk("mtk_tpd: tpd_misc_device register failed\n");
		
	}
	#endif
#endif
/*lenovo-sw xuwen1 delete 20131023 for not dupicate,begain*/
#ifdef CONFIG_SUPPORT_FTS_CTP_UPG
    	printk("[TSP] Step 0:init \n");
	//msleep(100);
	//fts_ctpm_fw_upgrade_with_i_file();

    	printk("[TSP] Step 8:init stop\n");
#endif	

#ifdef FTS_CTL_IIC
	if (ft_rw_iic_drv_init(client) < 0)
		dev_err(&client->dev, "%s:[FTS] create fts control iic driver failed\n",
				__func__);
#endif	
#ifdef SYSFS_DEBUG
	ft5x0x_create_sysfs(client);
#endif
#ifdef ESD_CHECK	
	ctp_read_id_workqueue = create_workqueue("ctp_read_id");
	INIT_DELAYED_WORK(&ctp_read_id_work, ESD_read_id_workqueue);
	ret = queue_delayed_work(ctp_read_id_workqueue, &ctp_read_id_work,400); //schedule a work for the first detection					
    	printk("[TSP] ret =%d\n",ret);
#endif		
	thread = kthread_run(touch_event_handler, 0, TPD_DEVICE);
	 if (IS_ERR(thread))
		 { 
		  retval = PTR_ERR(thread);
		  TPD_DMESG(TPD_DEVICE " failed to create kernel thread: %d\n", retval);
		}
#if defined(LENOVO_AREA_TOUCH)//lenovo jixu add
	 set_bit(ABS_MT_POSITION_X_W, tpd->dev->absbit);
	 set_bit(ABS_MT_POSITION_Y_W, tpd->dev->absbit);
#endif

	TPD_DMESG("ft5206 Touch Panel Device Probe %s\n", (retval < TPD_OK) ? "FAIL" : "PASS");
   return 0;
   
 }

 static int __devexit tpd_remove(struct i2c_client *client)
 
 {
   
	 TPD_DEBUG("TPD removed\n");
#ifdef CONFIG_SUPPORT_FTS_CTP_UPG
	 
	if(CTPI2CDMABuf_va)
	{
		dma_free_coherent(NULL, 4096, CTPI2CDMABuf_va, CTPI2CDMABuf_pa);
		CTPI2CDMABuf_va = NULL;
		CTPI2CDMABuf_pa = 0;
	}
#endif	
#ifdef ESD_CHECK	
	destroy_workqueue(ctp_read_id_workqueue);
#endif	
	#ifdef SYSFS_DEBUG
	ft5x0x_release_sysfs(client);
	#endif
   return 0;
 }
 
 
 static int tpd_local_init(void)
 {

 
  TPD_DMESG("Focaltech FT5206 I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);

   boot_mode = get_boot_mode();
   // Software reset mode will be treated as normal boot
   if(boot_mode==3) boot_mode = NORMAL_BOOT;

 
   if(i2c_add_driver(&tpd_i2c_driver)!=0)
   	{
  		TPD_DMESG("ft5206 unable to add i2c driver.\n");
      	return -1;
    }
    if(tpd_load_status == 0) 
    {
    	TPD_DMESG("ft5206 add error touch panel driver.\n");
    	i2c_del_driver(&tpd_i2c_driver);
    	return -1;
    }
    get_tpd_info();
#ifdef TPD_HAVE_BUTTON     
    tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
#endif   
  
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))    
    TPD_DO_WARP = 1;
    memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);
    memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);
#endif 

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
    memcpy(tpd_calmat, tpd_def_calmat_local, 8*4);
    memcpy(tpd_def_calmat, tpd_def_calmat_local, 8*4);	
#endif  
		TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);  
		tpd_type_cap = 1;
    return 0; 
 }

 static void tpd_resume( struct early_suspend *h )
 {
  //int retval = TPD_OK;
  //char data;
 
   TPD_DMESG("TPD wake up\n");
#ifdef TPD_CLOSE_POWER_IN_SLEEP	
	hwPowerOn(TPD_POWER_SOURCE,VOL_3300,"TP");

#else
	discard_resume_first_eint = KAL_TRUE;

	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);  
    msleep(1);  
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
#endif
	msleep(200);//add this line 
//lenovo-sw xuwen1 modify 20131023 for match platform
	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 

	#ifdef LENOVO_CTP_AREA_LOCK
	//if (get_tpd_area_status())
	{
		ft5x0x_write_reg(0xb9, 0x01);//open the area gesture
	}	
	#endif
#ifdef ESD_CHECK	
    	msleep(1);  
	queue_delayed_work(ctp_read_id_workqueue, &ctp_read_id_work,400); //schedule a work for the first detection					
#endif
	tpd_halt = 0;
	/* for resume debug
	if((i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 1, &data))< 0)
	{
		TPD_DMESG("resume I2C transfer error, line: %d\n", __LINE__);

	}
	*/
	tpd_up(0,0,0);
	input_sync(tpd->dev);
	/* Lenovo-sw yexm1 add for detect charger plug-in/out begin*/
	set_tp_protect(usb_cable_flag);
	/* Lenovo-sw yexm1 add for detect charger plug-in/out end*/
	TPD_DMESG("TPD wake up done\n");
	 //return retval;
 }

 static void tpd_suspend( struct early_suspend *h )
 {
	// int retval = TPD_OK;
	 static char data = 0x3;
/*Begin lenovo-sw, xuwen1, 20131121, add for slide touch begin*/
	int status = 0;
       int type = 0;
#ifdef LENOVO_CTP_SLIDE_WAKEUP	
	data = 0x4;
#endif

#ifdef DOUBLE_CLICK_TOUCH
	//data = 0x4;
	//printk("[wj]enter double click mode .\n");
#endif

	status = get_tpd_suspend_status();
      printk("[wj]status = 0x%02x.\n", status);
	switch(status)
	{
		case 1:
			data = 0x4;//slide mode
			break;
		case 2:
			//data = 0x4;//double click mode
			break;
		case 3:
			//data = 0x8;//all open
			data = 0x3;
			break;
		default:
			data = 0x3;
	}
	printk("[wj]data = 0x%02x.\n", data);
/*Begin lenovo-sw, xuwen1, 20131121, add for slide touch end*/


   type = getVersion();
   printk("[TPD-xw]:IC TYPE is 0x%2x\n",type);

#ifdef ESD_CHECK	
 	cancel_delayed_work_sync(&ctp_read_id_work);
#endif
 	 tpd_halt = 1;
	 TPD_DMESG("TPD enter sleep\n");
//lenovo-sw xuwen1 modify 20131023 for match platform 
/*lenovo-sw, xuwen1, 20131121, add for slide touch begin*/
#ifdef LENOVO_CTP_SLIDE_WAKEUP
       if((type>=0x40)&&(type!=0xa6))
	{ suspend_flag = 1;}
       else
	 {;} 	
#else	 
	 mt_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
#endif
/*lenovo-sw, xuwen1, 20131121, add for slide touch end*/

	 mutex_lock(&i2c_access);
#ifdef TPD_CLOSE_POWER_IN_SLEEP	
	hwPowerDown(TPD_POWER_SOURCE,"TP");
#else
i2c_smbus_write_i2c_block_data(i2c_client, 0xA5, 1, &data);  //TP enter sleep mode

#endif
	mutex_unlock(&i2c_access);
        TPD_DMESG("TPD enter sleep done\n");
	 //return retval;
 } 


 static struct tpd_driver_t tpd_device_driver = {
		 .tpd_device_name = "FT5206",
		 .tpd_local_init = tpd_local_init,
		 .suspend = tpd_suspend,
		 .resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
		 .tpd_have_button = 1,
#else
		 .tpd_have_button = 0,
#endif		
 };
 /* called when loaded into kernel */
 static int __init tpd_driver_init(void) {
	 printk("MediaTek FT5206 touch panel driver init\n");
	   i2c_register_board_info(0, &ft5206_i2c_tpd, 1);
		 if(tpd_driver_add(&tpd_device_driver) < 0)
			 TPD_DMESG("add FT5206 driver failed\n");
	 return 0;
 }
 
 /* should never be called */
 static void __exit tpd_driver_exit(void) {
	 TPD_DMESG("MediaTek FT5206 touch panel driver exit\n");
	 //input_unregister_device(tpd->dev);
	 tpd_driver_remove(&tpd_device_driver);
 }
 
 module_init(tpd_driver_init);
 module_exit(tpd_driver_exit);


