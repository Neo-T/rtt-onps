/*
 * 版权属于onps栈开发团队，遵循Apache License 2.0开源许可协议
 *
 */
#include "port/datatype.h"
#include "port/sys_config.h"
#include "onps_errors.h"
#include "port/os_datatype.h"
#include "one_shot_timer.h"
#include "onps_utils.h"
#include "protocols.h"
#include "onps_input.h"

#ifdef ONPS_ENABLE_NETTOOLS_TELNETSRV
#define SYMBOL_GLOBALS
#include "telnet/os_nvt.h"
#undef SYMBOL_GLOBALS



void os_nvt_init(void)
{
}

void os_nvt_uninit(void)
{
}

BOOL os_nvt_start(void *pvParam)
{
}

void os_nvt_stop(void *pvParam, BOOL blIsNvtEnd)
{
}

#if defined(ONPS_ENABLE_ETHERNET) && defined(ONPS_ENABLE_NVTCMD_IFIP)
#ifdef ONPS_ENABLE_ETH_EXTRA_IP
BOOL os_nvt_add_ip(const CHAR *pszIfName, in_addr_t unIp, in_addr_t unSubnetMask)
{
  return TRUE;
}

BOOL os_nvt_del_ip(const CHAR *pszIfName, in_addr_t unIp)
{
  return TRUE;
}
#endif //* #ifdef ONPS_ENABLE_ETH_EXTRA_IP
BOOL os_nvt_set_ip(const CHAR *pszIfName, in_addr_t unIp, in_addr_t unSubnetMask, in_addr_t unGateway)
{
  return TRUE;
}

BOOL os_nvt_set_mac(const CHAR *pszIfName, const CHAR *pszMac)
{
  return TRUE;
}

BOOL os_nvt_set_dns(const CHAR *pszIfName, in_addr_t unPrimaryDns, in_addr_t unSecondaryDns)
{
  return TRUE;
}

BOOL os_nvt_set_dhcp(const CHAR *pszIfName)
{
  return TRUE;
}

void os_nvt_system_reset(void)
{
}
#endif //* #if defined(ONPS_ENABLE_ETHERNET) && defined(ONPS_ENABLE_NVTCMD_IFIP)

#ifdef ONPS_ENABLE_NVTCMD_ROUTE
BOOL os_nvt_add_route_entry(const CHAR *pszIfName, in_addr_t unDestination, in_addr_t unGenmask, in_addr_t unGateway)
{
  return TRUE;
}

BOOL os_nvt_del_route_entry(in_addr_t unDestination)
{
  return TRUE;
}
#endif //* #ifdef ONPS_ENABLE_NVTCMD_ROUTE

#if defined(ONPS_ENABLE_NETTOOLS_PING) && defined(ONPS_ENABLE_NVTCMD_PING)
UINT os_get_elapsed_millisecs(void)
{

      return 0; 
}
#endif //* #if defined(ONPS_ENABLE_NETTOOLS_PING) && defined(ONPS_ENABLE_NVTCMD_PING)

#if defined(ONPS_ENABLE_NETTOOLS_SNTP)  && defined(ONPS_ENABLE_NVTCMD_NTP)
void os_nvt_set_system_time(time_t tNtpTime)
{

}
#endif //* #if defined(ONPS_ENABLE_NETTOOLS_SNTP)  && defined(ONPS_ENABLE_NVTCMD_NTP)

#endif //* #ifdef ONPS_ENABLE_NETTOOLS_TELNETSRV
