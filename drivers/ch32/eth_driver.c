/********************************** (C) COPYRIGHT *******************************
* File Name          : eth_driver.c
* Author             : Neo-T
* Version            : V1.0.0
* Date               : 2023/08/14
* Description        : eth program body.
*********************************************************************************/

#include "stdio.h"
#include "ch32v30x.h"
#include "ch32v30x_eth.h"
#include <string.h>
#include "onps.h"

#define SYMBOL_GLOBALS
#include "eth_driver.h"
#undef SYMBOL_GLOBALS

#define ETH_TXBUFNB     4 //* The number of descriptors sent by the MAC
#define ETH_RXBUFNB     4 //* Number of MAC received descriptors
#define ETH_RX_BUF_SIZE ETH_MAX_PACKET_SIZE //* buffer size for receive
#define ETH_TX_BUF_SIZE ETH_MAX_PACKET_SIZE //* buffer size for transmit

extern ETH_DMADESCTypeDef *DMATxDescToSet;
extern ETH_DMADESCTypeDef *DMARxDescToGet;

//* mac地址
static const UCHAR lr_ubaMacAddr[6] = { MAC_ADDR0_DEFAULT, MAC_ADDR1_DEFAULT, MAC_ADDR2_DEFAULT, MAC_ADDR3_DEFAULT, MAC_ADDR4_DEFAULT, MAC_ADDR5_DEFAULT };

__attribute__((__aligned__(4))) ETH_DMADESCTypeDef DMARxDscrTab[ETH_RXBUFNB]; //* MAC receive descriptor, 4-byte aligned
__attribute__((__aligned__(4))) ETH_DMADESCTypeDef DMATxDscrTab[ETH_TXBUFNB]; //* MAC send descriptor, 4-byte aligned

__attribute__((__aligned__(4))) UCHAR MACRxBuf[ETH_RXBUFNB*ETH_RX_BUF_SIZE]; //* MAC receive buffer, 4-byte aligned
__attribute__((__aligned__(4))) UCHAR MACTxBuf[ETH_TXBUFNB*ETH_TX_BUF_SIZE]; //* MAC send buffer, 4-byte aligned

//* 协议栈返回的netif结构
static PST_NETIF l_pstNetifEth = NULL;
static volatile BOOL l_blIsEthStartd = FALSE;
static HMUTEX l_hMtxEthSend = INVALID_HMUTEX;

//* ethernet网卡接收任务栈容量
#define THETHIIRECV_STK_SIZE  1280
#define THETHIIRECV_TIMESLICE 5
static void start_thread_ethernet_ii_recv(void *pvParam)
{
  rt_thread_t tid = rt_thread_create("thread_ethernet_ii_recv", thread_ethernet_ii_recv, pvParam, THETHIIRECV_STK_SIZE, THETHIIRECV_PRIO, THETHIIRECV_TIMESLICE);
  if(tid != RT_NULL)
    rt_thread_startup(tid);
  else
    printf("The creation of task thread_ethernet_ii_recv failed\r\n");
}

static void EthSetClock(void)
{
    RCC_PLL3Cmd(DISABLE);
    RCC_PREDIV2Config(RCC_PREDIV2_Div2);                /* HSE = 8M */
    RCC_PLL3Config(RCC_PLL3Mul_15);                     /* 4M*15 = 60MHz */
    RCC_PLL3Cmd(ENABLE);
    while(RESET == RCC_GetFlagStatus(RCC_FLAG_PLL3RDY));
}

