#include "stm32f10x_it.h"
#include "main.h"

u8  USart8266_temp[200];

u8 Humi_int =0 ; 
u8 Humi_deci=0 ;
u8 Temp_int =0 ; 
u8 Temp_deci=0 ;
float Soil_Humidity=0.0;                //土壤湿度值



static u8 count = 0;
//u8 Time_Flag=0;
u16 Upsata_Time=0;              //上传时间计时
u16 Mode_Time=0;                //录入模式的计时，一分钟后如果没有完成录入自动回到普通模式
u16 Administ_Time=0;            //用于管理员的计时
u16 Door_Time=0;                 //用于关门的计时
u16 BEEP_Time=0;               //用于蜂鸣器的报警时间为30秒

void USART2_IRQHandler ( void )
{ 
        if ( USART_GetITStatus ( USART2, USART_IT_RXNE ) != RESET )
        {
                USart8266_temp[count]  = USART_ReceiveData( USART2 );
                count++;
                if(USART_ReceiveData( USART2 ) == '*')
                {
                        count = 0;
                        Down_Control();
                }
        }
}

void  BASIC_TIM_IRQHandler (void)               //定时器中断  用于各种定时
{
        if ( TIM_GetITStatus( BASIC_TIM, TIM_IT_Update) != RESET ) 
        { 
                Upsata_Time++;
                if(Upsata_Time>=2500)           //定时2.5秒上传一次数据
                {
                        Show_flag=!Show_flag;
                        
                        Upsata_Time=0;
                                char DataStr [ 500 ]  ; 
                        //湿度来自于DHT11
                        sprintf ( DataStr,"{\"Temperature\":\"%d.%d\",\"Humidity\":\"%d.%d\",\"Illumination\":\"%0.2f\"}",\
                                Temp_int,Temp_deci,Humi_int,Humi_deci,Illumination);
                //        //湿度来自于土壤湿度传感器
                //        sprintf ( DataStr,"{\"Temperature\":\"%d.%d\",\"Humidity\":\"%0.2f\",\"Illumination\":\"%0.2f\"}",\
                //                Temp_int,Temp_deci,Soil_Humidity,Illumination);
                        printf("%s\r\n",DataStr);
                        Usart2_SendString(USART2,DataStr);
                }
                if(Mode==Entering_Mode) //一分钟退出管理员录入模式
                {
                        Mode_Time++;
                        if(Mode_Time>=60000)
                        {
                                Mode_Time=0;
                                Step=0;
                                Mode=General_Mode;
                        }
                }
                if( Administ_Flag !=0 )  //管理员计时  五秒后如果还是管理员卡那么就进入录入模式
                {
                        Administ_Time++;
                        if(Administ_Time > 5000 && Administ_Time < 6000 )
                        {
                                Administ_Entering=1;
                        }
                        if(Administ_Time>=20000)
                        {
                                Administ_Time=0;
                                Administ_Entering=0;
                                Administ_Flag=0;                //5秒后如果没有再次刷管理员卡退出本次计时
                                Step=0;
                                Mode=General_Mode;
                        }
                }
                
                
                if(Door_Flag!=0)                        //处于解锁但未开门状态开始计时
                {
                        Door_Time++;
                        if((Door_Time>=10000) && READDOOR)    //如果时间超过10秒还没有开门那么就上锁
                        {                                       //低电平
                                Door_Time=0;
                                Door_Flag=2;
                        }
                        if((Door_Time>=60000) && (!READDOOR))
                        {
                                Door_Time=0;
                                Mode=Abnormal_Mode;
                                anomaly=0;
                                //异常发出警报
                        }
                }
                if(BEEP_Flag!=0)                        //处于解锁但未开门状态开始计时
                {
                        BEEP_Time++;
                        if(  BEEP_Time>= 30000   ) 
                        { 
                                BEEP_Time=0;
                                BEEP_Flag=0;
                        }
                }
                
                
                TIM_ClearITPendingBit(BASIC_TIM , TIM_FLAG_Update); 
        } 
}

void ADC_IRQHandler(void)
{ 
        if (ADC_GetITStatus(ADCx,ADC_IT_EOC)==SET) 
        {
                // 读取ADC的转换值
                ADC_ConvertedValue = ADC_GetConversionValue(ADCx);
        }
        ADC_ClearITPendingBit(ADCx,ADC_IT_EOC);
}


void Updata (void)
{
        

}


void Down_Control(void)
{
        
        char *p;
        char *name=NULL;
        char *status=NULL;
        
        printf("receave %s",USart8266_temp);
        p = strtok((char*)USart8266_temp, "_");
        if(p)
        {
                name = p;
        }
        p = strtok(NULL, "_");
        if(p)
        {
                status = p;
        }
        if(! (strcmp("setMotor1", name) || strcmp("true", status) ) ) 
        {
                //LED1_ON;   //添加需要的代码
                TIM3->CCR4=200;
                Exhaust_flag=1;
        }
        if(! (strcmp("setMotor1", name) || strcmp("false", status) ) ) 
        {
                //Water_OFF;  //添加需要的代码
                TIM3->CCR4=0;
                Exhaust_flag=0;
        }
        if(! (strcmp("setHydrovalve1", name) || strcmp("true", status) ) ) 
        {
                TemDown_ON;  //添加需要的代码
                
                Irrigation_flag=1;
        }
        if(! (strcmp("setHydrovalve1", name) || strcmp("false", status) ) ) 
        {
                TemDown_OFF;   //添加需要的代码
                Irrigation_flag=0;
        }
        
        /*              //用户决定是否关闭开启温控系统并设置阈值
         if()   //用户关闭升温系统
        {
                Temp_flag = 0 ;
        }
        if()   //用户开启升温系统
        {
                Temp_flag = 1 ;
        }
        if()   //接收来自云端设置的阈值
        {
                temp_max =  ;
        }
        if()    //接收来自云端设置的阈值
        {
                temp_min =  ;
        }
        
        */

}







































/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
        printf("[%s][%d]", __func__, __LINE__);
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
        /* Go to infinite loop when Hard Fault exception occurs */
        printf("[%s][%d]", __func__, __LINE__);
        
        while (1)
        {
          
        }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
        /* Go to infinite loop when Memory Manage exception occurs */
        
        printf("[%s][%d]", __func__, __LINE__);
        
        while (1)
        {
                
        }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
        /* Go to infinite loop when Bus Fault exception occurs */
        
        printf("[%s][%d]", __func__, __LINE__);
        
        while (1)
        {
                
        }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
        /* Go to infinite loop when Usage Fault exception occurs */
        
        printf("[%s][%d]", __func__, __LINE__);
        
        while (1)
        {
                
        }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
        printf("[%s][%d]", __func__, __LINE__);
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
        printf("[%s][%d]", __func__, __LINE__);
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
        printf("[%s][%d]", __func__, __LINE__);
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
        
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
