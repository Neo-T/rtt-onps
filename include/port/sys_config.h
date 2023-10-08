/* sys_config.h
 *
 * 系统配置头文件，用户可根据实际情况对协议栈进行裁剪、参数配置等工作
 *
 * Neo-T, 创建于2022.03.11 14:45
 * 版本: 1.0
 *
 */
#ifndef SYS_CONFIG_H
#define SYS_CONFIG_H

#include <rtthread.h>
#ifndef __RT_THREAD_H__

 //* 系统支持哪些功能模块由此配置
 //* ===============================================================================================
//#define ONPS_ENABLE_IPV6          //* 是否支持IPv6
#define ONPS_ENABLE_PRINTF          //* 是否支持调用printf()输出相关调试或系统信息
#ifdef ONPS_ENABLE_PRINTF
  #define printf rt_kprintf
  #define ONPS_DEBUG_LEVEL         2 //* 共5个调试级别：
                                //* 0 输出协议栈底层严重错误
                                //* 1 输出所有系统错误（包括0级错误）
                                //* 2 输出协议栈重要的配置、运行信息，同时包括0、1级信息
                                //* 3 输出网卡的原始通讯通讯报文（ppp为收发，ethnernet为发送），以及0、1、2级信息
                                //* 4 输出ethernet网卡接收的原始通讯报文，被协议栈丢弃的非法（校验和错误、通讯链路不存在等原因）通讯报文，以及0、1、2、3级信息（除ethernet发送的原始报文）
#endif

//#define ONPS_ENABLE_PPP             //* 是否支持ppp模块：1，支持；0，不支持，如果选择支持，则系统会将ppp模块代码加入到协议栈中
#ifdef ONPS_ENABLE_PPP
    #define ONPS_APN_DEFAULT            "4gnet"     //* 根据实际情况在这里设置缺省APN
    #define ONPS_AUTH_USER_DEFAULT      "card"      //* ppp认证缺省用户名
    #define ONPS_AUTH_PASSWORD_DEFAULT   "any_char"  //* ppp认证缺省口令

    #define ONPS_PPP_NETLINK_NUM         1   //* 最多支持几路ppp链路（系统存在几个modem这里就指定几就行）
    #define ONPS_ENABLE_ECHO            1   //* 对端是否支持echo链路探测
    #define ONPS_WAIT_ACK_TIMEOUT_NUM    5   //* 在这里指定连续几次接收不到对端的应答报文就进入协议栈故障处理流程（STACKFAULT），这意味着当前链路已经因严重故障终止了
#endif

#define ONPS_ENABLE_ETHERNET      //* 是否支持ethernet：1，支持；0，不支持
#ifdef ONPS_ENABLE_ETHERNET
    #define ONPS_ETHERNET_NUM    1   //* 要添加几个ethernet网卡（实际存在几个就添加几个）
    #define ONPS_ARPENTRY_NUM    16  //* arp条目缓存表的大小，只要不小于局域网内目标通讯节点的个数即可确保arp寻址次数为1，否则就会出现频繁寻址的可能，当然这也不会妨碍正常通讯逻辑，只不过这会降低通讯效率
    #ifdef ONPS_ENABLE_IPV6
      #define ONPS_IPV6TOMAC_ENTRY_NUM   8   //* Ipv6到以太网mac地址映射缓存表的大小（不要超过127），这个配置项指定缓存条目的数量，同样确保其不小于Ipv6通讯节点数量即可避免重复寻址的问题
      #define ONPS_IPV6_CFG_ADDR_NUM     4   //* 指定所有以太网卡能够自动配置的最大地址数量（不要超过128），超过这个数量将无法为网卡配置新的地址，如果目标网络环境地址数量确定建议将该值调整到合适的值以节省内存
      #define ONPS_IPV6_ROUTER_NUM       2   //* 指定所有以太网卡能够添加的路由器最大数量（最多8个），请根据目标网络实际情况调整该值以最大限度节省内存使用
    #endif

    #define ONPS_ENABLE_ETH_EXTRA_IP         //* 是否允许添加多个ip地址
    #ifdef ONPS_ENABLE_ETH_EXTRA_IP
      #define ONPS_ETH_EXTRA_IP_NUM  2 //* 允许添加的ip地址数量
    #endif
#endif

#define ONPS_NETIF_NUM   (1)    //* 系统支持的网卡数量
 //* ===============================================================================================

//* ip支持的上层协议相关配置项
//* ===============================================================================================
#define ONPS_ENABLE_SACK       //* 系统是否支持sack项，sack项需要协议栈建立发送队列，这个非常消耗内存，请慎重选择