static void EthRegInit(ETH_InitTypeDef *ETH_InitStruct)
{
    UINT tmpreg = 0;

    /*---------------------- Physical layer configuration -------------------*/
    /* Set the SMI interface clock, set as the main frequency divided by 42  */
    tmpreg = ETH->MACMIIAR;
    tmpreg &= MACMIIAR_CR_MASK;
    tmpreg |= (UINT)ETH_MACMIIAR_CR_Div42;
    ETH->MACMIIAR = (UINT)tmpreg;

    /*------------------------ MAC register configuration  ----------------------- --------------------*/
    tmpreg = ETH->MACCR;
    tmpreg &= MACCR_CLEAR_MASK;
    tmpreg |= (UINT)(ETH_InitStruct->ETH_AutoNegotiation |
                  ETH_InitStruct->ETH_Watchdog |
                  ETH_InitStruct->ETH_Jabber |
                  ETH_InitStruct->ETH_InterFrameGap |
                  ETH_InitStruct->ETH_CarrierSense |
                  ETH_InitStruct->ETH_Speed |
                  ETH_InitStruct->ETH_ReceiveOwn |
                  ETH_InitStruct->ETH_LoopbackMode |
                  ETH_InitStruct->ETH_Mode |
                  ETH_InitStruct->ETH_ChecksumOffload |
                  ETH_InitStruct->ETH_RetryTransmission |
                  ETH_InitStruct->ETH_AutomaticPadCRCStrip |
                  ETH_InitStruct->ETH_BackOffLimit |
                  ETH_InitStruct->ETH_DeferralCheck);
    /* Write MAC Control Register */
    ETH->MACCR = (UINT)tmpreg;
    ETH->MACCR |= ETH_Internal_Pull_Up_Res_Enable;
    ETH->MACFFR = (UINT)(ETH_InitStruct->ETH_ReceiveAll |
                          ETH_InitStruct->ETH_SourceAddrFilter |
                          ETH_InitStruct->ETH_PassControlFrames |
                          ETH_InitStruct->ETH_BroadcastFramesReception |
                          ETH_InitStruct->ETH_DestinationAddrFilter |
                          ETH_InitStruct->ETH_PromiscuousMode |
                          ETH_InitStruct->ETH_MulticastFramesFilter |
                          ETH_InitStruct->ETH_UnicastFramesFilter);
    /*--------------- ETHERNET MACHTHR and MACHTLR Configuration ---------------*/
    /* Write to ETHERNET MACHTHR */
    ETH->MACHTHR = (UINT)ETH_InitStruct->ETH_HashTableHigh;
    /* Write to ETHERNET MACHTLR */
    ETH->MACHTLR = (UINT)ETH_InitStruct->ETH_HashTableLow;
    /*----------------------- ETHERNET MACFCR Configuration --------------------*/
    /* Get the ETHERNET MACFCR value */
    tmpreg = ETH->MACFCR;
    /* Clear xx bits */
    tmpreg &= MACFCR_CLEAR_MASK;
    tmpreg |= (UINT)((ETH_InitStruct->ETH_PauseTime << 16) |
                     ETH_InitStruct->ETH_ZeroQuantaPause |
                     ETH_InitStruct->ETH_PauseLowThreshold |
                     ETH_InitStruct->ETH_UnicastPauseFrameDetect |
                     ETH_InitStruct->ETH_ReceiveFlowControl |
                     ETH_InitStruct->ETH_TransmitFlowControl);
    ETH->MACFCR = (UINT)tmpreg;

    ETH->MACVLANTR = (UINT)(ETH_InitStruct->ETH_VLANTagComparison |
                               ETH_InitStruct->ETH_VLANTagIdentifier);

    tmpreg = ETH->DMAOMR;
    tmpreg &= DMAOMR_CLEAR_MASK;
    tmpreg |= (UINT)(ETH_InitStruct->ETH_DropTCPIPChecksumErrorFrame |
                    ETH_InitStruct->ETH_ReceiveStoreForward |
                    ETH_InitStruct->ETH_FlushReceivedFrame |
                    ETH_InitStruct->ETH_TransmitStoreForward |
                    ETH_InitStruct->ETH_TransmitThresholdControl |
                    ETH_InitStruct->ETH_ForwardErrorFrames |
                    ETH_InitStruct->ETH_ForwardUndersizedGoodFrames |
                    ETH_InitStruct->ETH_ReceiveThresholdControl |
                    ETH_InitStruct->ETH_SecondFrameOperate);
    ETH->DMAOMR = (UINT)tmpreg;

    /* Reset the physical layer */
    ETH_WritePHYRegister(PHY_ADDRESS, PHY_BCR, PHY_Reset);
    ETH_WritePHYRegister(PHY_ADDRESS, PHY_MDIX, PHY_PN_SWITCH_AUTO);
}

