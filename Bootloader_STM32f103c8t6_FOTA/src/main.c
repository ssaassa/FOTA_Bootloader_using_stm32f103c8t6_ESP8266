#include "STD_TYPES.h"
#include "GPIO.h"
#include "RCC_interface.h"

#include "HUART_interface.h"
#include "Bootloader.h"
#include "Debug.h"
#include "Trace.h"
#include "WIFI_interface.h"
#include "Delay_interface.h"




int main(void)
{
	u8 Bootloader_Request_button_State;

	RCC_f32GetPLLMultiplierValue();
	//RCC_voidSetClockStatus(RCC_ENABLE_HSE);
	//RCC_voidSWSelectClock(RCC_SW_HSE);
	RCC_voidChangeBusPrescalar(RCC_APB1_PRESCALAR_2);
	RCC_voidEnablePeripheralClock(RCC_PERIPHERALS_PORTB); //Activate clock for button port
	RCC_voidEnablePeripheralClock(RCC_PERIPHERALS_PORTC); //Activate clock for on-board led port
	RCC_voidEnablePeripheralClock(RCC_PERIPHERALS_CRC);   //Activate clock for CRC peripheral
	RCC_voidEnablePeripheralClock(RCC_PERIPHERALS_FLITF); //Activate clock for Flash driver

	HUART_u8Init(HUART_USART1, 115200, UART_STOP_BIT1, UART_PARITY_DISABLED);




	GPIO_Pin_t Bootloader_Request_button;
	Bootloader_Request_button.port = PORTB;
	Bootloader_Request_button.mode = GPIO_MODE_INPUT_PULLUP_PULLDOWN;
	Bootloader_Request_button.pin  = 12;
	Bootloader_Request_button.speed= GPIO_INPUT_MODE_RESET_STATE;

	GPIO_Init(&Bootloader_Request_button);
	GPIO_Pin_Write(&Bootloader_Request_button,HIGH);

	delay_ms(200);
	GPIO_Pin_Read(&Bootloader_Request_button,&Bootloader_Request_button_State);

	if(Bootloader_Request_button_State) bootloader_voidJumpToUserApp(); //if the button is not pressed
	else bootloader_voidUARTReadData();									//if the button is  pressed

	while(1)
	{
		trace_printf("zeft\n");
	}

	return 0;
}
