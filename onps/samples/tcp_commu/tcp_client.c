#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include "onps.h"
#include "drivers/rtc.h"
#define SYMBOL_GLOBALS
#include <samples/tcp_commu/tcp_client.h>
#undef SYMBOL_GLOBALS

static const USHORT l_usaCRC16[256] =
{
    0x0000,  0xC0C1,  0xC181,  0x0140,  0xC301,  0x03C0,  0x0280,  0xC241,
    0xC601,  0x06C0,  0x0780,  0xC741,  0x0500,  0xC5C1,  0xC481,  0x0440,
    0xCC01,  0x0CC0,  0x0D80,  0xCD41,  0x0F00,  0xCFC1,  0xCE81,  0x0E40,
    0x0A00,  0xCAC1,  0xCB81,  0x0B40,  0xC901,  0x09C0,  0x0880,  0xC841,
    0xD801,  0x18C0,  0x1980,  0xD941,  0x1B00,  0xDBC1,  0xDA81,  0x1A40,
    0x1E00,  0xDEC1,  0xDF81,  0x1F40,  0xDD01,  0x1DC0,  0x1C80,  0xDC41,
    0x1400,  0xD4C1,  0xD581,  0x1540,  0xD701,  0x17C0,  0x1680,  0xD641,
    0xD201,  0x12C0,  0x1380,  0xD341,  0x1100,  0xD1C1,  0xD081,  0x1040,
    0xF001,  0x30C0,  0x3180,  0xF141,  0x3300,  0xF3C1,  0xF281,  0x3240,
    0x3600,  0xF6C1,  0xF781,  0x3740,  0xF501,  0x35C0,  0x3480,  0xF441,
    0x3C00,  0xFCC1,  0xFD81,  0x3D40,  0xFF01,  0x3FC0,  0x3E80,  0xFE41,
    0xFA01,  0x3AC0,  0x3B80,  0xFB41,  0x3900,  0xF9C1,  0xF881,  0x3840,
    0x2800,  0xE8C1,  0xE981,  0x2940,  0xEB01,  0x2BC0,  0x2A80,  0xEA41,
    0xEE01,  0x2EC0,  0x2F80,  0xEF41,  0x2D00,  0xEDC1,  0xEC81,  0x2C40,
    0xE401,  0x24C0,  0x2580,  0xE541,  0x2700,  0xE7C1,  0xE681,  0x2640,
    0x2200,  0xE2C1,  0xE381,  0x2340,  0xE101,  0x21C0,  0x2080,  0xE041,
    0xA001,  0x60C0,  0x6180,  0xA141,  0x6300,  0xA3C1,  0xA281,  0x6240,
    0x6600,  0xA6C1,  0xA781,  0x6740,  0xA501,  0x65C0,  0x6480,  0xA441,
    0x6C00,  0xACC1,  0xAD81,  0x6D40,  0xAF01,  0x6FC0,  0x6E80,  0xAE41,
    0xAA01,  0x6AC0,  0x6B80,  0xAB41,  0x6900,  0xA9C1,  0xA881,  0x6840,
    0x7800,  0xB8C1,  0xB981,  0x7940,  0xBB01,  0x7BC0,  0x7A80,  0xBA41,
    0xBE01,  0x7EC0,  0x7F80,  0xBF41,  0x7D00,  0xBDC1,  0xBC81,  0x7C40,
    0xB401,  0x74C0,  0x7580,  0xB541,  0x7700,  0xB7C1,  0xB681,  0x7640,
    0x7200,  0xB2C1,  0xB381,  0x7340,  0xB101,  0x71C0,  0x7080,  0xB041,
    0x5000,  0x90C1,  0x9181,  0x5140,  0x9301,  0x53C0,  0x5280,  0x9241,
    0x9601,  0x56C0,  0x5780,  0x9741,  0x5500,  0x95C1,  0x9481,  0x5440,
    0x9C01,  0x5CC0,  0x5D80,  0x9D41,  0x5F00,  0x9FC1,  0x9E81,  0x5E40,
    0x5A00,  0x9AC1,  0x9B81,  0x5B40,  0x9901,  0x59C0,  0x5880,  0x9841,
    0x8801,  0x48C0,  0x4980,  0x8941,  0x4B00,  0x8BC1,  0x8A81,  0x4A40,
    0x4E00,  0x8EC1,  0x8F81,  0x4F40,  0x8D01,  0x4DC0,  0x4C80,  0x8C41,
    0x4400,  0x84C1,  0x8581,  0x4540,  0x8701,  0x47C0,  0x4680,  0x8641,
    0x8201,  0x42C0,  0x4380,  0x8341,  0x4100,  0x81C1,  0x8081,  0x4040,
};

