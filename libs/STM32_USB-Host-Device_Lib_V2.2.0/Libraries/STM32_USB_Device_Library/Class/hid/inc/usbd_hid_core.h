/**
  ******************************************************************************
  * @file    usbd_hid_core.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-July-2011
  * @brief   header file for the usbd_hid_core.c file.
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

#ifndef __USB_HID_CORE_H_
#define __USB_HID_CORE_H_

#include "usbd_ioreq.h"
#include "configs.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup USBD_HID
  * @brief This file is the Header file for USBD_msc.c
  * @{
  */ 


/** @defgroup USBD_HID_Exported_Defines
  * @{
  */ 
#define USB_HID_DESC_SIZ                9

#define HID_DESCRIPTOR_TYPE             0x21
#define HID_REPORT_DESC                 0x22


#define HID_REQ_SET_PROTOCOL            0x0B
#define HID_REQ_GET_PROTOCOL            0x03

#define HID_REQ_SET_IDLE                0x0A
#define HID_REQ_GET_IDLE                0x02

#define HID_REQ_SET_REPORT              0x09
#define HID_REQ_GET_REPORT              0x01

#define HID_REPORT_FEATURE              0x03

#define REPORT_ID_EMR_ONE_PEN           0x10 //1+7 Bytes
#define REPORT_ID_EMR_TWO_PEN           0x11 //1+17 Bytes
#define REPORT_ID_EMR_FEATURE           0x12 //

#define REPORT_ID_IR_PAIRS_INPUT        0xfd   //Input Data (6 bytes)
#define REPORT_ID_IR_PAIRS_FEATURE      0xfe   //FEATURE (4 bytes)
#define REPORT_ID_MOUSE                 0x01   //Mouse/Single-Touch, Input Data (7 bytes)
#define REPORT_ID_FB_SERVICE            0xfb   //Input Data (63 bytes), Output Data (63 bytes)
#define REPORT_ID_FC_SERVICE            0xfc   //Input Data (63 bytes), Output Data (63 bytes)
#define REPORT_ID_WIN7_MTOUCH           0x02   //Multi-Touch, Input Data (61 bytes)
#define REPORT_ID_CONTACT_CNT_MAX_FEATURE   0x03   //FEATURE (1 bytes)
#define REPORT_ID_MODE_ID_FEATURE       0x04   //FEATURE (2 bytes)

#define SEND_KEYBOARD_DATA_LEN          8 // (1 byte功能键 + 1 byte OEM Reserved + 6 Key Vlaues)
#define MTOUCH_REPORT_POINTS_MAX        6 // 1个report最大容纳6个ID

#define DEFAULT_MTOUCH_CONTACT_COUNT    20 // -QWB- 默认最大触摸点数目

#define TOUCH_RECIEVE_BUF_LEN           64
#define FC_BUF_LEN                      TOUCH_RECIEVE_BUF_LEN // -QWB-
#define FB_BUF_LEN                      TOUCH_RECIEVE_BUF_LEN // -QWB-

//**************************** www.huaxintouch.com START **************************
#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define HID_DESCRIPTOR_TYPE                     0x21
#define CUSTOMHID_SIZ_HID_DESC                  0x09
#define CUSTOMHID_OFF_HID_DESC                  0x12

#if CHG_PHY_SIZE==1
#define LOBYTE(x) (x&0xff)
#define HIBYTE(x) (x>>8&0xff)
#endif

#define KEYBOARD_SIZE_REPORT_DESC               45

#define KEY_WIN     0x08
#define KEY_CTRL    0x09
#define KEY_L       0x0F
#define KEY_R       0x15
#define KEY_PLUS    0x2E
#define KEY_MINUS   0x2D
#define KEY_F10     0x43
#if WIN7_WIDTH==1
    #if CHG_PHY_SIZE==1
    #define CUSTOMHID_SIZ_HID_REPORT_DESC_WIN7      (0x02b2+24+18)
    #else
    #define CUSTOMHID_SIZ_HID_REPORT_DESC_WIN7      (0x02b2+24)
    #endif
#else
    #if CHG_PHY_SIZE==1
    #define CUSTOMHID_SIZ_HID_REPORT_DESC_WIN7      (0x0276+24+18)
    #else
    #define CUSTOMHID_SIZ_HID_REPORT_DESC_WIN7      (0x0276+24)
    #endif
#endif

#define CUSTOMHID_SIZ_HID_REPORT_DESC           0x00d8

#define CUSTOMHID_SIZ_DEVICE_DESC               18

#if (CFG_ON == CFG_EMR_PEN_USED)
#define CUSTOMHID_SIZ_CONFIG_DESC               98
#else
#define CUSTOMHID_SIZ_CONFIG_DESC               73
#endif


#define CUSTOMHID_SIZ_STRING_LANGID             4
#define CUSTOMHID_SIZ_STRING_VENDOR             12

// 固定长度是为了修改描述符不影响固件的长度，112字节刚好能放进两个下载的包
#define CUSTOMHID_SIZ_STRING_PRODUCT            112 //2 + COMPANY_DESC_LENGTH + DESC_LENGTH

#define CUSTOMHID_SIZ_STRING_SERIAL             36

#define STANDARD_ENDPOINT_DESC_SIZE             0x09

#define  BCD_DEVICE ((Firmware_Version/0x100)|((Firmware_Version%0x100)*0x100))
//**************************** www.huaxintouch.com END **************************


typedef enum
{
    MOUSE_MODE = 0x00,
    SINGLE_TOUCH_MODE,
    MULTI_TOUCH_MODE,
} TOUCH_MODE_DEF;




/**
  * @}
  */ 


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */


/**
  * @}
  */ 



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */ 

extern USBD_Class_cb_TypeDef  USBD_HID_cb;
/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */ 
uint8_t USBD_HID_SendTouchReport (USB_OTG_CORE_HANDLE  *pdev, 
                                 uint8_t *report,
                                 uint16_t len);
uint8_t USBD_HID_SendKeyboardReport (USB_OTG_CORE_HANDLE  *pdev, 
                                 uint8_t *report,
                                 uint16_t len);
uint8_t USBD_HID_CheckUsbIdle(void);
/**
  * @}
  */ 

#endif  // __USB_HID_CORE_H_
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
