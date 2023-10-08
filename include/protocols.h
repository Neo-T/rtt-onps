/*
 * 版权属于onps栈开发团队，遵循Apache License 2.0开源许可协议
 *
 * 协议栈支持的通讯协议类型值
 *
 * Neo-T, 创建于2022.03.26 10:00
 *
 */
#ifndef PROTOCOLS_H
#define PROTOCOLS_H

//* 协议栈支持的通讯协议类型值，其用于业务逻辑实现
typedef enum {
    LCP = 0,
    PAP,
    CHAP,
    IPCP,
#ifdef ONPS_ENABLE_IPV6
    IPV6CP,
#endif
    IPV4,
#ifdef ONPS_ENABLE_IPV6
    IPV6,
#endif

    ICMP,
#ifdef ONPS_ENABLE_IPV6
    ICMPV6,
#endif
    ARP,
    TCP,
    UDP,
} EN_NPSPROTOCOL;

#endif