USHORT crc16(const UCHAR *pubCheckData, UINT unCheckLen, USHORT usInitVal)
{
    UINT i;
    USHORT usCRC = usInitVal;
    for (i = 0; i < unCheckLen; i++)
    {
        usCRC = (usCRC >> 8) ^ l_usaCRC16[(usCRC & 0x00FF) ^ *pubCheckData];
        pubCheckData++;
    }

    return usCRC;
}

BOOL connect_tcp_server(INT nThIdx, SOCKET hSocket, const CHAR *pszSrvIP, USHORT usSrvPort, INT nRcvTimeout, INT nConnTimeout)
{
  EN_ONPSERR enErr;

  if(!connect(hSocket, pszSrvIP, usSrvPort, nConnTimeout))
  {
    if(!socket_set_rcv_timeout(hSocket, nRcvTimeout, &enErr))
    {
      printf("socket_set_rcv_timeout() failed, %s\r\n", onps_error(enErr));
    }

#if TCP_COMMU_TEST_BY_IPV6
    printf("#%d#>connect [%s]:%d successfully!\r\n", nThIdx, pszSrvIP, usSrvPort);
#else
    printf("#%d#>connect %s:%d successfully!\r\n", nThIdx, pszSrvIP, usSrvPort);
#endif

    return TRUE;
  }
  else
  {
#if TCP_COMMU_TEST_BY_IPV6
    printf("#%d#>connect [%s]:%d failed, %s\r\n", nThIdx, pszSrvIP, usSrvPort, onps_get_last_error(hSocket, NULL));
#else
    printf("#%d#>connect %s:%d failed, %s\r\n", nThIdx, pszSrvIP, usSrvPort, onps_get_last_error(hSocket, NULL));
#endif

    return FALSE;
  }
}

