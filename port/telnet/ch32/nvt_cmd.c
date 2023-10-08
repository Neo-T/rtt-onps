/*
* 版权属于onps栈开发团队，遵循Apache License 2.0开源许可协议
*
*/
#include "port/datatype.h"
#include "port/sys_config.h"
#include "onps_errors.h"
#include "port/os_datatype.h"
#include "port/os_adapter.h"
#include "mmu/buddy.h"
#include "onps_utils.h"
#include "onps_input.h"
#include "netif/netif.h"
#include "netif/route.h"

#ifdef ONPS_ENABLE_NETTOOLS_TELNETSRV
#include "ch32v30x.h"
#include "net_tools/net_virtual_terminal.h"
#ifdef ONPS_ENABLE_NVTCMD_TELNET
#include "net_tools/telnet_client.h"
#endif //* #ifdef ONPS_ENABLE_NVTCMD_TELNET
#include "net_tools/telnet.h"
#define SYMBOL_GLOBALS
#include "telnet/nvt_cmd.h"
#undef SYMBOL_GLOBALS

//* 在这里定义你自己要添加的nvt指令
//* ===================================================================================
#ifdef ONPS_ENABLE_NVTCMD_TELNET
static INT telnet(CHAR argc, CHAR* argv[], ULONGLONG ullNvtHandle);
#endif //* #ifdef ONPS_ENABLE_NVTCMD_TELNET

#if defined(ONPS_ENABLE_NETTOOLS_PING) && defined(ONPS_ENABLE_NVTCMD_PING)
#include "net_tools/ping.h"
static INT nvt_cmd_ping(CHAR argc, CHAR* argv[], ULONGLONG ullNvtHandle);
#endif //* #if defined(ONPS_ENABLE_NETTOOLS_PING) && defined(ONPS_ENABLE_NVTCMD_PING)

#ifdef ONPS_ENABLE_NVTCMD_RESET
static INT reset(CHAR argc, CHAR* argv[], ULONGLONG ullNvtHandle);
#endif //* #ifdef ONPS_ENABLE_NVTCMD_RESET

//* NVT自定义命令数组
static const ST_NVTCMD l_staNvtCmd[] = {
#ifdef ONPS_ENABLE_NVTCMD_TELNET
    { telnet, "telnet", "used to log in to remote telnet host.\r\n" },
#endif //* #ifdef ONPS_ENABLE_NVTCMD_TELNET

#if defined(ONPS_ENABLE_NETTOOLS_PING) && defined(ONPS_ENABLE_NVTCMD_PING)
    { nvt_cmd_ping, "ping", "A lightweight ping testing tool that supports IPv4 and IPv6 address probing.\r\n" },
#endif //* #if defined(ONPS_ENABLE_NETTOOLS_PING) && defined(ONPS_ENABLE_NVTCMD_PING)

#ifdef ONPS_ENABLE_NVTCMD_RESET
    { reset, "reset", "system reset.\r\n" },
#endif //* #ifdef ONPS_ENABLE_NVTCMD_RESET

    {NULL, "", ""} //* 注意这个不要删除，当所有nvt命令被用户禁止时其被用于避免编译器报错
};

static ST_NVTCMD_NODE l_staNvtCmdNode[sizeof(l_staNvtCmd) / sizeof(ST_NVTCMD)];
//* ===================================================================================

#define NVTCMD_ONLY_ONE_INSTANCE "An instance of this command is already running, only one instance can run at a time. Please try again later.\r\n"

#define THNVTCMD_STK_SIZE 1360
static rt_thread_t l_tid_cmd = NULL;
void nvt_cmd_register(void)
{
  UCHAR i;
  for (i = 0; i < sizeof(l_staNvtCmd) / sizeof(ST_NVTCMD); i++)
  {
    if(l_staNvtCmd[i].pfun_cmd_entry)
      nvt_cmd_add(&l_staNvtCmdNode[i], &l_staNvtCmd[i]);
  }
}

//* 杀死当前正在执行的指令，该函数用于以任务方式启动的指令，当用户退出登录或者长时间没有任何操作，需要主动结束nvt结束运行时，
//* 此时如果存在正在执行的指令，nvt会先主动通知其结束运行，如果规定时间内其依然未结束运行，nvt会调用这个函数强制其结束运行释放占
//* 用的线程/任务资源
void nvt_cmd_kill(void)
{
  if(l_tid_cmd)
  {
    rt_thread_delete(l_tid_cmd);
    l_tid_cmd = NULL;
  }
}

//* 以任务方式运行地命令在任务结束时应显式地告知其已结束运行，因为协议栈运行的目标系统属于资源受限系统，凡是以任务运行的nvt命令在
//* 同一时刻只允许运行一个实例，这个函数确保nvt能够安全运行下一个任务实例
void nvt_cmd_thread_end(void)
{
  l_tid_cmd = NULL;
}

static void nvt_cmd_thread_start(ULONGLONG ullNvtHandle, void (*pfunThreadEntry)(void *), void *pvParam, CHAR *pbIsCpyEnd, const CHAR *pszThreadName)
{
  os_critical_init();

  os_enter_critical();
  if(NULL == l_tid_cmd)
  {
    l_tid_cmd = rt_thread_create(pszThreadName, pfunThreadEntry, pvParam, THNVTCMD_STK_SIZE, THNVTCMD_PRIO, 5);
    os_exit_critical();

    if(l_tid_cmd != RT_NULL)
    {
      if(RT_EOK == rt_thread_startup(l_tid_cmd))
      {
        while (!(*pbIsCpyEnd))
          os_sleep_ms(10);

        return;
      }
    }

    nvt_output(ullNvtHandle, "Failed to start command in thread mode.\r\n", sizeof("Failed to start command in thread mode.\r\n") - 1);
    nvt_cmd_exec_end(ullNvtHandle);
  }
  else
  {
    os_exit_critical();

    nvt_output(ullNvtHandle, NVTCMD_ONLY_ONE_INSTANCE, sizeof(NVTCMD_ONLY_ONE_INSTANCE) - 1);
    nvt_cmd_exec_end(ullNvtHandle);
  }
}