static void EthPhyLinkStatusChangeHandler( void )
{
    static UINT unPrevResetSecs = 0;
    USHORT usStatus, usAutoNegoStatus;
    usAutoNegoStatus = ETH_ReadPHYRegister(PHY_ADDRESS, PHY_ANLPAR);
    usStatus = ETH_ReadPHYRegister(PHY_ADDRESS, PHY_BSR);

    //* 根据链路状态确定其是否已经协商完毕，如果尚未协商完毕，则重启PHY芯片
    if((usStatus & (PHY_Linked_Status)) && !usAutoNegoStatus)
      goto __lblReset;

    //* 如果物理链路已建立且自动协商完成
    if((usStatus & (PHY_Linked_Status)) && (usStatus & PHY_AutoNego_Complete))
    {
      usStatus = ETH_ReadPHYRegister(PHY_ADDRESS, PHY_STATUS);
      if(usStatus & (1 << 2))
        ETH->MACCR |= ETH_Mode_FullDuplex;
      else
      {
        if((usAutoNegoStatus & PHY_ANLPAR_SELECTOR_FIELD) != PHY_ANLPAR_SELECTOR_VALUE)
          ETH->MACCR |= ETH_Mode_FullDuplex;
        else
          ETH->MACCR &= ~ETH_Mode_FullDuplex;
      }
      ETH->MACCR &= ~(ETH_Speed_100M|ETH_Speed_1000M);

      //* 启动以太网芯片
      ETH_Start();
      l_blIsEthStartd = TRUE;
    }
    else
      goto __lblReset;

    return;

__lblReset:
  if(os_get_system_secs() - unPrevResetSecs > 15)
  {
    //* 重启PHY
    ETH_WritePHYRegister(PHY_ADDRESS, PHY_BCR, PHY_Reset);
    EXTEN->EXTEN_CTR &= ~EXTEN_ETH_10M_EN;

    //* 延时
    os_sleep_ms(500);
    //UINT unTicksPerSecs = LOS_MS2Tick(500);
    //LOS_TaskDelay(unTicksPerSecs);

    EXTEN->EXTEN_CTR |= EXTEN_ETH_10M_EN;
    ETH_WritePHYRegister(PHY_ADDRESS, PHY_MDIX, PHY_PN_SWITCH_AUTO);

    unPrevResetSecs = os_get_system_secs();
  }
}

static void EthConfig(void)
{
    ETH_InitTypeDef ETH_InitStructure;
    USHORT usTimeout = 10000;

    /* Enable Ethernet MAC clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC|RCC_AHBPeriph_ETH_MAC_Tx|RCC_AHBPeriph_ETH_MAC_Rx,ENABLE);
    EthSetClock( );

    /* Enable internal 10BASE-T PHY*/
    EXTEN->EXTEN_CTR |= EXTEN_ETH_10M_EN;    /* Enable 10M Ethernet physical layer   */

    /* Reset ETHERNET on AHB Bus */
    ETH_DeInit();

    /* Software reset */
    ETH_SoftwareReset();

    /* Wait for software reset */
    do{
        UINT i;
        for(i=0; i<10000; i++)
            __asm volatile("nop");
    }while((ETH->DMABMR & ETH_DMABMR_SR) && (!--usTimeout));

    /* ETHERNET Configuration */
    /* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
    ETH_StructInit(&ETH_InitStructure);
    /* Fill ETH_InitStructure parameters */
    /*------------------------   MAC   -----------------------------------*/
    ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;
    ETH_InitStructure.ETH_Speed = ETH_Speed_10M;
    ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
    ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
    ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
    ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
    ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
    /* Filter function configuration */
    ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
    ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
    ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
    ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_None/*ETH_MulticastFramesFilter_Perfect*/;
    ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
    /*------------------------   DMA   -----------------------------------*/
    /* When we use the Checksum offload feature, we need to enable the Store and Forward mode:
    the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum,
    if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
    ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
    ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
    ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;
    ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
    ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;
    ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Disable;
    /* Configure Ethernet */
    EthRegInit(&ETH_InitStructure);

    /* Configure MAC address */
    ETH_MACAddressConfig(ETH_MAC_Address0, (UCHAR *)lr_ubaMacAddr);


    //ETH->MACA0HR = (uint32_t)((MAC_ADDR5_DEFAULT << 8) | MAC_ADDR4_DEFAULT);
    //ETH->MACA0LR = (uint32_t)(MAC_ADDR0_DEFAULT | (MAC_ADDR1_DEFAULT << 8) | (MAC_ADDR2_DEFAULT << 16) | (MAC_ADDR3_DEFAULT << 24));


    /* Enable the Ethernet Rx & PhyLink Interrupt */
    ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R /*| ETH_DMA_IT_T */| ETH_DMA_IT_PHYLINK | ETH_DMA_IT_AIS | ETH_DMA_IT_RBU, ENABLE);
}