void handle_read(INT nThIdx, SOCKET hSocket, UCHAR *pubRcvBuf, CHAR *pbParsingState, UINT *punWriteIdx, UINT *punReadIdx, UINT *punPktStartIdx, UINT *punTimestampToAck)
{
  UINT unRemainBytes = RCV_BUF_SIZE - *punWriteIdx;
  INT nRcvBytes = recv(hSocket, pubRcvBuf + *punWriteIdx, unRemainBytes);
  if (nRcvBytes > 0)
  {
    PST_COMMUPKT_HDR pstHdr;
    *punWriteIdx += (UINT)nRcvBytes;
    for (; *punReadIdx < *punWriteIdx; )
    {
      UCHAR ch = pubRcvBuf[*punReadIdx];
      switch(*pbParsingState)
      {
        case 0: //* 找到头部标志
          if (ch == PKT_FLAG)
          {
            *punPktStartIdx = *punReadIdx;
            *pbParsingState = 1;
          }
          else
          {
            *punReadIdx += 1;
            break;
          }

        case 1: //* 获取整个头部数据
          if (*punWriteIdx - *punPktStartIdx < sizeof(ST_COMMUPKT_HDR))
            return;
          else
          {
            pstHdr = (PST_COMMUPKT_HDR)(pubRcvBuf + *punPktStartIdx);
            if (pstHdr->usDataLen < PKT_DATA_LEN_MAX) //* 只有小于系统允许的最大数据长度才是合法报文
              *pbParsingState = 2;
            else
            {
              //* 不合法，重新查找
              *punReadIdx = *punPktStartIdx + 1;
              *pbParsingState = 0;
              break;
            }
          }

        case 2: //* 获取完整报文
          pstHdr = (PST_COMMUPKT_HDR)(pubRcvBuf + *punPktStartIdx);
          if (*punWriteIdx - *punPktStartIdx < sizeof(ST_COMMUPKT_HDR) + (UINT)pstHdr->usDataLen + 1)
            return;
          else
          {
            *punReadIdx = *punPktStartIdx + sizeof(ST_COMMUPKT_HDR) + (UINT)pstHdr->usDataLen;

            //* 尾部标识必须匹配
            ch = pubRcvBuf[*punReadIdx];
            if (ch == PKT_FLAG)
            {
              //* 判断校验和是否正确
              USHORT usPktChecksum = pstHdr->usChechsum;
              pstHdr->usChechsum = 0;
              USHORT usChecksum = crc16(pubRcvBuf + *punPktStartIdx + sizeof(CHAR), sizeof(ST_COMMUPKT_HDR) - sizeof(CHAR) + (UINT)pstHdr->usDataLen, 0xFFFF);
              if (usChecksum == usPktChecksum)
              {
                CHAR szPktTime[20];
                #ifdef RT_USING_RTC
                UnixTimeToLocalTime((time_t)pstHdr->unTimestamp + 8 * 3600, (int8_t *)szPktTime);
                #endif

                //* 收到对端下发的模拟控制指令
                if (pstHdr->bCmd == 1)
                {
                  UCHAR ubaSndBuf[sizeof(ST_COMMUPKT_ACK)];
                  PST_COMMUPKT_ACK pstAck = (PST_COMMUPKT_ACK)ubaSndBuf;
                  pstAck->stHdr.bFlag = (CHAR)PKT_FLAG;
                  pstAck->stHdr.bCmd = 0x01;
                  pstAck->stHdr.bLinkIdx = (CHAR)nThIdx;
                #ifdef RT_USING_RTC
                  pstAck->stHdr.unTimestamp = (UINT)time(NULL);
                #endif
                  pstAck->stHdr.usDataLen = sizeof(UINT) + sizeof(CHAR);
                  pstAck->stHdr.usChechsum = 0;
                  pstAck->unTimestamp = pstHdr->unTimestamp;
                  pstAck->bLinkIdx = pstHdr->bLinkIdx;
                  pstAck->bTail = (CHAR)PKT_FLAG;
                  pstAck->stHdr.usChechsum = crc16(&ubaSndBuf[sizeof(CHAR)], sizeof(ST_COMMUPKT_ACK) - 2 * sizeof(CHAR), 0xFFFF);

                #if 1
                  printf("%d#%s#>recved the control cmd packet, cmd = 0x%02X, LinkIdx = %d, data length is %d bytes\r\n", nThIdx, szPktTime, pstHdr->bCmd, pstHdr->bLinkIdx, pstHdr->usDataLen);
                #endif

                  send(hSocket, (UCHAR *)ubaSndBuf, sizeof(ST_COMMUPKT_ACK), 3);
                }
                else if (pstHdr->bCmd == 0)
                {
                  PST_COMMUPKT_ACK pstAck = (PST_COMMUPKT_ACK)pstHdr;
                  if (pstAck->unTimestamp == *punTimestampToAck && pstAck->bLinkIdx == (CHAR)nThIdx)
                  {
                    *punTimestampToAck = 0;

                  #if 0
                    printf("%d#%s#>recved acknowledge packet, AckedLinkIdx = %d, AckedTimestamp <", nThIdx, szPktTime, pstAck->bLinkIdx);
                #ifdef RT_USING_RTC
                    UnixTimeToLocalTime((time_t)pstAck->unTimestamp, (int8_t *)szPktTime);
                #endif
                    printf("%s>\r\n", szPktTime);
                  #endif
                  }
                }
                else;

                //* 搬运剩余的字节
                UINT unRemainBytes = *punWriteIdx - *punReadIdx - 1;
                if (unRemainBytes)
                  memmove(pubRcvBuf, pubRcvBuf + *punReadIdx + 1, unRemainBytes);
                *punWriteIdx = unRemainBytes;

                //* 开始截取下一个报文
                *punReadIdx = 0;
              }
              else
              {
                //* 不合法，从第一个标识后的字符开始重新查找
                *punReadIdx = *punPktStartIdx + 1;
              }
            }
            else
            {
              //* 不合法，从第一个标识后的字符开始重新查找
              *punReadIdx = *punPktStartIdx + 1;
            }
          }

          *pbParsingState = 0;
          break;
      }
    }
  }
}

