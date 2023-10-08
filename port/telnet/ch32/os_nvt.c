/*
 * 版权属于onps栈开发团队，遵循Apache License 2.0开源许可协议
 *
 */
#include "port/datatype.h"
#include "port/sys_config.h"
#include "onps_errors.h"
#include "port/os_datatype.h"
#include "port/os_adapter.h"
#include "one_shot_timer.h"
#include "onps_utils.h"
#include "protocols.h"
#include "onps_input.h"
#include "netif/netif.h"

#ifdef ONPS_ENABLE_NETTOOLS_TELNETSRV
#include "ch32v30x.h"
#include "ch32v30x_eth.h"

#if defined(ONPS_ENABLE_NETTOOLS_PING) && defined(ONPS_ENABLE_NVTCMD_PING)
#include "board.h"
#endif //* #if defined(ONPS_ENABLE_NETTOOLS_PING) && defined(ONPS_ENABLE_NVTCMD_PING)

#if defined(ONPS_ENABLE_ETHERNET) && defined(ONPS_ENABLE_NVTCMD_IFIP)
//#include "at24c02.h"
#endif //* #if defined(ONPS_ENABLE_ETHERNET) && defined(ONPS_ENABLE_NVTCMD_IFIP)

#if defined(ONPS_ENABLE_NETTOOLS_SNTP)  && defined(ONPS_ENABLE_NVTCMD_NTP)
#include "drivers/rtc.h"
#endif //* #if defined(ONPS_ENABLE_NETTOOLS_SNTP)  && defined(ONPS_ENABLE_NVTCMD_NTP)

#define SYMBOL_GLOBALS
#include "telnet/os_nvt.h"
#undef SYMBOL_GLOBALS



#define THNVT_STK_SIZE 1180
static const UCHAR l_ubaTHNvtPrio[ONPS_NVTNUM_MAX] = {THNVT0_PRIO, THNVT1_PRIO};
static struct {
  rt_thread_t tid;
  void *pvParam;
} l_staTHNvt[ONPS_NVTNUM_MAX];
extern void thread_nvt_handler(void *pvParam);
void os_nvt_init(void)
{
  UCHAR i;
  for(i=0; i<ONPS_NVTNUM_MAX; i++)
    l_staTHNvt[i].pvParam = NULL;
}

void os_nvt_uninit(void)
{
}

BOOL os_nvt_start(void *pvParam)
{
  UCHAR i;
  for(i=0; i<ONPS_NVTNUM_MAX; i++)
  {
    if(NULL == l_staTHNvt[i].pvParam)
    {
      l_staTHNvt[i].tid = rt_thread_create("thread_nvt_handler", thread_nvt_handler, pvParam, THNVT_STK_SIZE, l_ubaTHNvtPrio[i], 5);
      if(l_staTHNvt[i].tid != RT_NULL)
      {
        rt_thread_startup(l_staTHNvt[i].tid);
        l_staTHNvt[i].pvParam = pvParam;
        return TRUE;
      }
    }
  }

  return FALSE;
}

void os_nvt_stop(void *pvParam, BOOL blIsNvtEnd)
{
  UCHAR i;
  for(i=0; i<ONPS_NVTNUM_MAX; i++)
  {
    if(pvParam == l_staTHNvt[i].pvParam && RT_NULL != l_staTHNvt[i].tid)
    {
      if(!blIsNvtEnd)
          rt_thread_delete(l_staTHNvt[i].tid);
      l_staTHNvt[i].pvParam = NULL;
    }
  }
}

#if defined(ONPS_ENABLE_ETHERNET) && defined(ONPS_ENABLE_NVTCMD_IFIP)
#ifdef ONPS_ENABLE_ETH_EXTRA_IP
BOOL os_nvt_add_ip(const CHAR *pszIfName, in_addr_t unIp, in_addr_t unSubnetMask)
{
#warning "os_nvt_add_ip() not yet implemented"
  return TRUE;
}

BOOL os_nvt_del_ip(const CHAR *pszIfName, in_addr_t unIp)
{
#warning "os_nvt_del_ip() not yet implemented"
  return TRUE;
}
#endif //* #ifdef ONPS_ENABLE_ETH_EXTRA_IP
BOOL os_nvt_set_ip(const CHAR *pszIfName, in_addr_t unIp, in_addr_t unSubnetMask, in_addr_t unGateway)
{
#warning "os_nvt_set_ip() not yet implemented"
  return TRUE;
}

BOOL os_nvt_set_mac(const CHAR *pszIfName, const CHAR *pszMac)
{
#warning "os_nvt_set_mac() not yet implemented"
  return TRUE;
}

BOOL os_nvt_set_dns(const CHAR *pszIfName, in_addr_t unPrimaryDns, in_addr_t unSecondaryDns)
{
#warning "os_nvt_set_dns() not yet implemented"
  return TRUE;
}

BOOL os_nvt_set_dhcp(const CHAR *pszIfName)
{
#warning "os_nvt_set_dhcp() not yet implemented"
  return TRUE;
}

void os_nvt_system_reset(void)
{
  NVIC_SystemReset();
}
#endif //* #if defined(ONPS_ENABLE_ETHERNET) && defined(ONPS_ENABLE_NVTCMD_IFIP)

#ifdef ONPS_ENABLE_NVTCMD_ROUTE
BOOL os_nvt_add_route_entry(const CHAR *pszIfName, in_addr_t unDestination, in_addr_t unGenmask, in_addr_t unGateway)
{
#warning "os_nvt_add_route_entry() not yet implemented"
  return TRUE;
}

BOOL os_nvt_del_route_entry(in_addr_t unDestination)
{
#warning "os_nvt_del_route_entry() not yet implemented"
  return TRUE;
}
#endif //* #ifdef ONPS_ENABLE_NVTCMD_ROUTE

#if defined(ONPS_ENABLE_NETTOOLS_PING) && defined(ONPS_ENABLE_NVTCMD_PING)
UINT os_get_elapsed_millisecs(void)
{
  return os_get_system_msecs();
}
#endif //* #if defined(ONPS_ENABLE_NETTOOLS_PING) && defined(ONPS_ENABLE_NVTCMD_PING)

#if defined(ONPS_ENABLE_NETTOOLS_SNTP)  && defined(ONPS_ENABLE_NVTCMD_NTP)
void os_nvt_set_system_time(time_t tNtpTime)
{
  set_timestamp(tNtpTime);
}
#endif //* #if defined(ONPS_ENABLE_NETTOOLS_SNTP)  && defined(ONPS_ENABLE_NVTCMD_NTP)

#endif //* #ifdef ONPS_ENABLE_NETTOOLS_TELNETSRV