BOOL EthInit(void)
{
  EN_ONPSERR enErr;

  l_hMtxEthSend = os_thread_mutex_init();
  if (INVALID_HMUTEX == l_hMtxEthSend)
    return FALSE;

  os_sleep_ms(100);
  EthConfig();
  ETH_DMATxDescChainInit(DMATxDscrTab, MACTxBuf, ETH_TXBUFNB);
  ETH_DMARxDescChainInit(DMARxDscrTab, MACRxBuf, ETH_RXBUFNB);
  NVIC_EnableIRQ(ETH_IRQn);

  //* 等待物理层协商结束
  while(!l_blIsEthStartd)
  {
      EthPhyLinkStatusChangeHandler();
      os_sleep_secs(1);
  }

#if !DHCP_REQ_ADDR_EN
  ST_IPV4 stIPv4;
  stIPv4.unAddr = inet_addr("192.168.3.78");
  stIPv4.unSubnetMask = inet_addr("255.255.255.0");
  stIPv4.unGateway = inet_addr("192.168.3.1");
  stIPv4.unPrimaryDNS = inet_addr(PRIMARY_DNS_DEFAULT);
  stIPv4.unSecondaryDNS = inet_addr(SECONDARY_DNS_DEFAULT);
  stIPv4.unBroadcast = broadcast_addr(stIPv4.unAddr, stIPv4.unSubnetMask);
  l_pstNetifEth = ethernet_add(NETIF_ETH_NAME, lr_ubaMacAddr, &stIPv4, EthSend, start_thread_ethernet_ii_recv, &l_pstNetifEth, &enErr);
#else
  l_pstNetifEth = ethernet_add(NETIF_ETH_NAME, lr_ubaMacAddr, NULL, EthSend, start_thread_ethernet_ii_recv, &l_pstNetifEth, &enErr);
#endif

  if(!l_pstNetifEth)
  {
#ifdef ONPS_ENABLE_PRINTF
    printf("ethernet_add() failed, %s\r\n", onps_error(enErr));
#endif
    return FALSE;
  }

#if DHCP_REQ_ADDR_EN
  while(TRUE)
  {
    if(dhcp_req_addr(l_pstNetifEth, &enErr))
    {
    #ifdef ONPS_ENABLE_PRINTF
      printf("dhcp request ip address successfully.\r\n");
    #endif
      break;
    }
    else
    {
    #ifdef ONPS_ENABLE_PRINTF
      printf("dhcp request ip address failed, %s\r\n", onps_error(enErr));
    #endif
      os_sleep_secs(6);
    }
  }
#endif //* #if DHCP_REQ_ADDR_EN
  return TRUE;
}

static void EthReadData(void)
{
  UINT unFrameLen = 0;

__lblGetNext:
  if((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) != (UINT)RESET)
      return;

  if(((DMARxDescToGet->Status & ETH_DMARxDesc_ES) == (UINT)RESET) &&
     ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) != (UINT)RESET) &&
     ((DMARxDescToGet->Status & ETH_DMARxDesc_FS) != (UINT)RESET))
  {
    unFrameLen = ((DMARxDescToGet->Status & ETH_DMARxDesc_FL) >> ETH_DMARXDESC_FRAME_LENGTHSHIFT) - 4;
    if(l_pstNetifEth)
    {
      UCHAR *pubPacket;
      EN_ONPSERR enErr;
      pubPacket = (UCHAR *)buddy_alloc(sizeof(ST_SLINKEDLIST_NODE) + unFrameLen, &enErr);
      if(pubPacket)
      {
        //* 搬运数据到接收链表
        PST_SLINKEDLIST_NODE pstNode = (PST_SLINKEDLIST_NODE)pubPacket;
        pstNode->uniData.unVal = unFrameLen;
        memcpy(pubPacket + sizeof(ST_SLINKEDLIST_NODE), (__IO UCHAR *)(DMARxDescToGet->Buffer1Addr), unFrameLen);
        ethernet_put_packet(l_pstNetifEth, pstNode);
      }
      else
      {
      #if defined(ONPS_ENABLE_PRINTF) && defined(ONPS_DEBUG_LEVEL)
        printf("<EIRQ> %s\r\n", onps_error(enErr));
      #endif
      }
    }
  }

  DMARxDescToGet->Status = ETH_DMARxDesc_OWN;

  //* When Rx Buffer unavailable flag is set: clear it and resume reception
  if((ETH->DMASR & ETH_DMASR_RBUS) != (UINT)RESET)
  {
    ETH->DMASR = ETH_DMASR_RBUS; //* Clear RBUS ETHERNET DMA flag
    ETH->DMARPDR = 0;            //* Resume DMA reception

    return;
  }

  if((DMARxDescToGet->ControlBufferSize & ETH_DMARxDesc_RCH) != (UINT)RESET)
    DMARxDescToGet = (ETH_DMADESCTypeDef *)(DMARxDescToGet->Buffer2NextDescAddr);
  else
  {
    if((DMARxDescToGet->ControlBufferSize & ETH_DMARxDesc_RER) != (UINT)RESET)
      DMARxDescToGet = (ETH_DMADESCTypeDef *)(ETH->DMARDLAR);
    else
      DMARxDescToGet = (ETH_DMADESCTypeDef *)((UINT)DMARxDescToGet + 0x10 + ((ETH->DMABMR & ETH_DMABMR_DSL) >> 2));
  }

  goto __lblGetNext;
}

