/********************************** (C) COPYRIGHT *******************************
* File Name          : sample.c
* Author             : MXH
* Version            : V1.0.0
* Date               : 2023/09/14
* Description        : sample body.
*********************************************************************************/

#include <rtthread.h>
#include "onps.h"
#include "net_tools/ping.h"
#include "net_tools/dns.h"
#include "net_tools/sntp.h"
#include "eth_driver.h"
#ifdef ONPS_ENABLE_NETTOOLS_TELNETSRV
#include "net_tools/telnet_srv.h"
#endif //* #ifdef ONPS_ENABLE_NETTOOLS_TELNETSRV
#include <samples/tcp_commu/tcp_client.h>

#define RTCPSRV_PORT  6410

#define TCPCLT_TEST_EN 1 //* tcp通讯客户端测试使能，复位则禁止tcp通讯测试
#if TCPCLT_TEST_EN
#define TCPCLT0_EN 1 //* 第一个TCP通讯客户端任务使能，复位则禁止启动第一个tcp通讯测试任务
#define TCPCLT1_EN 0 //* 第二个TCP通讯客户端任务使能，复位则禁止启动第二个tcp通讯测试任务
#endif //* #if TCPCLT_TEST_EN

#if TCPCLT_TEST_EN && (TCPCLT0_EN || TCPCLT1_EN)
//* tcp client任务用到的收发缓冲区定义
static UCHAR l_ubaRcvBuf[RCV_BUF_SIZE];
static UCHAR l_ubaSndBuf[sizeof(ST_COMMUPKT_HDR) + PKT_DATA_LEN_MAX];
static UCHAR l_ubaRcvBuf1[RCV_BUF_SIZE];
static UCHAR l_ubaSndBuf1[sizeof(ST_COMMUPKT_HDR) + PKT_DATA_LEN_MAX];
static const ST_CLT_ADDR l_staRTcpSrvAddr[2] = {
#if defined(ONPS_ENABLE_IPV6) && defined(TCP_COMMU_TEST_BY_IPV6)
#if TCPCLT0_EN
  {0, AF_INET6, "2408:8215:41b:30b2:ae6a:a907:c8a7:42ab", RTCPSRV_PORT, l_ubaRcvBuf, l_ubaSndBuf},
#endif //* #if TCPCLT0_EN

#if TCPCLT1_EN
  {1, AF_INET, "47.92.239.107", RTCPSRV_PORT, l_ubaRcvBuf1, l_ubaSndBuf1},
#endif //* #if TCPCLT1_EN
#else
#if TCPCLT0_EN
  {0, AF_INET, "192.168.2.203", RTCPSRV_PORT, l_ubaRcvBuf, l_ubaSndBuf},
#endif //* #if TCPCLT0_EN

#if TCPCLT1_EN
  {1, AF_INET, "47.92.239.107", RTCPSRV_PORT, l_ubaRcvBuf1, l_ubaSndBuf1},
#endif //* #if TCPCLT1_EN
#endif //* #if defined(ONPS_ENABLE_IPV6) && defined(TCP_COMMU_TEST_BY_IPV6)
};
#endif //* #if TCPCLT_TEST_EN && (TCPCLT0_EN || TCPCLT1_EN)

static UCHAR l_ubaUdpBuf[256];
#define RUDPSRV_IP     "47.92.239.107"/*"192.168.3.68"*/
#define RUDPSRV_PORT  6416
#define LUDPSRV_PORT  6415          //* 本地udp服务器的端口

#if defined(ONPS_ENABLE_IPV6) && defined(TCP_COMMU_TEST_BY_IPV6)
#define THTCPCLT_STK_SIZE     1528 //* tcp客户端栈大小
#else
#define THTCPCLT_STK_SIZE     1180 //* tcp客户端栈大小
#endif // #if defined(ONPS_ENABLE_IPV6) && defined(TCP_COMMU_TEST_BY_IPV6)

#ifdef ONPS_ENABLE_NETTOOLS_TELNETSRV
#define THTELNETSRV_STK_SIZE   1280
#define THTELNETSRV_TIMESLICE  10
#endif //* #ifdef ONPS_ENABLE_NETTOOLS_TELNETSRV

uint32_t HalGetElapsedMSecs(void)
{
    return os_get_system_msecs();
}

uint32_t HalGetElapsedSecs(void)
{
    return os_get_system_secs();
}

