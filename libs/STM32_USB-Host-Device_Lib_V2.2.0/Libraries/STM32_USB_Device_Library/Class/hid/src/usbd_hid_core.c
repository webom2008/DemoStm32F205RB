/**
  ******************************************************************************
  * @file    usbd_hid_core.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-July-2011
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                HID Class  Description
  *          =================================================================== 
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse protocol
  *             - Usage Page : Generic Desktop
  *             - Usage : Joystick)
  *             - Collection : Application 
  *      
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *           
  *      
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"
#include <string.h>
#include "test_keyboard.h"
#include "test_mtouch.h"

#if (CFG_ON == CFG_USB_DBG_INFO)
#include <stdio.h>
#define DBG(fmt, arg...)        printf("\r\n[USB]usbd_hid_core.c:"fmt, ##arg)
#else
#define DBG(fmt, arg...)        do{}while(0)
#endif

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_HID 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup USBD_HID_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_HID_Private_Defines
  * @{
  */ 

/**
  * @}
  */ 


/** @defgroup USBD_HID_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 

#define USB_TOUCH_BUSY      1
#define USB_TOUCH_IDLE      0


#define INTF_NUM0_FOR_MTOUCH                0
#define INTF_NUM1_FOR_KEYBOARD              1
#define INTF_NUM2_FOR_EMR_PEN               2
#define INTF_NUM0_DESC_TYPE_OFFSET          18
#define INTF_NUM1_DESC_TYPE_OFFSET          50
#define INTF_NUM2_DESC_TYPE_OFFSET          82



/** @defgroup USBD_HID_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_HID_Init (void  *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_HID_DeInit (void  *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_HID_Setup (void  *pdev, 
                                USB_SETUP_REQ *req);

static uint8_t  *USBD_HID_GetCfgDesc (uint8_t speed, uint16_t *length);

static uint8_t  USBD_HID_DataIn (void  *pdev, uint8_t epnum);

static uint8_t  USBD_HID_EP0_TxSent (void  *pdev);

static uint8_t  USBD_HID_EP0_RxReady (void  *pdev);

static uint8_t  USBD_HID_DataOut (void *pdev, uint8_t epnum);

static uint8_t  USBD_HID_SOF (void *pdev);

/**
  * @}
  */ 

/** @defgroup USBD_HID_Private_Variables
  * @{
  */ 

USBD_Class_cb_TypeDef  USBD_HID_cb = 
{
  USBD_HID_Init,
  USBD_HID_DeInit,
  USBD_HID_Setup,
  USBD_HID_EP0_TxSent, /*EP0_TxSent*/  
  USBD_HID_EP0_RxReady, /*EP0_RxReady*/
  USBD_HID_DataIn,  /*DataIn*/
  USBD_HID_DataOut, /*DataOut*/
  USBD_HID_SOF, /*SOF */
  NULL,
  NULL,      
  USBD_HID_GetCfgDesc,
#ifdef USB_OTG_HS_CORE  
  USBD_HID_GetCfgDesc, /* use same config as per FS */
#endif  
};

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */        
__ALIGN_BEGIN static uint32_t  USBD_HID_AltSet  __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */      
__ALIGN_BEGIN static uint32_t  USBD_HID_Protocol  __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  
__ALIGN_BEGIN static uint32_t  USBD_HID_IdleState __ALIGN_END = 0;


#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  
/* 键盘报告描述符定义了如下:
 * INPUT: 8字节(1字节功能键+1字节保留+6字节按键数组)
 * OUTPUT: 尚未使用
 */
__ALIGN_BEGIN static uint8_t Keyboard_ReportDescriptor[KEYBOARD_SIZE_REPORT_DESC] __ALIGN_END =
{
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x06, // USAGE (Keyboard)
    0xa1, 0x01, // COLLECTION (Application)
    //0x85, 0x07, // REPORT_ID (7),因为单个设备可以省掉ID
    0x05, 0x07, // USAGE_PAGE (Keyboard)

    /*使用1个字节[8bit]代表8个功能键是否按下,每个bit独立,即为组合按键
     * b0 = e0 [LeftControl]
     * b1 = e1
     * b2 = e2
     * ...
     * b7 = e7 [Right GUI]
     */
    0x19, 0xe0, // USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7, // USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00, // LOGICAL_MINIMUM (0)
    0x25, 0x01, // LOGICAL_MAXIMUM (1)
    0x75, 0x01, // REPORT_SIZE (1)
    0x95, 0x08, // REPORT_COUNT (8)
    0x81, 0x02, // INPUT (Data,Var,Abs)

    /*保留字节(1字节),OEM使用
     */
    0x95, 0x01, // REPORT_COUNT (1)
    0x75, 0x08, // REPORT_SIZE (8)
    0x81, 0x03, // INPUT (Cnst,Var,Abs)
     
    /*6个字节数据,每个字节(8bit)表示1个键值
     */
    0x95, 0x06, // REPORT_COUNT (6)
    0x75, 0x08, // REPORT_SIZE (8)
    0x15, 0x00, // LOGICAL_MINIMUM (0)
    0x25, 0xFF, // LOGICAL_MAXIMUM (255)
    0x05, 0x07, // USAGE_PAGE (Keyboard)
    0x19, 0x00, // USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65, // USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00, // INPUT (Data,Ary,Abs)
     
    0xc0, // END_COLLECTION
};


#if (CFG_ON == CFG_EMR_PEN_USED)
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  
__ALIGN_BEGIN static uint8_t   EMRPenReportDescriptor[] __ALIGN_END =
{
#if 0
/* SZ-HDX Digitizer-pen */
0x05,0x0d,
0x09,0x02,
0xa1,0x01,
0x85,REPORT_ID_EMR_ONE_PEN,
0x09,0x20,
0xa1,0x00,
0x09,0x42,
0x09,0x44,
0x09,0x45,
0x09,0x3c,
0x09,0x32,
0x15,0x00,
0x25,0x01,
0x75,0x01,
0x95,0x05,
0x81,0x02,
0x95,0x03,
0x81,0x03,
0x05,0x01,
0x09,0x30,
0x75,0x10,
0x95,0x01,
0xa4,
0x55,0x0d,
0x65,0x11,
0x15,0x00,
0x26,0xff,0x7f,
0x35,0x00,
0x46,0xff,0x7f,
0x81,0x02,
0x09,0x31,
0x15,0x00,
0x26,0x00,0x48,
0x35,0x00,
0x46,0x00,0x48,
0x81,0x02,
0x05,0x0d,
0x09,0x30,
0x15,0x00,
0x26,0xff,0x03,
0x35,0x00,
0x46,0xff,0x03,
0x81,0x02,
0xc0,
0xc0,
0x06,0x00,0xff,
0x09,0x01,
0xa1,0x01,
0x85,0x02,
0x09,0x20,
0xa1,0x00,
0x09,0x3f,
0x15,0x00,
0x25,0xff,
0x75,0x08,
0x95,0x07,
0x81,0x02,
0x09,0x40,
0x75,0x08,
0x95,0x03,
0x15,0x00,
0x25,0xff,
0xb1,0x02,
0xc0,
0xc0, 
#else
    /*电磁屏报告描述
     *1.ID=0x10: 
     *          1 byte(5bit用于Tip/Barrel/Eraser/Invert/In Range,其他3bit保留)
     *          2 bytes X坐标
     *          2 bytes Y坐标
     *          2 bytes 压力值
     */
    0x05, 0x0d,                    // USAGE_PAGE (Digitizers)
    0x09, 0x02,                    // USAGE (Pen)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, REPORT_ID_EMR_ONE_PEN,        //   REPORT_ID (16)
    0x09, 0x20,                    //   USAGE (Stylus)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x09, 0x42,                    //     USAGE (Tip Switch)
    0x09, 0x44,                    //     USAGE (Barrel Switch)
    0x09, 0x45,                    //     USAGE (Eraser)
    0x09, 0x3c,                    //     USAGE (Invert)
    0x09, 0x32,                    //     USAGE (In Range)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x95, 0x05,                    //     REPORT_COUNT (5)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x10,                    //     REPORT_SIZE (16)
    0xa4,                          //     PUSH
    0x55, 0x0d,                    //     UNIT_EXPONENT (-3)
    0x65, 0x11,                    //     UNIT (SI Lin:Distance)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x7f,              //     LOGICAL_MAXIMUM (32767)
    0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)
    0x46, 0xff, 0x7f,              //     PHYSICAL_MAXIMUM (32767)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x09, 0x31,                    //     USAGE (Y)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0x00, 0x48,              //     LOGICAL_MAXIMUM (18432)
    0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)
    0x46, 0x00, 0x48,              //     PHYSICAL_MAXIMUM (18432)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x05, 0x0d,                    //     USAGE_PAGE (Digitizers)
    0x09, 0x30,                    //     USAGE (Tip Pressure)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x03,              //     LOGICAL_MAXIMUM (1023)
    0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)
    0x46, 0xff, 0x03,              //     PHYSICAL_MAXIMUM (1023)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