void ETH_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void ETH_IRQHandler(void)
{
    GET_INT_SP();
    rt_interrupt_enter();
    {
        UINT unStatus = ETH->DMASR;
        if (unStatus & ETH_DMA_IT_AIS)
        {
            if (unStatus & ETH_DMA_IT_RBU)
            {
               ETH_DMAClearITPendingBit(ETH_DMA_IT_RBU);
            }
            ETH_DMAClearITPendingBit(ETH_DMA_IT_AIS);
        }

        if(unStatus & ETH_DMA_IT_NIS)
        {
            if(unStatus & ETH_DMA_IT_R)
            {
                EthReadData();
                ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
            }

            //if(unStatus & ETH_DMA_IT_T)
            //{
            //    ETH_DMAClearITPendingBit(ETH_DMA_IT_T);
            //}

            if (unStatus & ETH_DMA_IT_PHYLINK)
            {
                ETH_DMAClearITPendingBit(ETH_DMA_IT_PHYLINK);
            }

            if (unStatus & ETH_DMA_IT_TBU)
            {
                /* Resume DMA transmission */
                ETH->DMATPDR = 0;
                ETH_DMAClearITPendingBit(ETH_DMA_IT_TBU);
            }

            ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
        }
    }
    rt_interrupt_leave(); //* 离开中断
    FREE_INT_SP();
}

INT ETH_TxPktChainMode(USHORT usFramLen, SHORT sBufListHead)
{
    /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
    if((DMATxDescToSet->Status & ETH_DMATxDesc_OWN) != (UINT)RESET)
        return -1;

    buf_list_merge_packet(sBufListHead, (UCHAR *)DMATxDescToSet->Buffer1Addr);

    /* Setting the Frame Length: bits[12:0] */
    DMATxDescToSet->ControlBufferSize = (usFramLen & ETH_DMATxDesc_TBS1);
    /* Setting the last segment and first segment bits (in this case a frame is transmitted in one descriptor) */
    DMATxDescToSet->Status |= ETH_DMATxDesc_LS | ETH_DMATxDesc_FS;

    /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
    DMATxDescToSet->Status |= ETH_DMATxDesc_OWN;

    /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
    if ((ETH->DMASR & ETH_DMASR_TBUS) != (UINT)RESET)
    {
        /* Clear TBUS ETHERNET DMA flag */
        ETH->DMASR = ETH_DMASR_TBUS;
        /* Resume DMA transmission*/
        ETH->DMATPDR = 0;
    }
    /* Update the ETHERNET DMA global Tx descriptor with next Tx descriptor */
    /* Chained Mode */
    /* Selects the next DMA Tx descriptor list for next buffer to send */
    DMATxDescToSet = (ETH_DMADESCTypeDef*) (DMATxDescToSet->Buffer2NextDescAddr);
    return (INT)usFramLen;
}

INT EthSend(SHORT sBufListHead, UCHAR *pubErr)
{
  //* 首先看看报文长度是否超出了DMA发送缓冲区的限制
  UINT unEthPacketLen = buf_list_get_len(sBufListHead);
  if(unEthPacketLen > ETH_TX_BUF_SIZE * ETH_TXBUFNB)
  {
    if(pubErr)
      *((EN_ONPSERR *)pubErr) = ERRPACKETTOOLARGE;
    return -1;
  }

  //* 发送之
  INT nRtnVal, nTryNum = 100;
  do
  {
    os_thread_mutex_lock(l_hMtxEthSend);
    {
        nRtnVal = ETH_TxPktChainMode((USHORT)unEthPacketLen, sBufListHead);
    }
    os_thread_mutex_unlock(l_hMtxEthSend);
  } while(nRtnVal < 0 && nTryNum++ < 1000);

  if(nRtnVal < 0)
  {
      if(pubErr)
        *((EN_ONPSERR *)pubErr) = ERRNETIFSEND;
  }

  return (INT)nRtnVal;
}