#ifdef ONPS_ENABLE_NVTCMD_TELNET
#define NVTHELP_TELNET_USAGE       "Please enter the telnet server address, usage as follows:\r\n \033[01;37mtelnet xxx.xxx.xxx.xxx [port]\033[0m\r\n"
#define NVTHELP_TELNET_LOGIN_LOCAL "Due to resource constraints, the telnet command is prohibited from logging in to its own server.\r\n"
static INT telnet(CHAR argc, CHAR* argv[], ULONGLONG ullNvtHandle)
{
  ST_TELCLT_STARTARGS stArgs;

  if (argc != 2 && argc != 3)
  {
    nvt_output(ullNvtHandle, NVTHELP_TELNET_USAGE, sizeof(NVTHELP_TELNET_USAGE) - 1);
    nvt_cmd_exec_end(ullNvtHandle);
    return -1;
  }

  if (is_local_ip(inet_addr(argv[1])))
  {
    nvt_output(ullNvtHandle, NVTHELP_TELNET_LOGIN_LOCAL, sizeof(NVTHELP_TELNET_LOGIN_LOCAL) - 1);
    nvt_cmd_exec_end(ullNvtHandle);
    return -1;
  }

  stArgs.bIsCpyEnd = FALSE;
  stArgs.ullNvtHandle = ullNvtHandle;
  stArgs.stSrvAddr.saddr_ipv4 = inet_addr(argv[1]);
  if (argc == 3)
    stArgs.stSrvAddr.usPort = atoi(argv[2]);
  else
    stArgs.stSrvAddr.usPort = 23;

  nvt_cmd_thread_start(ullNvtHandle, telnet_clt_entry, &stArgs, &stArgs.bIsCpyEnd, "telnet_clt_entry");

  return 0;
}
#endif //* #ifdef ONPS_ENABLE_NVTCMD_TELNET

#ifdef ONPS_ENABLE_NVTCMD_RESET
static INT reset(CHAR argc, CHAR* argv[], ULONGLONG ullNvtHandle)
{
  nvt_output(ullNvtHandle, "The system will be reset ...", sizeof("The system will be reset ...") - 1);
  os_sleep_secs(3);
  nvt_close(ullNvtHandle);

  NVIC_SystemReset();
  return 0;
}
#endif //* #ifdef ONPS_ENABLE_NVTCMD_RESET

#if defined(ONPS_ENABLE_NETTOOLS_PING) && defined(ONPS_ENABLE_NVTCMD_PING)
#ifdef ONPS_ENABLE_IPV6
#define NVTHELP_PING_USAGE "Usage as follows:\r\n  \033[01;37mping [4] xxx.xxx.xxx.xxx\033[0m\r\n  \033[01;37mping 6 xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx\033[0m\r\n"
#else //* #ifdef ONPS_ENABLE_IPV6
#define NVTHELP_PING_USAGE "Usage as follows:\r\n  ping xxx.xxx.xxx.xxx\r\n"
#endif //* #ifdef ONPS_ENABLE_IPV6
static INT nvt_cmd_ping(CHAR argc, CHAR* argv[], ULONGLONG ullNvtHandle)
{
  ST_PING_STARTARGS stArgs;

#ifdef ONPS_ENABLE_IPV6
  if (argc != 2 && argc != 3)
#else
  if (argc != 2)
#endif
    goto __lblHelp;
  else
  {
#ifdef ONPS_ENABLE_IPV6
    if (argc == 3)
    {
      if (strlen(argv[1]) == 1)
      {
        if('4' == argv[1][0])
          stArgs.nFamily = AF_INET;
        else if('6' == argv[1][0])
          stArgs.nFamily = AF_INET6;
        else
          goto __lblHelp;

        snprintf(stArgs.szDstIp, sizeof(stArgs.szDstIp), "%s", argv[2]);
      }
      else
        goto __lblHelp;
    }
    else
    {
      stArgs.nFamily = AF_INET;
      snprintf(stArgs.szDstIp, sizeof(stArgs.szDstIp), "%s", argv[1]);
    }
#else
    stArgs.nFamily = AF_INET;
    snprintf(stArgs.szDstIp, sizeof(stArgs.szDstIp), "%s", argv[1]);
#endif
  }

  stArgs.bIsCpyEnd = FALSE;
  stArgs.ullNvtHandle = ullNvtHandle;


  //* 启动ping任务
  nvt_cmd_thread_start(ullNvtHandle, nvt_cmd_ping_entry, &stArgs, &stArgs.bIsCpyEnd, "nvt_cmd_ping_entry");

  return 0;

__lblHelp:
  nvt_output(ullNvtHandle, NVTHELP_PING_USAGE, sizeof(NVTHELP_PING_USAGE) - 1);
  nvt_cmd_exec_end(ullNvtHandle);
  return -1;
}
#endif //* #if defined(ONPS_ENABLE_NETTOOLS_PING) && defined(ONPS_ENABLE_NVTCMD_PING)
#endif //* #ifdef ONPS_ENABLE_NETTOOLS_TELNETSRV
