#include "port/datatype.h"
#include "onps_errors.h"
#include "port/sys_config.h"
#include "port/os_datatype.h"
#include "one_shot_timer.h"
#include "board.h"
#include "onps_utils.h"
#include "protocols.h"
#include "onps_input.h"
#include "ip/tcp.h"

#ifdef ONPS_ENABLE_PPP
#include "ppp/ppp.h"
#endif

#define SYMBOL_GLOBALS
#include "port/os_adapter.h"
#undef SYMBOL_GLOBALS

#ifdef ONPS_ENABLE_ETHERNET
//* 系统存在几个网卡局添加几个网卡名称(ETHERNET_NUM宏定义的数量与名称数量应一致)，注意网卡名称不能重名
const CHAR *or_pszaEthName[ONPS_ETHERNET_NUM] = {
    "eth0"
};
#endif

//* 用户自定义变量声明区
//* ===============================================================================================
#define THOSTIMERCOUNT_STK_SIZE  1024
#define THOSTIMERCOUNT_TIMESLICE 10

#ifdef ONPS_ENABLE_SACK
#define THTCPHANDLER_STK_SIZE  1024
#define THTCPHANDLER_TIMESLICE 5
#endif

//* ===============================================================================================

//* 协议栈内部工作线程列表
const static STCB_PSTACKTHREAD lr_stcbaPStackThread[] = {
    { thread_one_shot_timer_count, (void *)0, "thread_one_shot_timer_count", THOSTIMERCOUNT_STK_SIZE, THOSTIMERCOUNT_PRIO, THOSTIMERCOUNT_TIMESLICE },

#ifdef ONPS_ENABLE_SACK
  { thread_tcp_handler, (void *)0, "thread_tcp_handler", THTCPHANDLER_STK_SIZE, THTCPHANDLER_PRIO, THTCPHANDLER_TIMESLICE },
#endif
};

//* 当前线程休眠指定的秒数，参数unSecs指定要休眠的秒数
void os_sleep_secs(UINT unSecs)
{
    rt_thread_mdelay(unSecs * 1000);
}

//* 当前线程休眠指定的毫秒数，单位：毫秒
void os_sleep_ms(UINT unMSecs)
{
    rt_thread_mdelay(unMSecs);
}

//* 获取系统启动以来已运行的秒数（从0开始）
UINT os_get_system_secs(void)
{
    return rt_tick_get() / RT_TICK_PER_SECOND;
}

//* 获取系统启动以来已运行的毫秒数（从0开始）
UINT os_get_system_msecs(void)
{
    return rt_tick_get();
}

void os_thread_onpstack_start(void *pvParam)
{
    //* 建立工作线程
    rt_thread_t tid;
    INT i;
    for (i = 0; i < sizeof(lr_stcbaPStackThread) / sizeof(STCB_PSTACKTHREAD); i++)
    {
        tid = rt_thread_create(lr_stcbaPStackThread[i].pszThreadName, lr_stcbaPStackThread[i].pfunThread, RT_NULL, lr_stcbaPStackThread[i].unStackSize, lr_stcbaPStackThread[i].ubPrio, lr_stcbaPStackThread[i].unTimeSlice);
        if(tid != RT_NULL)
            rt_thread_startup(tid);
    }
}

HMUTEX os_thread_mutex_init(void)
{
    HMUTEX hMutex = rt_mutex_create("rt-mutex", RT_IPC_FLAG_FIFO);
    if(RT_NULL != hMutex)
        return hMutex;

    return INVALID_HMUTEX; //* 初始失败要返回一个无效句柄
}

void os_thread_mutex_lock(HMUTEX hMutex)
{
    rt_mutex_take(hMutex, RT_WAITING_FOREVER);
}

void os_thread_mutex_unlock(HMUTEX hMutex)
{
    rt_mutex_release(hMutex);
}

void os_thread_mutex_uninit(HMUTEX hMutex)
{
    rt_mutex_delete(hMutex);
}

HSEM os_thread_sem_init(UINT unInitVal, UINT unCount)
{
    HSEM hSem = rt_sem_create("rt-dsem", unInitVal, RT_IPC_FLAG_FIFO);
    if(RT_NULL != hSem)
        return hSem;

    return INVALID_HSEM; //* 初始失败要返回一个无效句柄
}

void os_thread_sem_post(HSEM hSem)
{
    rt_sem_release(hSem);
}

INT os_thread_sem_pend(HSEM hSem, INT nWaitSecs)
{
    if(nWaitSecs)
        nWaitSecs = nWaitSecs * RT_TICK_PER_SECOND;
    else
        nWaitSecs = RT_WAITING_FOREVER;

    rt_err_t lRtnVal = rt_sem_take(hSem, nWaitSecs);
    if(RT_EOK == lRtnVal)
        return 0;
    else if(-RT_ETIMEOUT == lRtnVal)
        return 1;
    else
        return -1;
}

void os_thread_sem_uninit(HSEM hSem)
{
    rt_sem_delete(hSem);
}

#ifdef ONPS_ENABLE_PPP
HTTY os_open_tty(const CHAR *pszTTYName)
{
    pszTTYName = pszTTYName; //* 避免编译器警告
    return 0;
}

void os_close_tty(HTTY hTTY)
{
    hTTY = hTTY; //* 避免编译器警告
}

INT os_tty_send(HTTY hTTY, UCHAR *pubData, INT nDataLen)
{
    hTTY = hTTY; //* 避免编译器警告
    return 0;
}

INT os_tty_recv(HTTY hTTY, UCHAR *pubRcvBuf, INT nRcvBufLen, INT nWaitSecs)
{
    hTTY = hTTY; //* 避免编译器警告
    return 0;
}

void os_modem_reset(HTTY hTTY)
{
}
#endif

#ifndef ONPS_ENABLE_BUDDY_MMU
void *buddy_alloc(UINT unSize, EN_ONPSERR *penErr)
{
  void *pvMem = rt_malloc(unSize);
  if(!pvMem && penErr)
    *penErr = ERRNOFREEMEM;

  return pvMem;
}
#endif
