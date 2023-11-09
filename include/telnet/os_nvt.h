/* os_nvt.h
*
* 网络虚拟终端(Network Virtual Terminal)操作系统层适配接口相关声明
*
* Neo-T, 创建于2022.06.17 18:09
*
*/
#ifndef OS_NVT_H
#define OS_NVT_H

#ifdef SYMBOL_GLOBALS
#define OS_NVT_EXT
#else
#define OS_NVT_EXT extern
#endif //* SYMBOL_GLOBALS

#ifdef PKG_ONPS_NETTOOLS_TELNETSRV
OS_NVT_EXT void os_nvt_init(void);
OS_NVT_EXT void os_nvt_uninit(void);
OS_NVT_EXT BOOL os_nvt_start(void *pvParam);
OS_NVT_EXT void os_nvt_stop(void *pvParam, BOOL blIsNvtEnd);

#if defined(PKG_ONPS_SUPPORT_ETHERNET) && defined(PKG_ONPS_NVTCMD_IFIP_EN)
#ifdef PKG_ONPS_ETH_EXTRA_IP_EN
OS_NVT_EXT BOOL os_nvt_add_ip(const CHAR *pszIfName, in_addr_t unIp, in_addr_t unSubnetMask);  //* 将新添加的ip地址写入目标系统的非易失性存储器
OS_NVT_EXT BOOL os_nvt_del_ip(const CHAR *pszIfName, in_addr_t unIp);                          //* 从目标系统的非易失性存储器中删除之前添加的ip地址
#endif //* #ifdef PKG_ONPS_ETH_EXTRA_IP_EN
OS_NVT_EXT BOOL os_nvt_set_ip(const CHAR *pszIfName, in_addr_t unIp, in_addr_t unSubnetMask, in_addr_t unGateway);
OS_NVT_EXT BOOL os_nvt_set_mac(const CHAR *pszIfName, const CHAR *pszMac);
OS_NVT_EXT BOOL os_nvt_set_dns(const CHAR *pszIfName, in_addr_t unPrimaryDns, in_addr_t unSecondaryDns);
OS_NVT_EXT BOOL os_nvt_set_dhcp(const CHAR *pszIfName);
OS_NVT_EXT void os_nvt_system_reset(void);
#endif //* #if defined(PKG_ONPS_SUPPORT_ETHERNET) && defined(PKG_ONPS_NVTCMD_IFIP_EN)

#ifdef PKG_ONPS_NVTCMD_ROUTE_EN
OS_NVT_EXT BOOL os_nvt_add_route_entry(const CHAR *pszIfName, in_addr_t unDestination, in_addr_t unGenmask, in_addr_t unGateway);
OS_NVT_EXT BOOL os_nvt_del_route_entry(in_addr_t unDestination);
#endif //* #ifdef PKG_ONPS_NVTCMD_ROUTE_EN

#if defined(PKG_ONPS_NETTOOLS_PING) && defined(PKG_ONPS_NVTCMD_PING_EN)
OS_NVT_EXT UINT os_get_elapsed_millisecs(void);
#endif //* #if defined(PKG_ONPS_NETTOOLS_PING) && defined(PKG_ONPS_NVTCMD_PING_EN)

#if defined(PKG_ONPS_NETTOOLS_SNTP) && defined(PKG_ONPS_NVTCMD_NTP_EN)
OS_NVT_EXT void os_nvt_set_system_time(time_t tNtpTime);
#endif //* #if defined(PKG_ONPS_NETTOOLS_SNTP) && defined(PKG_ONPS_NVTCMD_NTP_EN)

#endif //* #ifdef PKG_ONPS_NETTOOLS_TELNETSRV

#endif