#endif
};
#endif /* #if (CFG_ON == CFG_EMR_PEN_USED) */


#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */ 
/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgDesc[CUSTOMHID_SIZ_CONFIG_DESC] __ALIGN_END =
{
    /* 配置描述符 */
    /* Offset = 0 */
    0x09,                              // Length
    USB_CONFIGURATION_DESCRIPTOR_TYPE, // Type
    CUSTOMHID_SIZ_CONFIG_DESC&0xFF,
    (CUSTOMHID_SIZ_CONFIG_DESC>>8)&0xFF,// Totallength (= 9+9+9+7+7)
#if (CFG_ON == CFG_EMR_PEN_USED)
    0x03,                              // NumInterfaces ,3个接口
#else
    0x02,                              // NumInterfaces ,两个接口
#endif
    0x01,                              // bConfigurationValue
    0x00,                              // iConfiguration
    0xA0,                              // bmAttributes (Modified by Daniel 20100511 -- Self-Powered, Remote-Wakeup D7 bit of this field should be always 1)
    0x96,

    //<!-- ===================复合设备接口 START =========================== -->
    /* 接口描述符 */
    /* Offset = 9 */
    0x09,                              // bLength
    USB_INTERFACE_DESCRIPTOR_TYPE,     // bDescriptorType
    INTF_NUM0_FOR_MTOUCH,              // bInterfaceNumber
    0x00,                              // bAlternateSetting
    0x02,                              // bNumEndpoints,两个端点[1个输入,1个输出]
    0x03,                              // bInterfaceClass (3 = HID)
    0x00,                              // bInterfaceSubClass
    0x00,                              // bInterfaceProcotol
    0x00,                              // iInterface

    /* HID描述符 */
    /* Offset = 18 INTF_NUM0_DESC_TYPE_OFFSET */
    0x09,                                 // bLength
    HID_DESCRIPTOR_TYPE,                  // bDescriptorType
    0x11,0x01,                         // bcdHID
    0x00,                                 // bCountryCode
    0x01,                                 // bNumDescriptors
    0x22,                               // bDescriptorType
    CUSTOMHID_SIZ_HID_REPORT_DESC_WIN7&0xFF,// wItemLength (len. of report descriptor)
    (CUSTOMHID_SIZ_HID_REPORT_DESC_WIN7>>8)&0xFF,

    /* 输入端点 */
    /* Offset = 27 */
    0x07,                              // bLength
    USB_ENDPOINT_DESCRIPTOR_TYPE,      // bDescriptorType
    HID_TOUCH_IN_EP,                   // bEndpointAddress
    0x03,                              // bmAttributes
    HID_TOUCH_IN_PACKET&0xFF,
    (HID_TOUCH_IN_PACKET>>8)&0xFF,     // MaxPacketSize (LITTLE ENDIAN)
    1,                                 // bInterval,1ms

    /* 输出端点 */
    /* Offset = 34 */
    0x07,                              // bLength
    USB_ENDPOINT_DESCRIPTOR_TYPE,      // bDescriptorType
    HID_TOUCH_OUT_EP,                              // bEndpointAddress
    0x03,                              // bmAttributes
    HID_TOUCH_OUT_PACKET&0xFF,
    (HID_TOUCH_OUT_PACKET>>8)&0xFF,    // MaxPacketSize (LITTLE ENDIAN)
    1,                                 // bInterval,1ms
    //<!-- ===================复合设备接口 END =========================== -->

    //<!-- ===================键盘设备接口 START =========================== -->
    /* Offset = 41 */
    0x09,          /*bLength: Interface Descriptor size*/
    USB_INTERFACE_DESCRIPTOR_TYPE,/*bDescriptorType: Interface descriptor type*/
    INTF_NUM1_FOR_KEYBOARD,       /*bInterfaceNumber: Number of Interface*/
    0x00,          /*bAlternateSetting: Alternate setting*/
    0x02,          /*bNumEndpoints,两个端点[1个输入,1个输出]*/
    0x03,          /*bInterfaceClass: HID*/
    0x00,          /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x01,          /*bInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0,         /*iInterface: Index of string descriptor*/

    /* Offset = 50 INTF_NUM1_DESC_TYPE_OFFSET */
    0x09,          /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x00,          /*bcdHID: HID Class Spec release number*/
    0x01,
    0x00,          /*bCountryCode: Hardware target country*/
    0x01,          /*bNumDescriptors: Number of HID class descriptors to follow*/
    0x22,          /*bDescriptorType*/
    KEYBOARD_SIZE_REPORT_DESC&0xFF,/*wItemLength: Total length of Report descriptor*/
    (KEYBOARD_SIZE_REPORT_DESC>>8)&0xFF,

    /* Offset = 59 */
    0x07,           /*bLength: Endpoint Descriptor size*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/
    HID_KEYBOARD_IN_EP,           /*bEndpointAddress: Endpoint Address (IN)*/
    0x03,           /*bmAttributes: Interrupt endpoint*/
    HID_KEYBOARD_IN_PACKET&0xFF,           /*wMaxPacketSize: 8 Byte max */
    (HID_KEYBOARD_IN_PACKET>>8)&0xFF,
    0x01,           /*bInterval: Polling Interval (32 ms)*/

    /* Offset = 66 */
    0x07,           /*bLength: Endpoint Descriptor size*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/
    HID_KEYBOARD_OUT_EP,           /*bEndpointAddress: Endpoint Address (OUT)*/
    0x03,           /*bmAttributes: Interrupt endpoint*/
    HID_KEYBOARD_OUT_PACKET&0xFF,           /*wMaxPacketSize: 8 Byte max */
    (HID_KEYBOARD_OUT_PACKET>>8)&0xFF,
    0x01,            /*bInterval: Polling Interval (1 ms)*/
    //<!-- ===================键盘设备接口 END =========================== -->
    
#if (CFG_ON == CFG_EMR_PEN_USED)
    //<!-- ===================电磁笔设备接口 START =========================== -->
    /* Offset = 73 */
    0x09,          /*bLength: Interface Descriptor size*/
    USB_INTERFACE_DESCRIPTOR_TYPE,/*bDescriptorType: Interface descriptor type*/
    INTF_NUM2_FOR_EMR_PEN,        /*bInterfaceNumber: Number of Interface*/
    0x00,          /*bAlternateSetting: Alternate setting*/
    0x01,          /*bNumEndpoints,1个INPUT*/
    0x03,          /*bInterfaceClass: HID*/
    0x00,          /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x02,          /*bInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0,         /*iInterface: Index of string descriptor*/

    /* Offset = 82 INTF_NUM2_DESC_TYPE_OFFSET */
    0x09,          /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x10,          /*bcdHID: HID Class Spec release number*/
    0x01,
    0x00,          /*bCountryCode: Hardware target country*/
    0x01,          /*bNumDescriptors: Number of HID class descriptors to follow*/
    0x22,          /*bDescriptorType*/
    sizeof(EMRPenReportDescriptor)&0xFF,/*wItemLength: Total length of Report descriptor*/
    (sizeof(EMRPenReportDescriptor)>>8)&0xFF,

    /* Offset = 91 */
    0x07,           /*bLength: Endpoint Descriptor size*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/
    HID_EMR_PEN_IN_EP,           /*bEndpointAddress: Endpoint Address (IN)*/
    0x03,           /*bmAttributes: Interrupt endpoint*/
    HID_EMR_PEN_IN_PACKET&0xFF,           /*wMaxPacketSize:  Byte max */
    (HID_EMR_PEN_IN_PACKET>>8)&0xFF,
    0x01,           /*bInterval: Polling Interval (1 ms)*/
    /* Total = 98 */
#endif /* #if (CFG_ON == CFG_EMR_PEN_USED) */
}; /* CustomHID_ConfigDescriptor */

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  
__ALIGN_BEGIN static uint8_t CustomHID_ReportDescriptor[CUSTOMHID_SIZ_HID_REPORT_DESC_WIN7] __ALIGN_END =
{ 
    /*对管测试用途页
     *1.ID=0xfd: 6 bytes INPUT
     *2.ID=0xfe: 4 bytes FEATURE
     */
    0x06, 0x00, 0xff,              // USAGE_PAGE (Undefined)
    0x09, 0x00,                    // USAGE (Undefined)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, REPORT_ID_IR_PAIRS_INPUT,//   REPORT_ID (253)
    0x06, 0x00, 0xff,              //   USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    // USAGE (Undefined)
    0x09, 0x02,                    // USAGE (Undefined)
    0x09, 0x03,                    // USAGE (Undefined)
    0x09, 0x04,                    // USAGE (Undefined)
    0x09, 0x05,                    // USAGE (Undefined)
    0x09, 0x06,                    // USAGE (Undefined)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)

    0x85, REPORT_ID_IR_PAIRS_FEATURE,//   REPORT_ID (254)
    0x06, 0x00, 0xff,              //   USAGE_PAGE (Generic Desktop)
    0x09, 0x01,                    // USAGE (Undefined)
    0x09, 0x02,                    // USAGE (Undefined)
    0x09, 0x03,                    // USAGE (Undefined)
    0x09, 0x04,                    // USAGE (Undefined)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, 0x04,                    //   REPORT_COUNT (4)
    0xb1, 0x02,                    //   FEATURE (Data,Var,Abs)
    0xc0,
    
    /*XP单点用途页
     *1.ID=0x01: 
     *          1 byte(3bit用于左键/右键/中键值,其他5bit保留)
     *          4 bytes(X坐标,Y坐标,范围0 to 4095)
     *          1 byte(Digitizers,范围0 to 255)
     *          1 byte(滚轮,范围-127 to +127)
     */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)

    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x85, REPORT_ID_MOUSE,         //     REPORT_ID (1)

    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)

    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x7f,              //     LOGICAL_MAXIMUM (4095)
    0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)
    0x46, 0xff, 0x7f,              //     PHYSICAL_MAXIMUM (4095)
    0x75, 0x10,                    //     REPORT_SIZE (16)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x05, 0x0D,                    //        USAGE_PAGE (Digitizers)
