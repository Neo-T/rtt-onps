/********************************** (C) COPYRIGHT *******************************
* File Name          : eth_driver.h
* Author             : Neo-T
* Version            : V1.0.0
* Date               : 2023/08/14
* Description        : This file contains the headers of the ETH Driver.
*********************************************************************************/
#ifndef _ETH_DRIVER
#define _ETH_DRIVER

#ifdef __cplusplus
 extern "C" {
#endif

#ifdef SYMBOL_GLOBALS
   #define ETH_EXT
#else
   #define ETH_EXT extern
#endif //* SYMBOL_GLOBALS

#define PHY_ADDRESS 1

#define PHY_ANLPAR_SELECTOR_FIELD   0x1F
#define PHY_ANLPAR_SELECTOR_VALUE   0x01       /* 5B'00001 */

#define PHY_PN_SWITCH_P     (0<<2)
#define PHY_PN_SWITCH_N     (1<<2)
#define PHY_PN_SWITCH_AUTO  (2<<2)

#define DHCP_REQ_ADDR_EN    1       //* dhcp请求ip地址使能宏

#define NETIF_ETH_NAME      "eth0"  //* ethernet网卡名称
#define DP83848_PHY_ADDRESS 0x01    //* phy地址，参见原理图

//* mac地址定义
#define MAC_ADDR0_DEFAULT 0x4E
#define MAC_ADDR1_DEFAULT 0x65
#define MAC_ADDR2_DEFAULT 0x6F
#define MAC_ADDR3_DEFAULT 0x22
#define MAC_ADDR4_DEFAULT 0x06
#define MAC_ADDR5_DEFAULT 0x01

#define PRIMARY_DNS_DEFAULT   "1.2.4.8"
#define SECONDARY_DNS_DEFAULT "8.8.8.8"

ETH_EXT void ETH_IRQHandler(void);
ETH_EXT BOOL EthInit(void);
ETH_EXT INT EthSend(SHORT sBufListHead, UCHAR *pubErr);

#ifdef __cplusplus
}
#endif

#endif