#define ONPS_ICMPRCVBUF_SIZE 128     //* icmp发送echo请求报文时指定的接收缓冲区的缺省大小，注意，如果要发送较大的ping包就必须指定较大的接收缓冲区

#define ONPS_TCPRCVBUF_SIZE  2048    //* tcp层缺省的接收缓冲区大小，大小应是2^n次幂才能最大限度不浪费budyy模块分配的内存
#ifdef ONPS_ENABLE_SACK
#define ONPS_TCPSNDBUF_SIZE  4096    //* tcp层发送缓冲区大小，同接收缓冲区，大小应是2^n次幂才能最大限度不浪费budyy模块分配的内存
#endif

#define ONPS_TCPUDP_PORT_START   20000   //* TCP/UDP协议动态分配的起始端口号

#define ONPS_TCP_WINDOW_SCALE    0       //* 窗口扩大因子缺省值
#define ONPS_TCP_CONN_TIMEOUT    10      //* 缺省TCP连接超时时间
#define ONPS_TCP_ACK_TIMEOUT     3       //* 缺省TCP应答超时时间
#define ONPS_TCP_LINK_NUM_MAX    16      //* 系统支持最多建立多少路TCP链路（涵盖所有TCP客户端 + TCP服务器的并发连接数），超过这个数量将无法建立新的tcp链路
#define ONPS_TCP_ACK_DELAY_MSECS 100     //* 延迟多少毫秒发送ack报文，这个值最小40毫秒，最大200毫秒

#ifdef ONPS_ENABLE_ETHERNET
#define ONPS_TCPSRV_BACKLOG_NUM_MAX  8   //* tcp服务器支持的最大请求队列数量，任意时刻所有已开启的tcp服务器的请求连接队列数量之和应小于该值，否则将会出现拒绝连接的情况
#define ONPS_TCPSRV_NUM_MAX          1   //* 系统能够同时建立的tcp服务器数量
#define ONPS_TCPSRV_RECV_QUEUE_NUM   64  //* tcp服务器接收队列大小，所有已开启的tcp服务器共享该队列资源，如果单位时间内到达所有已开启tcp服务器的报文数量较大，应将该值调大
#endif //* #ifdef ONPS_ENABLE_ETHERNET

#define ONPS_UDP_LINK_NUM_MAX    4       //* 调用connect()函数连接对端udp服务器的最大数量（一旦调用connect()函数，收到的非服务器报文将被直接丢弃）
#define ONPS_SOCKET_NUM_MAX      16      //* 系统支持的最大SOCKET数量，如实际应用中超过这个数量则会导致用户层业务逻辑无法全部正常运行（icmp/tcp/udp业务均受此影响）
#define ONPS_IP_TTL_DEFAULT      64      //* 缺省TTL值
#define ONPS_ROUTE_ITEM_NUM      8       //* 系统路由表数量，如果使能telnet server则这个数量一定注意不要超过eeprom的容量
//* ===============================================================================================

//* 网络工具配置项
//* ===============================================================================================
#define ONPS_ENABLE_NETTOOLS_PING          //* ping
#define ONPS_ENABLE_NETTOOLS_DNS_CLIENT    //* dns查询客户端
#define ONPS_ENABLE_NETTOOLS_SNTP       //* sntp客户端
#define ONPS_ENABLE_NETTOOLS_TELNETSRV  //* 使能或禁止telnet服务端，其值必须为0或1（禁止/使能），因为其还被用于tcp服务器资源分配统计（ONPS_TCPSRV_NUM_MAX + ONPS_ENABLE_NETTOOLS_TELNETSRV）