#if LINUX == 1
    0x09, 0x00,                    //     USAGE (Undefined)
#else
    0x09, 0x33,                    //     USAGE (Touch)     // for Android
#endif
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //     LOGICAL_MAXIMUM (255)
    0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)
    0x46, 0xff, 0x00,              //     PHYSICAL_MAXIMUM (255)   
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs) 
    0x05, 0x01,            //        USAGE_PAGE (Generic Desktop)
    0x09, 0x38,                    //     USAGE (Wheel)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x35, 0x81,                    //     PHYSICAL_MINIMUM (-127)
    0x45, 0x7f,                    //     PHYSICAL_MAXIMUM (127)   
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)    
    0xc0,                          //   END_COLLECTION(Physical)
    0xc0,                           // END_COLLECTION(Application)

    /*服务程序用途页
     *1.ID=0xfc:
     *          63 bytes:INPUT(数据,范围0 to 255)
     *          63 bytes:OUTPUT(数据,范围0 to 255)
     *2.ID=0xfb:
     *          63 bytes:INPUT(数据,范围0 to 255)
     *          63 bytes:OUTPUT(数据,范围0 to 255)
     */
    0x06, 0x00, 0xff,              // USAGE_PAGE (Undefined)
    0x09, 0x00,                    // USAGE (Undefined)
    0xa1, 0x01,                    // COLLECTION (Application)    
    0x85, REPORT_ID_FC_SERVICE,    //   REPORT_ID (0xfc)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x19, 0x01,                    //   USAGE MIN(1)
    0x29, FC_BUF_LEN-1,            //   USAGE MAX(63)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, FC_BUF_LEN-1,            //   REPORT_COUNT (63)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x19, 0x01,                    //   USAGE MIN(1)
    0x29, FC_BUF_LEN-1,            //   USAGE MAX(63) 
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)   
    0xc0,                          // END_COLLECTION

    0x06, 0x00, 0xff,              // USAGE_PAGE (Undefined)
    0x09, 0x00,                    // USAGE (Undefined)
    0xa1, 0x01,                    // COLLECTION (Application)    
    0x85, REPORT_ID_FB_SERVICE,    //   REPORT_ID (0xfb)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
    0x19, 0x01,                    //   USAGE MIN(1)
    0x29, FB_BUF_LEN-1,            //   USAGE MAX(63)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x95, FB_BUF_LEN-1,            //   REPORT_COUNT (63)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x19, 0x01,                    //   USAGE MIN(1)
    0x29, FB_BUF_LEN-1,            //   USAGE MAX(63) 
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)   
    0xc0,                          // END_COLLECTION

    /*Win7多点用途页
     *1.ID=0x02:
     *          1 byte (b7~b3:Reserved, 
     *                     b2:Touch Valid,触摸是意外触摸还是确认触摸,
     *                     b1:In Range,触摸的位置是否处于有效, 
     *                     b0:Tip Switch,笔尖开关)
     *          1 byte (ID)
     *          4 bytes (X坐标,y坐标.范围0 to 32767)
     *          4 bytes ([可选]X宽度,y宽度.范围0 to 32767)
     *          10 bytes (第2个点)
     *          10 bytes (第3个点)
     *          10 bytes (第4个点)
     *          10 bytes (第5个点)
     *          10 bytes (第6个点)
     *          1 bytes (有效点的个数,范围0 to 255)
     */
    0x05  ,0x0d,                   // USAGE_PAGE (Digitizers)
    0x09  ,0x04,                   // USAGE (Touch Screen)
    0xa1  ,0x01,                   // COLLECTION (Application)
    0x85  ,REPORT_ID_WIN7_MTOUCH,  //   REPORT_ID (02)  
    0x09  ,0x22,                   //   USAGE (Finger)
    /*第1个手指坐标*/
    0xa1  ,0x02,                   //     COLLECTION (Logical)
    0x09  ,0x42,                   //       USAGE (Tip Switch)
    0x15  ,0x00,                   //       LOGICAL_MINIMUM (0)
    0x25  ,0x01,                   //       LOGICAL_MAXIMUM (1)
    0x75  ,0x01,                   //       REPORT_SIZE (1)
    0x95  ,0x01,                   //       REPORT_COUNT (1)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x32,                   //       USAGE (In Range)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x47,                   //       USAGE (Touch Valid)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x95  ,0x05,                   //       REPORT_COUNT (5)
    0x81  ,0x03,                   //       INPUT (Cnst,Ary,Abs)
    0x75  ,0x08,                   //       REPORT_SIZE (8)
    0x09  ,0x51,                   //       USAGE (Contact Identifier)
    0x95  ,0x01,                   //       REPORT_COUNT (1)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x05  ,0x01,                   //       USAGE_PAGE (Generic Desktop)
    0x15  ,0x00,                   //       LOGICAL_MINIMUM (0)
    0x26  ,0xff,  0x7f,            //       LOGICAL_MAXIMUM (32767)
    0x75  ,0x10,                   //       REPORT_SIZE (16)
#if CHG_PHY_SIZE==1
    0x55  ,0x0e,                   //       UNIT_EXPONENT (0)
    0x65  ,0x11,                   //       UNIT (None)
    0x09  ,0x30,                   //       USAGE (X)
    0x35  ,0x00,                   //       PHYSICAL_MINIMUM (0)
    0x46  ,LOBYTE(PHYSICAL_SIZE_X),  HIBYTE(PHYSICAL_SIZE_X),          //       PHYSICAL_MAXIMUM (0)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x46  ,LOBYTE(PHYSICAL_SIZE_Y),  HIBYTE(PHYSICAL_SIZE_Y),          //       PHYSICAL_MAXIMUM (0)
    0x09  ,0x31,                   //       USAGE (Y)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)    
#else
    0x55  ,0x00,                   //       UNIT_EXPONENT (0)
    0x65  ,0x00,                   //       UNIT (None)
    0x09  ,0x30,                   //       USAGE (X)
    0x35  ,0x00,                   //       PHYSICAL_MINIMUM (0)
    0x46  ,0x00,  0x00,            //       PHYSICAL_MAXIMUM (0)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x31,                   //       USAGE (Y)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)    