#ifdef ONPS_ENABLE_SACK
static void THTcpClt(void *pvUserParam)
{
  UINT unLastOptSecs;
  EN_ONPSERR enErr;
  PST_CLT_ADDR pstCltAddr = (PST_CLT_ADDR)pvUserParam;

  CHAR bParsingState;
  UINT unWriteIdx, unReadIdx, unPktStartIdx, unTimestampToAck, unSeqNum = 0;
  SOCKET hSocket = INVALID_SOCKET;
  CHAR bIsConnected = FALSE;

  INT i;
  for(i=0; i < PKT_DATA_LEN_MAX - 1; i++)
    pstCltAddr->pubSndBuf[sizeof(ST_COMMUPKT_HDR) + i] = (UCHAR)i;

  while(TRUE)
  {

    //* 分配一个socket
    if(INVALID_SOCKET == hSocket)
    {
      bIsConnected = FALSE;

      hSocket = socket(pstCltAddr->nFamily, SOCK_STREAM, 0, &enErr);
      if(INVALID_SOCKET == hSocket)
      {
        if(enErr != ERRNOFREEMEM)
        {
          printf("<%d>socket() failed, %s\r\n", pstCltAddr->nThIdx, onps_error(enErr));
        }
        os_sleep_ms(1000);
        continue;
      }

      unLastOptSecs = 0;
    }

    //* 连接服务器
    if(!bIsConnected)
    {
      if(!unLastOptSecs || HalGetElapsedSecs() - unLastOptSecs > 15)
      {
        unLastOptSecs = HalGetElapsedSecs();
        if(connect_tcp_server(pstCltAddr->nThIdx, hSocket, pstCltAddr->pszAddr, pstCltAddr->usPort, 0, 10))
        {
          unSeqNum = 0;
          bParsingState = 0;
          unWriteIdx = unReadIdx = unPktStartIdx = unTimestampToAck = 0;
          bIsConnected = TRUE;
        }
        else
        {
          os_sleep_ms(1000);
          continue;
        }
      }
      else
      {
        os_sleep_ms(1000);
        continue;
      }
    }

    //* 接收
    handle_read(pstCltAddr->nThIdx, hSocket, pstCltAddr->pubRcvBuf, &bParsingState, &unWriteIdx, &unReadIdx, &unPktStartIdx, &unTimestampToAck);

    //* 上传数据
    tcp_commu_packet_ready(pstCltAddr->nThIdx, pstCltAddr->pubSndBuf, 900, unSeqNum++, &unTimestampToAck);
    while(!tcp_commu_packet_send(pstCltAddr->nThIdx, hSocket, pstCltAddr->pubSndBuf))
    {
      onps_get_last_error(hSocket, &enErr);

      printf("<%d>tcp_commu_packet_send() failed, %s\r\n", pstCltAddr->nThIdx, onps_error(enErr));

      if(ERRTCPNOTCONNECTED == enErr || ERRTCPCONNRESET == enErr)
      {
        close(hSocket);
        hSocket = INVALID_SOCKET;
        break;
      }

      os_sleep_ms(1000);
    }
  }
}
#else
static void THTcpClt(void *pvUserParam)
{
  UINT unLastLedRunToggleSecs = 0, unLastOptSecs;
  EN_ONPSERR enErr;
  BOOL blIsLedRunOn = FALSE;
  PST_CLT_ADDR pstCltAddr = (PST_CLT_ADDR)pvUserParam;

  INT i;
  for(i=0; i < PKT_DATA_LEN_MAX - 1; i++)
    pstCltAddr->pubSndBuf[sizeof(ST_COMMUPKT_HDR) + i] = (UCHAR)i;


  //* 通讯任务开始工作：1）上传数据；2)等待应答；3)接收控制指令；4)回馈控制指令应答报文
  CHAR bParsingState;
  UINT unWriteIdx, unReadIdx, unPktStartIdx, unTimestampToAck, unSeqNum = 0;
  SOCKET hSocket = INVALID_SOCKET;
  CHAR bIsConnected = FALSE;
  CHAR bSendingState = 0;
  CHAR bTryNum = 0;
  CHAR szNowTime[20];

  while(TRUE)
  {
    //* 运行指示灯
        if(HalGetElapsedSecs() - unLastLedRunToggleSecs)
        {
            unLastLedRunToggleSecs = HalGetElapsedSecs();
        }


    //* 分配一个socket
    if(INVALID_SOCKET == hSocket)
    {
      bIsConnected = FALSE;

      hSocket = socket(pstCltAddr->nFamily, SOCK_STREAM, 0, &enErr);
      if(INVALID_SOCKET == hSocket)
      {
        if(enErr != ERRNOFREEMEM)
        {
          printf("<%d>socket() failed, %s\r\n", pstCltAddr->nThIdx, onps_error(enErr));
        }
        os_sleep_ms(1000);
        continue;
      }

      unLastOptSecs = 0;
    }

    //* 连接服务器
    if(!bIsConnected)
    {
      if(!unLastOptSecs || HalGetElapsedSecs() - unLastOptSecs > 15)
      {
        unLastOptSecs = HalGetElapsedSecs();
        if(connect_tcp_server(pstCltAddr->nThIdx, hSocket, pstCltAddr->pszAddr, pstCltAddr->usPort, 0, 10))
        {
          unSeqNum = 0;
          bParsingState = 0;
          bSendingState = 0; //* 重连后重新开启发送
          bTryNum = 0;
          unWriteIdx = unReadIdx = unPktStartIdx = unTimestampToAck = 0;
          bIsConnected = TRUE;
        }
        else
        {
          os_sleep_ms(1000);
          continue;
        }
      }
      else
      {
        os_sleep_ms(1000);
        continue;
      }
    }

    //* 接收
    handle_read(pstCltAddr->nThIdx, hSocket, pstCltAddr->pubRcvBuf, &bParsingState, &unWriteIdx, &unReadIdx, &unPktStartIdx, &unTimestampToAck);

    //* 上传数据
    if(bSendingState == 2)
    {
      if(unTimestampToAck)
      {
        //* 等待应答超时，则重新发送
        if(HalGetElapsedSecs() - unLastOptSecs > 6)
        {
          printf("<#%d#err>recv ack timeout\r\n", pstCltAddr->nThIdx);

          bSendingState = 1;
        }
      }
      else
      {
        bTryNum = 0;
        bSendingState = 0; //* 发送下一个
      }
    }

    if(!bSendingState)
    {
      //if(unSeqNum > 10000)
      //{
      //  while(TRUE)
      //    rt_thread_mdelay(1000);
      //}

      tcp_commu_packet_ready(pstCltAddr->nThIdx, pstCltAddr->pubSndBuf, 900, unSeqNum++, &unTimestampToAck);
      bSendingState = 1;
    }

    if(bSendingState == 1)
    {
      if(tcp_commu_packet_send(pstCltAddr->nThIdx, hSocket, pstCltAddr->pubSndBuf))
      {
        unLastOptSecs = HalGetElapsedSecs();
        bSendingState = 2;

        bTryNum++;
        if(bTryNum > 2)
        {
          //* 已经超出重试次数，断开当前连接以重连服务器
          close(hSocket);
          hSocket = INVALID_SOCKET;
          unLastOptSecs = HalGetElapsedSecs();
        #ifdef RT_USING_RTC
          UnixTimeToLocalTime(time(NULL), (int8_t *)szNowTime);
        #endif
          printf("%d#%d#%s#>Failed to send, so disconnect the tcp connection with server %s:%d\r\n", pstCltAddr->nThIdx, unSeqNum - 1, szNowTime, pstCltAddr->pszAddr, pstCltAddr->usPort);

          os_sleep_ms(1000);
        }
      }
      else
      {
        onps_get_last_error(hSocket, &enErr);
        if(enErr != ERRTCPACKTIMEOUT)
        {
          bTryNum = 3;
          close(hSocket);
          hSocket = INVALID_SOCKET;
          unLastOptSecs = HalGetElapsedSecs();
        }
      }
    }
  }
}
#endif

