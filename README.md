# onps网络协议栈

## 简介

onps是一个开源且完全自主开发的国产网络协议栈，适用于资源受限的单片机系统，提供完整地ethernet/ppp/tcp/ip协议族实现，同时提供sntp、dns、ping等网络工具，支持以太网环境下dhcp动态ip地址申请，也支持动态及静态路由表。协议栈还封装实现了一个伯克利套接字（Berkeley sockets）层。

此仓库是针对 RT-Thread 标准版的适配。

## 依赖

- RT-Thread 3.1.0+
- RT_USING_HEAP (select)
- RT_USING_RTC (select)
- RT_USING_SERIAL(select)

## 示例仓库

目前移植到 RT-Thread 标准版的 port 如下表所示。

| 厂商 | 芯片或者系列 | Port    | 当前版本 |
| ---- | ------------ | ------- | -------- |
| WCH  | CH32V307     | 10M-PHY | v1.1.0   |

## 参考

源项目地址：[Neo-T/OpenNPStack: An open source network protocol stack (PPP/IP/TCP/UDP) for embedded systems with limited resources. (github.com)](https://github.com/Neo-T/OpenNPStack)

详细移植说明：[onps网络协议栈移植及使用说明v1.0](https://gitee.com/Neo-T/open-npstack/releases/download/v1.0.0.221017/onps网络协议栈移植及使用说明v1.0.7z)，[onps栈移植手册](https://gitee.com/Neo-T/open-npstack/raw/master/onps栈移植手册.pdf)

开发一般性指导文件：[onps栈API接口手册](https://gitee.com/Neo-T/open-npstack/raw/master/onps栈API接口手册.pdf)，[onps栈用户使用手册](https://gitee.com/Neo-T/open-npstack/raw/master/onps栈用户使用手册.pdf)

移植样例：

1. **STM32F407VET6平台** ： [RT-Thread移植样例](https://gitee.com/Neo-T/onps-rtthread) [ucos-ii移植样例](https://gitee.com/Neo-T/onps-ucosii)
2. 沁恒CH32V307平台 ： [鸿蒙LiteOS-M移植样例](https://gitee.com/Neo-T/Onps-WCH-CH32V307/tree/master/HarmonyOS/LiteOS_m) [Free-rtos移植样例](https://gitee.com/Neo-T/Onps-WCH-CH32V307/tree/master/FreeRTOS) [RT-Thread移植样例](https://gitee.com/Neo-T/Onps-WCH-CH32V307/tree/master/rt-thread)

## 社区支持

您可以随时访问[ **onps栈官方网站** ](http://www.onps.org.cn/)，获取协议栈研发进度、后续计划、最新版本等相关信息。
如您在使用过程中遇到任何问题或建议，您可以到 **[onps栈交流社区](http://neo.onps.org.cn/)** 提出您的建议或问题，新版本发布也会在交流社区第一时间通知。
您也可以加入QQ群进行在线技术交流：
[![qq交流群](https://github.com/Neo-T/OpenNPStack/raw/master/onps%E7%BD%91%E7%BB%9C%E5%8D%8F%E8%AE%AE%E6%A0%88%E6%8A%80%E6%9C%AF%E4%BA%A4%E6%B5%81%E7%BE%A4%E7%BE%A4%E4%BA%8C%E7%BB%B4%E7%A0%81.png)](https://github.com/Neo-T/OpenNPStack/blob/master/onps网络协议栈技术交流群群二维码.png)