#endif
#if WIN7_WIDTH==1
    0x05  ,0x0d,                   //            USAGE PAGE (Digitizers)     
    0x09  ,0x48,                   //            USAGE (Width)   
    0x81  ,0x02,                   //            INPUT (Data,Var,Abs)  
    0x09  ,0x49,                   //            USAGE (Height)                      
    0x81  ,0x02,                   //            INPUT (Data,Var,Abs)    
#endif
    0xc0  ,                        //    END_COLLECTION(Logical)
    /*第2个手指坐标*/
    0xa1  ,0x02,                   //    COLLECTION (Logical)
    0x05  ,0x0d,                   //     USAGE_PAGE (Digitizers)
    0x09  ,0x42,                   //       USAGE (Tip Switch)
    0x15  ,0x00,                   //       LOGICAL_MINIMUM (0)
    0x25  ,0x01,                   //       LOGICAL_MAXIMUM (1)
    0x75  ,0x01,                   //       REPORT_SIZE (1)
    0x95  ,0x01,                   //       REPORT_COUNT (1)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x32,                   //       USAGE (In Range)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x47,                   //       USAGE (Touch Valid)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x95  ,0x05,                   //       REPORT_COUNT (5)
    0x81  ,0x03,                   //       INPUT (Cnst,Ary,Abs)
    0x75  ,0x08,                   //       REPORT_SIZE (8)
    0x09  ,0x51,                   //       USAGE (Contact Identifier)
    0x95  ,0x01,                   //       REPORT_COUNT (1)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x05  ,0x01,                   //       USAGE_PAGE (Generic Desktop)
    0x15  ,0x00,                   //       LOGICAL_MINIMUM (0)
    0x26  ,0xff,  0x7f,            //       LOGICAL_MAXIMUM (32767)
    0x75  ,0x10,                   //       REPORT_SIZE (16)
#if CHG_PHY_SIZE==1
    0x55  ,0x0e,                   //       UNIT_EXPONENT (0)
    0x65  ,0x11,                   //       UNIT (None)
    0x09  ,0x30,                   //       USAGE (X)
    0x35  ,0x00,                   //       PHYSICAL_MINIMUM (0)
    0x46  ,LOBYTE(PHYSICAL_SIZE_X),  HIBYTE(PHYSICAL_SIZE_X),          //       PHYSICAL_MAXIMUM (0)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x46  ,LOBYTE(PHYSICAL_SIZE_Y),  HIBYTE(PHYSICAL_SIZE_Y),          //       PHYSICAL_MAXIMUM (0)
    0x09  ,0x31,                   //       USAGE (Y)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)    
#else
    0x55  ,0x00,                   //       UNIT_EXPONENT (0)
    0x65  ,0x00,                   //       UNIT (None)
    0x09  ,0x30,                   //       USAGE (X)
    0x35  ,0x00,                   //       PHYSICAL_MINIMUM (0)
    0x46  ,0x00,  0x00,            //       PHYSICAL_MAXIMUM (0)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x31,                   //       USAGE (Y)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
#endif
#if WIN7_WIDTH==1
    0x05  ,0x0d,                   //            USAGE PAGE (Digitizers)     
    0x09  ,0x48,                   //            USAGE (Width)   
    0x81  ,0x02,                   //            INPUT (Data,Var,Abs)  
    0x09  ,0x49,                   //            USAGE (Height)                      
    0x81  ,0x02,                   //            INPUT (Data,Var,Abs)  
#endif
    0xc0  ,                        //    END_COLLECTION
    /*第3个手指坐标*/
    0xa1  ,0x02,                   //    COLLECTION (Logical)
    0x05  ,0x0d,                   //     USAGE_PAGE (Digitizers)
    0x09  ,0x42,                   //       USAGE (Tip Switch)
    0x15  ,0x00,                   //       LOGICAL_MINIMUM (0)
    0x25  ,0x01,                   //       LOGICAL_MAXIMUM (1)
    0x75  ,0x01,                   //       REPORT_SIZE (1)
    0x95  ,0x01,                   //       REPORT_COUNT (1)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x32,                   //       USAGE (In Range)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x47,                   //       USAGE (Touch Valid)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x95  ,0x05,                   //       REPORT_COUNT (5)
    0x81  ,0x03,                   //       INPUT (Cnst,Ary,Abs)
    0x75  ,0x08,                   //       REPORT_SIZE (8)
    0x09  ,0x51,                   //       USAGE (Contact Identifier)
    0x95  ,0x01,                   //       REPORT_COUNT (1)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x05  ,0x01,                   //       USAGE_PAGE (Generic Desktop)
    0x15  ,0x00,                   //       LOGICAL_MINIMUM (0)
    0x26  ,0xff,  0x7f,            //       LOGICAL_MAXIMUM (2240)
    0x75  ,0x10,                   //       REPORT_SIZE (16)
#if CHG_PHY_SIZE==1
    0x55  ,0x0e,                   //       UNIT_EXPONENT (0)
    0x65  ,0x11,                   //       UNIT (None)
    0x09  ,0x30,                   //       USAGE (X)
    0x35  ,0x00,                   //       PHYSICAL_MINIMUM (0)
    0x46  ,LOBYTE(PHYSICAL_SIZE_X),  HIBYTE(PHYSICAL_SIZE_X),          //       PHYSICAL_MAXIMUM (0)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x46  ,LOBYTE(PHYSICAL_SIZE_Y),  HIBYTE(PHYSICAL_SIZE_Y),          //       PHYSICAL_MAXIMUM (0)
    0x09  ,0x31,                   //       USAGE (Y)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)    
#else
    0x55  ,0x00,                   //       UNIT_EXPONENT (0)
    0x65  ,0x00,                   //       UNIT (None)
    0x09  ,0x30,                   //       USAGE (X)
    0x35  ,0x00,                   //       PHYSICAL_MINIMUM (0)
    0x46  ,0x00,  0x00,            //       PHYSICAL_MAXIMUM (0)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x31,                   //       USAGE (Y)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
#endif
#if WIN7_WIDTH==1
    0x05  ,0x0d,                   //            USAGE PAGE (Digitizers)     
    0x09  ,0x48,                   //            USAGE (Width)   
    0x81  ,0x02,                   //            INPUT (Data,Var,Abs)  
    0x09  ,0x49,                   //            USAGE (Height)                      
    0x81  ,0x02,                   //            INPUT (Data,Var,Abs)  
#endif
    0xc0  ,                        //    END_COLLECTION
    /*第4个手指坐标*/
    0xa1  ,0x02,                   //    COLLECTION (Logical)
    0x05  ,0x0d,                   //     USAGE_PAGE (Digitizers)
    0x09  ,0x42,                   //       USAGE (Tip Switch)
    0x15  ,0x00,                   //       LOGICAL_MINIMUM (0)
    0x25  ,0x01,                   //       LOGICAL_MAXIMUM (1)
    0x75  ,0x01,                   //       REPORT_SIZE (1)
    0x95  ,0x01,                   //       REPORT_COUNT (1)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x32,                   //       USAGE (In Range)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x47,                   //       USAGE (Touch Valid)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x95  ,0x05,                   //       REPORT_COUNT (5)
    0x81  ,0x03,                   //       INPUT (Cnst,Ary,Abs)
    0x75  ,0x08,                   //       REPORT_SIZE (8)
    0x09  ,0x51,                   //       USAGE (Contact Identifier)
    0x95  ,0x01,                   //       REPORT_COUNT (1)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x05  ,0x01,                   //       USAGE_PAGE (Generic Desktop)
    0x15  ,0x00,                   //       LOGICAL_MINIMUM (0)
    0x26  ,0xff,  0x7f,            //       LOGICAL_MAXIMUM (2240)
    0x75  ,0x10,                   //       REPORT_SIZE (16)
#if CHG_PHY_SIZE==1
    0x55  ,0x0e,                   //       UNIT_EXPONENT (0)
    0x65  ,0x11,                   //       UNIT (None)
    0x09  ,0x30,                   //       USAGE (X)
    0x35  ,0x00,                   //       PHYSICAL_MINIMUM (0)
    0x46  ,LOBYTE(PHYSICAL_SIZE_X),  HIBYTE(PHYSICAL_SIZE_X),          //       PHYSICAL_MAXIMUM (0)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x46  ,LOBYTE(PHYSICAL_SIZE_Y),  HIBYTE(PHYSICAL_SIZE_Y),          //       PHYSICAL_MAXIMUM (0)
    0x09  ,0x31,                   //       USAGE (Y)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)    