int THMain(void)
{
    EN_ONPSERR enErr;
    SOCKET hSocket = INVALID_SOCKET;
    CHAR szNowTime[20];
    rt_thread_t tid;

    if(open_npstack_load(&enErr))
      {
        printf("The open source network protocol stack (ver %s) is loaded successfully. \r\n", ONPS_VER);
        if(EthInit())
          goto __lblStart;
        else
          printf("The open source network protocol stack failed to load, ethernet device initialization failed\r\n");
      }
      else
      {
        printf("The open source network protocol stack failed to load, %s\r\n", onps_error(enErr));
      }

    //* 协议栈加载失败则不再继续正常的工作流程，在这里死循环即可
    while(TRUE)
    {
        printf("Load Stack Failed.\r\n");
        rt_thread_mdelay(1000);
    }

__lblStart:
#ifdef ONPS_ENABLE_NETTOOLS_TELNETSRV
    tid = rt_thread_create("telnet_srv_entry", telnet_srv_entry, RT_NULL, THTELNETSRV_STK_SIZE, THTELNETSRV_PRIO, THTELNETSRV_TIMESLICE);
    if(tid != RT_NULL)
        rt_thread_startup(tid);
#endif

  //* 启动tcp客户端测试线程
#if TCPCLT_TEST_EN
#if TCPCLT0_EN
    tid = rt_thread_create("THTcpClt", THTcpClt, (void *)&l_staRTcpSrvAddr[0], THTCPCLT_STK_SIZE, THTCPCLT_PRIO, THTCPCLT_TIMESLICE);
    if(tid != RT_NULL)
        rt_thread_startup(tid);
    else
        printf("The creation of task THTcpClt failed\r\n");
#endif //* #if TCPCLT0_EN

#if TCPCLT1_EN
    rt_uint32_t unStackSize = THTCPCLT_STK_SIZE;
    #if defined(ONPS_ENABLE_IPV6) && defined(TCP_COMMU_TEST_BY_IPV6)
    unStackSize = THTCPCLT_STK_SIZE - 384;
    #endif
    tid = rt_thread_create("THTcpClt1", THTcpClt, (void *)&l_staRTcpSrvAddr[1], unStackSize, THTCPCLT_PRIO, THTCPCLT_TIMESLICE);
    if(tid != RT_NULL)
        rt_thread_startup(tid);
    else
        printf("The creation of task THTcpClt1 failed\r\n");
#endif //* #if TCPCLT1_EN
#endif //* #if TCPCLT_TEST_EN && (TCPCLT0_EN || TCPCLT1_EN)

    while(1)
    {
        //* 分配一个socket
        if(INVALID_SOCKET == hSocket)
        {
          hSocket = socket(AF_INET, SOCK_DGRAM, 0, &enErr);
          if(INVALID_SOCKET == hSocket)
          {
            if(enErr != ERRNOFREEMEM)
            {
        #ifdef RT_USING_RTC
              UnixTimeToLocalTime(time(NULL), (int8_t *)szNowTime);
        #endif
              printf("U#%s#>socket() failed, %s\r\n", szNowTime, onps_error(enErr));
            }
            os_sleep_ms(1000);
            continue;
          }
          else
          {
            if(connect(hSocket, RUDPSRV_IP, RUDPSRV_PORT, 0))
            {
        #ifdef RT_USING_RTC
              UnixTimeToLocalTime(time(NULL), (int8_t *)szNowTime);
        #endif
              printf("U#%s#>connect %s:%d failed, %s\r\n", szNowTime, RUDPSRV_IP, RUDPSRV_PORT, onps_get_last_error(hSocket, NULL));

              close(hSocket);
              hSocket = INVALID_SOCKET;

              os_sleep_ms(1000);
              continue;
            }

            if(!socket_set_rcv_timeout(hSocket, 1, &enErr))
            {
            #ifdef RT_USING_RTC
              UnixTimeToLocalTime(time(NULL), (int8_t *)szNowTime);
            #endif
              printf("U#%s#>socket_set_rcv_timeout() failed, %s\r\n", szNowTime, onps_error(enErr));

              close(hSocket);
              hSocket = INVALID_SOCKET;

              os_sleep_ms(1000);
              continue;
            }
          }
        }

        #ifdef RT_USING_RTC
        UnixTimeToLocalTime(time(NULL), (int8_t *)szNowTime);
        #endif
        sprintf((char *)l_ubaUdpBuf, "U#%s#>1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ", szNowTime);
        INT nSendDataLen = strlen((const char *)l_ubaUdpBuf);
        if(nSendDataLen != send(hSocket, l_ubaUdpBuf, nSendDataLen, 0))
        {
          printf("U#%s#>send failed, %s\r\n", szNowTime, onps_get_last_error(hSocket, NULL));
        }

        memset(l_ubaUdpBuf, 0, sizeof(l_ubaUdpBuf));
        INT nRcvBytes = recv(hSocket, l_ubaUdpBuf, sizeof(l_ubaUdpBuf));
        if(nRcvBytes > 0)
        {
        #ifdef RT_USING_RTC
          UnixTimeToLocalTime(time(NULL), (int8_t *)szNowTime);
        #endif
          printf("U#%s#>recv %d bytes, Data = <%s>\r\n", szNowTime, nRcvBytes, (const char *)l_ubaUdpBuf);
        }
        else
        {
          if(nRcvBytes < 0)
          {
        #ifdef RT_USING_RTC
            UnixTimeToLocalTime(time(NULL), (int8_t *)szNowTime);
        #endif
            printf("U#%s#>recv failed, %s\r\n", szNowTime, onps_get_last_error(hSocket, NULL));

            close(hSocket);
            hSocket = INVALID_SOCKET;
          }
        }
    }

    return RT_EOK;
}
INIT_APP_EXPORT(THMain);