void tcp_commu_packet_ready(INT nThIdx, UCHAR *pubSndBuf, USHORT usDataLen, UINT unSeqNum, UINT *punTimestampToAck)
{
  PST_COMMUPKT_HDR pstHdr = (PST_COMMUPKT_HDR)pubSndBuf;
  pstHdr->bFlag = (CHAR)PKT_FLAG;
  pstHdr->bCmd = 0x00;
  pstHdr->bLinkIdx = (CHAR)nThIdx;
  pstHdr->unSeqNum = unSeqNum;
#ifdef RT_USING_RTC
  pstHdr->unTimestamp = time(NULL);
#endif
  pstHdr->usDataLen = usDataLen;
  pstHdr->usChechsum = 0;
  pstHdr->usChechsum = crc16(pubSndBuf + sizeof(CHAR), sizeof(ST_COMMUPKT_HDR) - sizeof(CHAR) + usDataLen, 0xFFFF);
  pubSndBuf[sizeof(ST_COMMUPKT_HDR) + usDataLen] = PKT_FLAG;

  /*
  if(unSeqNum > 10)
  {
    BOOL blIsRunning = TRUE;
    while(blIsRunning)
      rt_thread_mdelay(1000);
  }
  */

  //* 更新要应答报文标识
  *punTimestampToAck = pstHdr->unTimestamp;
}

BOOL tcp_commu_packet_send(INT nThIdx, SOCKET hSocket, UCHAR *pubSndPacket)
{
  PST_COMMUPKT_HDR pstHdr = (PST_COMMUPKT_HDR)pubSndPacket;

  //* 日志输出
  CHAR szPktTime[20];
#ifdef RT_USING_RTC
  UnixTimeToLocalTime((time_t)pstHdr->unTimestamp, (int8_t *)szPktTime);
#endif
#if 0
  printf("%d#%s#>sent user's data to peer, the cmd is 0x%02X, the data length is %d bytes\r\n", nThIdx, szPktTime, pstHdr->bCmd, pstHdr->usDataLen);
#endif

  INT nPacketLen = sizeof(ST_COMMUPKT_HDR) + pstHdr->usDataLen + sizeof(CHAR);
  INT nSndBytes, nSndNum = 0;

#ifdef ONPS_ENABLE_SACK
  INT nHasSendBytes = 0;
  while(nHasSendBytes < nPacketLen)
  {
    nSndBytes = send(hSocket, (UCHAR *)pubSndPacket + nHasSendBytes, nPacketLen - nHasSendBytes, 0);
    if(nSndBytes < 0)
    {
      EN_ONPSERR enErr;
      const CHAR *pszErr = onps_get_last_error(hSocket, &enErr);

      printf("<err>%d#%s#>sent user's data to peer (%d), the cmd is 0x%02X, the data length is %d bytes, %04X\r\n", nThIdx, szPktTime, pstHdr->unSeqNum, pstHdr->bCmd, pstHdr->usDataLen, pstHdr->usChechsum);
      printf("<err>%d#%s#>sent %d bytes failed (try num is %d), %s, socket: %d\r\n", nThIdx, szPktTime, nPacketLen, nSndNum + 1, pszErr, hSocket);

      return FALSE;
    }
    else if(!nSndBytes)
    {
      os_sleep_ms(10);
    }
    else
    {
      nHasSendBytes += nSndBytes;
    }
  }

  return TRUE;
#else
__lblSend:
  if(nSndNum > 2)
    return FALSE;

  //* 发送数据
  UINT unStartMillSecs = HalGetElapsedMSecs();
  nSndBytes = send(hSocket, (UCHAR *)pubSndPacket, nPacketLen, 3);
  if(nSndBytes == nPacketLen)
    return TRUE;
  else
  {
    EN_ONPSERR enErr;
    const CHAR *pszErr = onps_get_last_error(hSocket, &enErr);

    printf("<err>%d#%s#>sent user's data to peer (%d), the cmd is 0x%02X, the data length is %d bytes, %04X, wait %d millsecs\r\n", nThIdx, szPktTime, pstHdr->unSeqNum, pstHdr->bCmd, pstHdr->usDataLen, pstHdr->usChechsum, HalGetElapsedMSecs() - unStartMillSecs);
    printf("<err>%d#%s#>sent %d bytes failed (try num is %d), %s, socket: %d\r\n", nThIdx, szPktTime, nPacketLen, nSndNum + 1, pszErr, hSocket);

    if(enErr != ERRTCPACKTIMEOUT)
      return FALSE;

    nSndNum++;
    goto __lblSend;
  }
#endif
}

void UnixTimeToLocalTime(time_t tUnixTimestamp, CHAR *pszDatetime)
{
  struct tm stTime;
  localtime_r((time_t*)&tUnixTimestamp, &stTime);

  sprintf((char *)pszDatetime, "%d-%02d-%02d %02d:%02d:%02d", stTime.tm_year + 1900, stTime.tm_mon + 1, stTime.tm_mday, stTime.tm_hour, stTime.tm_min, stTime.tm_sec);
}