#else
    0x55  ,0x00,                   //       UNIT_EXPONENT (0)
    0x65  ,0x00,                   //       UNIT (None)
    0x09  ,0x30,                   //       USAGE (X)
    0x35  ,0x00,                   //       PHYSICAL_MINIMUM (0)
    0x46  ,0x00,  0x00,            //       PHYSICAL_MAXIMUM (0)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x31,                   //       USAGE (Y)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
#endif
#if WIN7_WIDTH==1
    0x05  ,0x0d,                   //            USAGE PAGE (Digitizers)     
    0x09  ,0x48,                   //            USAGE (Width)   
    0x81  ,0x02,                   //            INPUT (Data,Var,Abs)  
    0x09  ,0x49,                   //            USAGE (Height)                      
    0x81  ,0x02,                   //            INPUT (Data,Var,Abs)   
#endif
    0xc0  ,                        //    END_COLLECTION
    /*第5个手指坐标*/
    0xa1  ,0x02,                   //    COLLECTION (Logical)
    0x05  ,0x0d,                   //     USAGE_PAGE (Digitizers)
    0x09  ,0x42,                   //       USAGE (Tip Switch)
    0x15  ,0x00,                   //       LOGICAL_MINIMUM (0)
    0x25  ,0x01,                   //       LOGICAL_MAXIMUM (1)
    0x75  ,0x01,                   //       REPORT_SIZE (1)
    0x95  ,0x01,                   //       REPORT_COUNT (1)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x32,                   //       USAGE (In Range)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x47,                   //       USAGE (Touch Valid)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x95  ,0x05,                   //       REPORT_COUNT (5)
    0x81  ,0x03,                   //       INPUT (Cnst,Ary,Abs)
    0x75  ,0x08,                   //       REPORT_SIZE (8)
    0x09  ,0x51,                   //       USAGE (Contact Identifier)
    0x95  ,0x01,                   //       REPORT_COUNT (1)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x05  ,0x01,                   //       USAGE_PAGE (Generic Desktop)
    0x15  ,0x00,                   //       LOGICAL_MINIMUM (0)
    0x26  ,0xff,  0x7f,            //       LOGICAL_MAXIMUM (2240)
    0x75  ,0x10,                   //       REPORT_SIZE (16)
#if CHG_PHY_SIZE==1
    0x55  ,0x0e,                   //       UNIT_EXPONENT (0)
    0x65  ,0x11,                   //       UNIT (None)
    0x09  ,0x30,                   //       USAGE (X)
    0x35  ,0x00,                   //       PHYSICAL_MINIMUM (0)
    0x46  ,LOBYTE(PHYSICAL_SIZE_X),  HIBYTE(PHYSICAL_SIZE_X),          //       PHYSICAL_MAXIMUM (0)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x46  ,LOBYTE(PHYSICAL_SIZE_Y),  HIBYTE(PHYSICAL_SIZE_Y),          //       PHYSICAL_MAXIMUM (0)
    0x09  ,0x31,                   //       USAGE (Y)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)    
#else
    0x55  ,0x00,                   //       UNIT_EXPONENT (0)
    0x65  ,0x00,                   //       UNIT (None)
    0x09  ,0x30,                   //       USAGE (X)
    0x35  ,0x00,                   //       PHYSICAL_MINIMUM (0)
    0x46  ,0x00,  0x00,            //       PHYSICAL_MAXIMUM (0)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x31,                   //       USAGE (Y)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
#endif
#if WIN7_WIDTH==1
    0x05  ,0x0d,                   //            USAGE PAGE (Digitizers)     
    0x09  ,0x48,                   //            USAGE (Width)   
    0x81  ,0x02,                   //            INPUT (Data,Var,Abs)  
    0x09  ,0x49,                   //            USAGE (Height)                      
    0x81  ,0x02,                   //            INPUT (Data,Var,Abs)     
#endif
    0xc0  ,                        //    END_COLLECTION
    /*第6个手指坐标*/
    0xa1  ,0x02,                   //    COLLECTION (Logical)
    0x05  ,0x0d,                   //     USAGE_PAGE (Digitizers)
    0x09  ,0x42,                   //       USAGE (Tip Switch)
    0x15  ,0x00,                   //       LOGICAL_MINIMUM (0)
    0x25  ,0x01,                   //       LOGICAL_MAXIMUM (1)
    0x75  ,0x01,                   //       REPORT_SIZE (1)
    0x95  ,0x01,                   //       REPORT_COUNT (1)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x32,                   //       USAGE (In Range)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x47,                   //       USAGE (Touch Valid)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x95  ,0x05,                   //       REPORT_COUNT (5)
    0x81  ,0x03,                   //       INPUT (Cnst,Ary,Abs)
    0x75  ,0x08,                   //       REPORT_SIZE (8)
    0x09  ,0x51,                   //       USAGE (Contact Identifier)
    0x95  ,0x01,                   //       REPORT_COUNT (1)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x05  ,0x01,                   //       USAGE_PAGE (Generic Desktop)
    0x15  ,0x00,                   //       LOGICAL_MINIMUM (0)
    0x26  ,0xff,  0x7f,            //       LOGICAL_MAXIMUM (2240)
    0x75  ,0x10,                   //       REPORT_SIZE (16)
#if CHG_PHY_SIZE==1
    0x55  ,0x0e,                   //       UNIT_EXPONENT (0)
    0x65  ,0x11,                   //       UNIT (None)
    0x09  ,0x30,                   //       USAGE (X)
    0x35  ,0x00,                   //       PHYSICAL_MINIMUM (0)
    0x46  ,LOBYTE(PHYSICAL_SIZE_X),  HIBYTE(PHYSICAL_SIZE_X),          //       PHYSICAL_MAXIMUM (1)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x46  ,LOBYTE(PHYSICAL_SIZE_Y),  HIBYTE(PHYSICAL_SIZE_Y),          //       PHYSICAL_MAXIMUM (0)
    0x09  ,0x31,                   //       USAGE (Y)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)    
#else
    0x55  ,0x00,                   //       UNIT_EXPONENT (0)
    0x65  ,0x00,                   //       UNIT (None)
    0x09  ,0x30,                   //       USAGE (X)
    0x35  ,0x00,                   //       PHYSICAL_MINIMUM (0)
    0x46  ,0x00,  0x00,            //       PHYSICAL_MAXIMUM (0)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
    0x09  ,0x31,                   //       USAGE (Y)
    0x81  ,0x02,                   //       INPUT (Data,Var,Abs)
#endif
#if WIN7_WIDTH==1
    0x05  ,0x0d,                   //            USAGE PAGE (Digitizers)     
    0x09  ,0x48,                   //            USAGE (Width)   
    0x81  ,0x02,                   //            INPUT (Data,Var,Abs)  
    0x09  ,0x49,                   //            USAGE (Height)                      
    0x81  ,0x02,                   //            INPUT (Data,Var,Abs)
#endif
    0xc0  ,                        //    END_COLLECTION

    /*1字节,有效点个数*/
    0x05  ,0x0d,                   //    USAGE_PAGE (Digitizers)
    0x09  ,0x54,                   //    USAGE (Contact count)
    0x15  ,0x00,                   //    LOGICAL_MINIMUM (0)
    0x26  ,0xff,  0x00,            //    LOGICAL_MAXIMUM (255)
    0x95  ,0x01,                   //    REPORT_COUNT (1)
    0x75  ,0x08,                   //    REPORT_SIZE (8)
    0x81  ,0x02,                   //    INPUT (Data,Var,Abs)
    
    /*获取设备最多支持多少点触摸(只读属性)
     *1.ID=0x03:
     *          1 byte (设备最多支持触摸点数,范围0~128)
     */
    0x85  ,REPORT_ID_CONTACT_CNT_MAX_FEATURE,//    REPORT_ID (03) 
    0x09  ,0x55,                   //    USAGE (Contact count maximum)
    0x15  ,0x00,                   //    LOGICAL_MINIMUM (0)
    0x25  ,0x80,                   //    LOGICAL_MAXIMUM (128)
    0xb1  ,0x02,                   //    FEATURE (Data,Var,Abs)
    0xc0  ,                        // END_COLLECTION
    
    /*获取设备模式和ID(只读属性)
     *1.ID=0x04:
     *          1 byte (设备工作模式,范围0~10)
     *          1 byte (设备ID,范围0~10)
     */
    0x09  ,0x0e,                   // USAGE (Device Configuration)
    0xa1  ,0x01,                   // COLLECTION (Application)
    0x85  ,REPORT_ID_MODE_ID_FEATURE,//   REPORT_ID (04)
    0x09  ,0x23,                   //   USAGE (Device Setting)
    0xa1  ,0x02,                   //   COLLECTION (Logical)
    0x09  ,0x52,                   //     USAGE (Device mode)
    0x09  ,0x53,                   //     USAGE (Device Identifier)
    0x15  ,0x00,                   //     LOGICAL_MINIMUM (0)
    0x25  ,0x0a,                   //     LOGICAL_MAXIMUM (10)
    0x75  ,0x08,                   //     REPORT_SIZE (8)
    0x95  ,0x02,                   //     REPORT_COUNT (2)
    0xb1  ,0x02,                   //     FEATURE (Data,Var,Abs)
    0xc0  ,                        //   END_COLLECTION
    0xc0  ,                        // END_COLLECTION  */
}; /* CustomHID_ReportDescriptor */


