/* tcp_client.h
 *
 * tcp协议测试客户端，与之对应的服务器为协议栈源码之TcpServerForStackTesting工程（IDE：vs2015）
 *
 * Neo-T, 创建于2022.05.20 13:50
 * 版本: 1.0
 *
 */
#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#ifdef SYMBOL_GLOBALS
    #define TCP_CLIENT_EXT
#else
    #define TCP_CLIENT_EXT extern
#endif //* SYMBOL_GLOBALS

#define RCV_BUF_SIZE      1300  //* 接收缓冲区容量
#define PKT_DATA_LEN_MAX  1200  //* 报文携带的数据最大长度，凡是超过这个长度的报文都将被丢弃

#define PKT_FLAG    0xEE
typedef struct _ST_COMMUPKT_HDR_ {
  CHAR bFlag;
  CHAR bCmd;
  CHAR bLinkIdx;
  UINT unSeqNum;
  UINT unTimestamp;
  USHORT usDataLen;
  USHORT usChechsum;
} PACKED ST_COMMUPKT_HDR, *PST_COMMUPKT_HDR;

typedef struct _ST_COMMUPKT_ACK_ {
  ST_COMMUPKT_HDR stHdr;
  UINT unTimestamp;
  CHAR bLinkIdx;
  CHAR bTail;
} PACKED ST_COMMUPKT_ACK, *PST_COMMUPKT_ACK;

typedef struct _ST_CLT_ADDR_ {
  INT nThIdx;
  INT nFamily;
  CHAR *pszAddr;
  USHORT usPort;
  UCHAR *pubRcvBuf;
  UCHAR *pubSndBuf;
} ST_CLT_ADDR, *PST_CLT_ADDR;

TCP_CLIENT_EXT BOOL connect_tcp_server(INT nThIdx, SOCKET hSocket, const CHAR *pszSrvIP, USHORT usSrvPort, INT nRcvTimeout, INT nConnTimeout);
TCP_CLIENT_EXT void handle_read(INT nThIdx, SOCKET hSocket, UCHAR *pubRcvBuf, CHAR *pbParsingState, UINT *punWriteIdx, UINT *punReadIdx, UINT *punPktStartIdx, UINT *punTimestampToAck);
TCP_CLIENT_EXT void tcp_commu_packet_ready(INT nThIdx, UCHAR *pubSndBuf, USHORT usDataLen, UINT unSeqNum, UINT *punTimestampToAck);
TCP_CLIENT_EXT BOOL tcp_commu_packet_send(INT nThIdx, SOCKET hSocket, UCHAR *pubSndPacket);
void UnixTimeToLocalTime(time_t tUnixTimestamp, CHAR *pszDatetime);

#endif