#ifdef ONPS_ENABLE_NETTOOLS_TELNETSRV
    #define ONPS_ENABLE_NVTCMD_MEMUSAGE      //* 使能nvt命令：memusage
    #define ONPS_ENABLE_NVTCMD_NETIF         //* 使能nvt命令：netif
    #define ONPS_ENABLE_NVTCMD_IFIP          //* 使能nvt命令：ifip
    #define ONPS_ENABLE_NVTCMD_ROUTE         //* 使能nvt命令：route
    #define ONPS_ENABLE_NVTCMD_TELNET        //* 使能nvt命令：telnet

    #ifdef ONPS_ENABLE_NETTOOLS_SNTP
    #define ONPS_ENABLE_NVTCMD_NTP           //* 使能nvt命令：ntp，其必须先使能NETTOOLS_SNTP
    #endif //* #ifdef ONPS_ENABLE_NETTOOLS_SNTP

    #ifdef ONPS_ENABLE_NETTOOLS_DNS_CLIENT
    #define ONPS_ENABLE_NVTCMD_NSLOOKUP      //* 使能nvt命令：ntp，其必须先使能NETTOOLS_SNTP
    #endif //* #ifdef ONPS_ENABLE_NETTOOLS_DNS_CLIENT

    #ifdef ONPS_ENABLE_NETTOOLS_PING
    #define ONPS_ENABLE_NVTCMD_PING          //* 使能nvt命令：ping，使能ping命令时应同时使能NETTOOLS_PING
    #endif //* #ifdef ONPS_ENABLE_NETTOOLS_PING

    #define ONPS_ENABLE_NVTCMD_RESET         //* 使能nvt命令：reset

    #ifdef ONPS_ENABLE_NVTCMD_TELNET
        //* telnet客户端接收缓冲区大小，注意关闭TCP SACK选项时，设置的发送缓冲区大小一旦超过过tcp mtu（一般
        //* 为1460字节），就必须是在用户层限定单个发包的大小不能超过tcp mtu，否则会丢失数据
        #define ONPS_TELNETCLT_RCVBUF_SIZE 1024
    #endif //* #ifdef ONPS_ENABLE_NVTCMD_TELNET

    #define ONPS_NVTNUM_MAX          2   //* 指定nvt并发工作的数量，其实就是指定telnet服务器在同一时刻并发连接的数量，超过这个数值服务器拒绝连接
    #define ONPS_ENABLE_NVTCMDCACHE          //* 是否支持命令缓存，也就是通过“↑↓”切换曾经输入的指令
    #define ONPS_NVTCMDCACHE_SIZE    256 //* 指定指令缓存区的大小

    #ifdef ONPS_ENABLE_IPV6
        #define ONPS_ENABLE_TELNETSRV_IPV6 0 //* telnet服务器使能或禁止ipv6支持，与NETTOOLS_TELNETSRV同，其值必须为0或1（禁止/使能）
    #endif //* #ifdef ONPS_ENABLE_IPV6

#endif //* #ifdef ONPS_ENABLE_NETTOOLS_TELNETSRV
//* ===============================================================================================

//* 内存管理单元(mmu)相关配置项
//* ===============================================================================================
#define ONPS_BUF_LIST_NUM        64      //* 缓冲区链表的节点数，最大不能超过2的15次方（32768）

//#define ONPS_ENABLE_BUDDY_MMU

#ifdef ONPS_ENABLE_BUDDY_MMU
#define ONPS_BUDDY_PAGE_SIZE     0       //* 系统能够分配的最小页面大小，其值必须是2的整数次幂
#define ONPS_BUDDY_ARER_COUNT    8       //* 指定buddy算法管理的内存块数组单元数量
#define ONPS_BUDDY_MEM_SIZE      8192    //* buddy算法管理的内存总大小，其值由BUDDY_PAGE_SIZE、BUDDY_ARER_COUNT两个宏计算得到：
                                    //* 64 * (2 ^ (10 - 1))，即BUDDY_MEM_SIZE = ONPS_BUDDY_PAGE_SIZE * (2 ^ (ONPS_BUDDY_ARER_COUNT - 1))
                                    //* 之所以在此定义好要管理的内存大小，原因是buddy管理的内存其实就是一块提前分配好的静态存储
                                    //* 时期的字节型一维数组，以确保协议栈不占用宝贵的堆空间
#else
#include <rtdef.h>
extern void *rt_malloc(rt_size_t size);
extern void rt_free(void *rmem);
#define buddy_free(pvStart)     rt_free(pvStart)
#endif
//* ===============================================================================================

#endif //* #ifndef __RT_THREAD_H__

#define ONPS_NETIF_NUM (ONPS_PPP_NETLINK_NUM + ONPS_ETHERNET_NUM)

//* mian、timer、idle三个系统启动的任务优先级分别是12、4、10
#define THTCPSRV_PRIO       13  //* Tcp服务器线程优先级
#define THTCPCLT_PRIO       14  //* Tcp客户端线程优先级
#define THTCPCLT_TIMESLICE  5   //* 分配给tcp客户端的运行时间片

#ifdef ONPS_ENABLE_NETTOOLS_TELNETSRV
#define THTELNETSRV_PRIO  7
#define THNVT0_PRIO       8
#define THNVT1_PRIO       9
#define THNVTCMD_PRIO     11
#endif

#define THOSTIMERCOUNT_PRIO 6   //* onps栈定时器计数线程优先级
#ifdef ONPS_ENABLE_SACK
#define THTCPHANDLER_PRIO   5   //* onps栈tcp协议主处理线程
#endif
#define THETHIIRECV_PRIO    3   //* ethernet网卡接收线程优先级

#endif //* #ifndef SYS_CONFIG_H