#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  
__ALIGN_BEGIN static uint8_t SetupReportBuffer[80] __ALIGN_END =
{
0,
};



#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  
__ALIGN_BEGIN TOUCH_MODE_DEF gTouchMode __ALIGN_END = MOUSE_MODE;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  
__ALIGN_BEGIN static uint32_t  u32TouchUsbBusyFlag  __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  
__ALIGN_BEGIN static uint8_t  u8TouchOutBuf[TOUCH_RECIEVE_BUF_LEN]  __ALIGN_END;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  
__ALIGN_BEGIN static uint8_t  u8KeyBoardOutBuf[HID_KEYBOARD_OUT_PACKET]  __ALIGN_END;


#define TOUCH_CTRL_BUFFER_LEN               64
typedef struct cvteTOUCH_CTRL_S
{
    uint16_t TouchCtlLen;
    uint8_t  TouchCtlCmd;
    uint8_t  TouchCtlUnit;
    uint8_t  TouchCtl[TOUCH_CTRL_BUFFER_LEN];
} TOUCH_CTRL_S, *PTR_TOUCH_CTRL_S;
static TOUCH_CTRL_S gTouchCtrlStruct;
static PTR_TOUCH_CTRL_S gpTouchCtrlStruct = &gTouchCtrlStruct;


/**
  * @}
  */ 

/** @defgroup USBD_HID_Private_Functions
  * @{
  */ 

/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_Init (void  *pdev, 
                               uint8_t cfgidx)
{
  DBG("%s", __func__);
  /* Open EP IN */
  DCD_EP_Open(pdev,
              HID_TOUCH_IN_EP,
              HID_TOUCH_IN_PACKET,
              USB_OTG_EP_INT);
  DCD_EP_Open(pdev,
              HID_KEYBOARD_IN_EP,
              HID_KEYBOARD_IN_PACKET,
              USB_OTG_EP_INT);
#if (CFG_ON == CFG_EMR_PEN_USED)
  DCD_EP_Open(pdev,
              HID_EMR_PEN_IN_EP,
              HID_EMR_PEN_IN_PACKET,
              USB_OTG_EP_INT);
#endif
  
  /* Open EP OUT */
  DCD_EP_Open(pdev,
              HID_TOUCH_OUT_EP,
              HID_TOUCH_OUT_PACKET,
              USB_OTG_EP_INT);
  DCD_EP_PrepareRx(pdev,
                    HID_TOUCH_OUT_EP,
                    u8TouchOutBuf,
                    TOUCH_RECIEVE_BUF_LEN);
  DCD_EP_Open(pdev,
              HID_KEYBOARD_OUT_EP,
              HID_KEYBOARD_OUT_PACKET,
              USB_OTG_EP_INT);
  DCD_EP_PrepareRx(pdev,
                    HID_KEYBOARD_OUT_EP,
                    u8KeyBoardOutBuf,
                    HID_KEYBOARD_OUT_PACKET);

  memset(gpTouchCtrlStruct, 0x00, sizeof(TOUCH_CTRL_S));
  memset(u8TouchOutBuf, 0x00, TOUCH_RECIEVE_BUF_LEN);
  memset(u8KeyBoardOutBuf, 0x00, HID_KEYBOARD_OUT_PACKET);
  gTouchMode = MOUSE_MODE;
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_DeInit (void  *pdev, 
                                 uint8_t cfgidx)
{
  DBG("%s", __func__);
  /* Close HID EPs */
  DCD_EP_Close (pdev , HID_TOUCH_IN_EP);
  DCD_EP_Close (pdev , HID_TOUCH_OUT_EP);
  DCD_EP_Close (pdev , HID_KEYBOARD_IN_EP);
  DCD_EP_Close (pdev , HID_KEYBOARD_OUT_EP);
#if (CFG_ON == CFG_EMR_PEN_USED)
  DCD_EP_Close (pdev , HID_EMR_PEN_IN_EP);
#endif
  
  
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_HID_Setup (void  *pdev, 
                                USB_SETUP_REQ *req)
{
  uint16_t len = 0;
  uint8_t  *pbuf = NULL;
  
  DBG("%s", __func__);
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS : 
    DBG("%s USB_REQ_TYPE_CLASS", __func__); 
    switch (req->bRequest)
    {
    case HID_REQ_SET_PROTOCOL:
      USBD_HID_Protocol = (uint8_t)(req->wValue);
      break;
      
    case HID_REQ_GET_PROTOCOL:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_HID_Protocol,
                        1);    
      break;
      
    case HID_REQ_SET_IDLE:
      USBD_HID_IdleState = (uint8_t)(req->wValue >> 8);
      break;
      
    case HID_REQ_GET_IDLE:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_HID_IdleState,
                        1);        
      break;  
    // =========== Add by QWB for MTouch START ===============
    case HID_REQ_GET_REPORT: 
    {
        DBG(">>HID_REQ_GET_REPORT wValue[0x%02x],wIndex[0x%02x],wLength[0x%02x]",
            req->wValue, req->wIndex, req->wLength);
        if (((HID_REPORT_FEATURE << 8) | REPORT_ID_CONTACT_CNT_MAX_FEATURE) \
            == req->wValue)
        {
            DBG(">>REPORT_ID_CONTACT_CNT_MAX_FEATURE");
            memset(SetupReportBuffer, 0x00, sizeof(SetupReportBuffer));
            SetupReportBuffer[0] = REPORT_ID_CONTACT_CNT_MAX_FEATURE;
            SetupReportBuffer[1] = DEFAULT_MTOUCH_CONTACT_COUNT;
            USBD_CtlSendData (  pdev, 
                                SetupReportBuffer,
                                req->wLength); 
        }
        else if (((HID_REPORT_FEATURE << 8) | REPORT_ID_IR_PAIRS_FEATURE) \
            == req->wValue)
        {
            DBG(">>Android: test REPORT_ID_IR_PAIRS_FEATURE");
            memset(SetupReportBuffer, 0x00, sizeof(SetupReportBuffer));
            SetupReportBuffer[0] = REPORT_ID_IR_PAIRS_FEATURE;
            USBD_CtlSendData (  pdev, 
                                SetupReportBuffer,
                                req->wLength); 
        }
        else if (((HID_REPORT_FEATURE << 8) | REPORT_ID_MODE_ID_FEATURE) \
            == req->wValue)
        {
            DBG(">>Android: test REPORT_ID_MODE_ID_FEATURE");
            memset(SetupReportBuffer, 0x00, sizeof(SetupReportBuffer));
            SetupReportBuffer[0] = REPORT_ID_MODE_ID_FEATURE;
            SetupReportBuffer[1] = (u8)gTouchMode; // Mode
            SetupReportBuffer[2] = 0x01; //ID
            
            USBD_CtlSendData (  pdev, 
                                SetupReportBuffer,
                                req->wLength); 
        }
#if (CFG_ON == CFG_EMR_PEN_USED)
        else if (((HID_REPORT_FEATURE << 8) | REPORT_ID_EMR_FEATURE) \
            == req->wValue)
        {
            DBG(">>Android: test REPORT_ID_EMR_FEATURE");
            memset(SetupReportBuffer, 0x00, sizeof(SetupReportBuffer));
            SetupReportBuffer[0] = REPORT_ID_EMR_FEATURE;
            SetupReportBuffer[1] = 0x02;//-TODO-
            
            USBD_CtlSendData (  pdev, 
                                SetupReportBuffer,
                                req->wLength); 
        }
#endif
        else if (((HID_REQ_GET_REPORT << 8) | REPORT_ID_IR_PAIRS_INPUT) \
            == req->wValue)
        {
            DBG(">>Android: test REPORT_ID_IR_PAIRS_INPUT");
            memset(SetupReportBuffer, 0x00, sizeof(SetupReportBuffer));
            SetupReportBuffer[0] = REPORT_ID_IR_PAIRS_INPUT;
            USBD_CtlSendData (  pdev, 
                                SetupReportBuffer,
                                req->wLength); 
        }
        else if (((HID_REQ_GET_REPORT << 8) | REPORT_ID_MOUSE) \
            == req->wValue)
        {
            DBG(">>Android: test REPORT_ID_MOUSE");
            memset(SetupReportBuffer, 0x00, sizeof(SetupReportBuffer));
            SetupReportBuffer[0] = REPORT_ID_MOUSE;
            USBD_CtlSendData (  pdev, 
                                SetupReportBuffer,
                                req->wLength); 
        }
        else if (((HID_REQ_GET_REPORT << 8) | REPORT_ID_FC_SERVICE) \
            == req->wValue)
        {
            DBG(">>Android: test REPORT_ID_FC_SERVICE");
            memset(SetupReportBuffer, 0x00, sizeof(SetupReportBuffer));
            SetupReportBuffer[0] = REPORT_ID_FC_SERVICE;
            USBD_CtlSendData (  pdev, 
                                SetupReportBuffer,
                                req->wLength); 
        }
        else if (((HID_REQ_GET_REPORT << 8) | REPORT_ID_FB_SERVICE) \
            == req->wValue)
        {
            DBG(">>Android: test REPORT_ID_FB_SERVICE");
            memset(SetupReportBuffer, 0x00, sizeof(SetupReportBuffer));
            SetupReportBuffer[0] = REPORT_ID_FB_SERVICE;
            USBD_CtlSendData (  pdev, 
                                SetupReportBuffer,
                                req->wLength); 
        }
        else if (((HID_REQ_GET_REPORT << 8) | REPORT_ID_WIN7_MTOUCH) \
            == req->wValue)
        {
            DBG(">>Android: test REPORT_ID_WIN7_MTOUCH");
            memset(SetupReportBuffer, 0x00, sizeof(SetupReportBuffer));
            SetupReportBuffer[0] = REPORT_ID_WIN7_MTOUCH;
            USBD_CtlSendData (  pdev, 
                                SetupReportBuffer,
                                req->wLength); 
        }
        else if (((HID_REQ_GET_REPORT << 8) | 0x00) == req->wValue)
        {
            DBG(">>Android: test REPORTID=0x00");
            memset(SetupReportBuffer, 0x00, sizeof(SetupReportBuffer));
            USBD_CtlSendData (  pdev, 
                                SetupReportBuffer,
                                req->wLength); 
        }
#if (CFG_ON == CFG_EMR_PEN_USED)
        else if (((HID_REQ_GET_REPORT << 8) | REPORT_ID_EMR_ONE_PEN) \
            == req->wValue)
        {
            DBG(">>Android: test REPORT_ID_EMR_ONE_PEN");
            memset(SetupReportBuffer, 0x00, sizeof(SetupReportBuffer));
            SetupReportBuffer[0] = REPORT_ID_EMR_ONE_PEN;
            USBD_CtlSendData (  pdev, 
                                SetupReportBuffer,
                                req->wLength); 
        }
        else if (((HID_REQ_GET_REPORT << 8) | REPORT_ID_EMR_TWO_PEN) \
            == req->wValue)
        {
            DBG(">>Android: test REPORT_ID_EMR_TWO_PEN");
            memset(SetupReportBuffer, 0x00, sizeof(SetupReportBuffer));
            SetupReportBuffer[0] = REPORT_ID_EMR_TWO_PEN;
            USBD_CtlSendData (  pdev, 
                                SetupReportBuffer,
                                req->wLength); 
        }
#endif
        else
        {
            DBG("%s HID_REQ_GET_REPORT Unknown!", __func__);
            USBD_CtlError (pdev, req);
            return USBD_FAIL; 
        }
    }
    break;
    case HID_REQ_SET_REPORT: 
    {
        DBG(">>HID_REQ_SET_REPORT wValue[0x%02x],wIndex[0x%02x],wLength[0x%02x]",
            req->wValue, req->wIndex, req->wLength);
        if (((HID_REPORT_FEATURE << 8) | REPORT_ID_MODE_ID_FEATURE) == req->wValue)
        {
            DBG(">>REPORT_ID_MODE_ID_FEATURE");
            if (req->wLength)
            {
                /* Prepare the reception of the buffer over EP0 */
                USBD_CtlPrepareRx (pdev, 
                                    gpTouchCtrlStruct->TouchCtl,
                                    req->wLength);

                /* Set the global variables indicating current request and its length 
                to the function USBD_HID_EP0_RxReady() which will process the request */
                gpTouchCtrlStruct->TouchCtlCmd = REPORT_ID_MODE_ID_FEATURE;/* Set the request value */
                gpTouchCtrlStruct->TouchCtlLen = req->wLength;          /* Set the request data length */
                gpTouchCtrlStruct->TouchCtlUnit = HIBYTE(req->wIndex);  /* Set the request target unit */
            }
        }
        else
        {
            DBG("%s HID_REQ_SET_REPORT Unknown!", __func__);
            USBD_CtlError (pdev, req);
            return USBD_FAIL; 
        }
    }
    break;
    // =========== Add by QWB for MTouch END ===============
    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL; 
    }
    break;
    
  case USB_REQ_TYPE_STANDARD:
    DBG("%s USB_REQ_TYPE_STANDARD", __func__); 
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR: 
      if( req->wValue >> 8 == HID_REPORT_DESC)
      {
        if (INTF_NUM1_FOR_KEYBOARD == req->wIndex)
        {
            DBG("%s:HID_REPORT_DESC index=%d", __func__, req->wIndex);
            len = MIN(KEYBOARD_SIZE_REPORT_DESC , req->wLength);
            pbuf = Keyboard_ReportDescriptor;
        }
#if (CFG_ON == CFG_EMR_PEN_USED)
        else if (INTF_NUM2_FOR_EMR_PEN == req->wIndex)
        {
            DBG("%s:HID_REPORT_DESC index=%d", __func__, req->wIndex);
            len = MIN(sizeof(EMRPenReportDescriptor) , req->wLength);
            pbuf = EMRPenReportDescriptor;
        }
#endif
        else
        {
            DBG("%s:HID_REPORT_DESC index=%d", __func__, req->wIndex);
            len = MIN(CUSTOMHID_SIZ_HID_REPORT_DESC_WIN7 , req->wLength);
            pbuf = CustomHID_ReportDescriptor;
        }
      }
      else if( req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
      {
        if (INTF_NUM1_FOR_KEYBOARD == req->wIndex)
        {
            DBG("%s:HID_DESCRIPTOR_TYPE index=%d", __func__, req->wIndex);
            pbuf = USBD_HID_CfgDesc + INTF_NUM1_DESC_TYPE_OFFSET;
            len = MIN(USB_HID_DESC_SIZ , req->wLength);
        }
#if (CFG_ON == CFG_EMR_PEN_USED)
        else if (INTF_NUM2_FOR_EMR_PEN == req->wIndex)
        {
            DBG("%s:HID_DESCRIPTOR_TYPE index=%d", __func__, req->wIndex);
            pbuf = USBD_HID_CfgDesc + INTF_NUM2_DESC_TYPE_OFFSET;
            len = MIN(USB_HID_DESC_SIZ , req->wLength);
        }
#endif
        else
        {
            DBG("%s:HID_DESCRIPTOR_TYPE index=%d", __func__, req->wIndex);
            pbuf = USBD_HID_CfgDesc + INTF_NUM0_DESC_TYPE_OFFSET;
            len = MIN(USB_HID_DESC_SIZ , req->wLength);
        }
      }
      
      USBD_CtlSendData (pdev, 
                        pbuf,
                        len);
      
      break;
      
    case USB_REQ_GET_INTERFACE :
      DBG("%s USB_REQ_GET_INTERFACE", __func__); 
      USBD_CtlSendData (pdev,
                        (uint8_t *)&USBD_HID_AltSet,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      DBG("%s USB_REQ_SET_INTERFACE", __func__); 
      USBD_HID_AltSet = (uint8_t)(req->wValue);
      break;
    }
  }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_HID_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  DBG("%s", __func__);
  *length = sizeof (USBD_HID_CfgDesc);
  return USBD_HID_CfgDesc;
}

/**
  * @brief  USBD_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_HID_DataIn (void  *pdev, 
                              uint8_t epnum)
{
  DBG("%s epnum=%d", __func__, epnum);
  
  u32TouchUsbBusyFlag = USB_TOUCH_IDLE;
  
  /* Ensure that the FIFO is empty before a new transfer, this condition could 
  be caused by  a new transfer before the end of the previous transfer */
  if ((HID_TOUCH_IN_EP&0x7f) == epnum)
  {
    DCD_EP_Flush(pdev, HID_TOUCH_IN_EP);
  }
  else if ((HID_KEYBOARD_IN_EP&0x7f) == epnum)
  {
    DCD_EP_Flush(pdev, HID_KEYBOARD_IN_EP);
  }
  
#if (CFG_ON == CFG_EMR_PEN_USED)
  else if ((HID_EMR_PEN_IN_EP&0x7f) == epnum)
  {
    DCD_EP_Flush(pdev, HID_EMR_PEN_IN_EP);
  }
#endif

  return USBD_OK;
}

/*****************************************************************************
 Prototype    : USBD_HID_EP0_RxReady
 Description  : 
 Input        : void  *pdev  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2016/12/16
    Author       : qiuweibo
    Modification : Created function

*****************************************************************************/
static uint8_t  USBD_HID_EP0_RxReady (void  *pdev)
{ 
    DBG("%s", __func__);
    if (REPORT_ID_MODE_ID_FEATURE == gpTouchCtrlStruct->TouchCtlCmd)
    {
        if (3 == gpTouchCtrlStruct->TouchCtlLen)
        {
            DBG("REPORT_ID_MODE_ID_FEATURE [0]=0x%02x,[1]=0x%02x,[2]=0x%02x",
                gpTouchCtrlStruct->TouchCtl[0], // REPORT ID
                gpTouchCtrlStruct->TouchCtl[1], // DEVICE MODE
                gpTouchCtrlStruct->TouchCtl[2]); //DEVICE IDENTIFIER
            gTouchMode = (TOUCH_MODE_DEF)gpTouchCtrlStruct->TouchCtl[1];
            switch (gTouchMode)
            {
            case MOUSE_MODE:
            {
                DBG("MOUSE_MODE");
            }
            break;
            case SINGLE_TOUCH_MODE:
            {
                DBG("SINGLE_TOUCH_MODE");
            }
            break;
            case MULTI_TOUCH_MODE:
            {
                DBG("MULTI_TOUCH_MODE");
            }
            break;
            default :
            {
                DBG("TOUCH_MODE Unknown!!");
            }
            break;
            }
        }
        
        /* Reset the variable to prevent re-entering this function */
        memset(gpTouchCtrlStruct->TouchCtl, 0x00, TOUCH_CTRL_BUFFER_LEN);
        gpTouchCtrlStruct->TouchCtlCmd = 0;
        gpTouchCtrlStruct->TouchCtlLen = 0;
        gpTouchCtrlStruct->TouchCtlUnit = 0;
    } 

    return USBD_OK;
}

/*****************************************************************************
 Prototype    : USBD_HID_EP0_TxSent
 Description  : 
 Input        : void  *pdev  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2016/12/17
    Author       : qiuweibo
    Modification : Created function

*****************************************************************************/
static uint8_t  USBD_HID_EP0_TxSent (void  *pdev)
{ 
    DBG("%s", __func__);
    return USBD_OK;
}

/*****************************************************************************
 Prototype    : USBD_HID_DataOut
 Description  : 
 Input        : void *pdev     
                uint8_t epnum  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2016/12/17
    Author       : qiuweibo
    Modification : Created function

*****************************************************************************/
static uint8_t  USBD_HID_DataOut (void *pdev, uint8_t epnum)
{
    DBG("%s", __func__);
    if (HID_TOUCH_OUT_EP == epnum)
    {
        DBG(">>HID_TOUCH_OUT_EP");
        MTouchRecvDataPush(u8TouchOutBuf, TOUCH_RECIEVE_BUF_LEN);
        memset(u8TouchOutBuf, 0x00, TOUCH_RECIEVE_BUF_LEN);
        DCD_EP_PrepareRx(pdev,
                        HID_TOUCH_OUT_EP,
                        u8TouchOutBuf,
                        TOUCH_RECIEVE_BUF_LEN);
    }
    else if (HID_KEYBOARD_OUT_EP == epnum)
    {
        DBG(">>HID_KEYBOARD_OUT_EP");
        KeyBoardRecvDataPush(u8KeyBoardOutBuf, HID_KEYBOARD_OUT_PACKET);
        memset(u8KeyBoardOutBuf, 0x00, HID_KEYBOARD_OUT_PACKET);
        DCD_EP_PrepareRx(pdev,
                        HID_KEYBOARD_OUT_EP,
                        u8KeyBoardOutBuf,
                        HID_KEYBOARD_OUT_PACKET);
    }
    else
    {
        DBG("%s Unknown epnum=%d", __func__, epnum);
    }

    return USBD_OK;
}

/*****************************************************************************
 Prototype    : USBD_HID_SOF
 Description  : 
 Input        : void *pdev  
 Output       : None
 Return Value : static
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2016/12/17
    Author       : qiuweibo
    Modification : Created function

*****************************************************************************/
static uint8_t  USBD_HID_SOF (void *pdev)
{
    //DBG("%s", __func__);
    return USBD_OK;
}


/*****************************************************************************
 Prototype    : USBD_HID_SendTouchReport
 Description  : 
 Input        : USB_OTG_CORE_HANDLE  *pdev  
                uint8_t *report             
                uint16_t len                
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2016/12/17
    Author       : qiuweibo
    Modification : Created function

*****************************************************************************/
uint8_t USBD_HID_SendTouchReport (USB_OTG_CORE_HANDLE  *pdev, 
                                 uint8_t *report,
                                 uint16_t len)
{
    DBG("%s", __func__);
    if (pdev->dev.device_status == USB_OTG_CONFIGURED )
    {
        u32TouchUsbBusyFlag = USB_TOUCH_BUSY;
        DCD_EP_Tx (pdev, HID_TOUCH_IN_EP, report, len);
    }
    return USBD_OK;
}

/*****************************************************************************
 Prototype    : USBD_HID_SendKeyboardReport
 Description  : 
 Input        : USB_OTG_CORE_HANDLE  *pdev  
                uint8_t *report             
                uint16_t len                
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2016/12/17
    Author       : qiuweibo
    Modification : Created function

*****************************************************************************/
uint8_t USBD_HID_SendKeyboardReport (USB_OTG_CORE_HANDLE  *pdev, 
                                 uint8_t *report,
                                 uint16_t len)
{
    DBG("%s", __func__);
    if (pdev->dev.device_status == USB_OTG_CONFIGURED )
    {
        printf("\r\n>>report:0x%02x, 0x%02x, 0x%02x", report[0],report[1],report[2]);
        u32TouchUsbBusyFlag = USB_TOUCH_BUSY;
        DCD_EP_Tx (pdev, HID_KEYBOARD_IN_EP, report, len);
    }
    return USBD_OK;
}

#if (CFG_ON == CFG_EMR_PEN_USED)
uint8_t USBD_HID_SendEmrPenReport (USB_OTG_CORE_HANDLE  *pdev, 
                                 uint8_t *report,
                                 uint16_t len)
{
    DBG("%s", __func__);
    if (pdev->dev.device_status == USB_OTG_CONFIGURED )
    {
        printf("\r\n>>report:0x%02x, 0x%02x, 0x%02x", report[0],report[1],report[2]);
        u32TouchUsbBusyFlag = USB_TOUCH_BUSY;
        DCD_EP_Tx (pdev, HID_EMR_PEN_IN_EP, report, len);
    }
    return USBD_OK;
}
#endif

uint8_t USBD_HID_CheckUsbIdle(void)
{
    if (USB_TOUCH_IDLE == u32TouchUsbBusyFlag)
    {
        return 1;
    }
    return 0;
}

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
