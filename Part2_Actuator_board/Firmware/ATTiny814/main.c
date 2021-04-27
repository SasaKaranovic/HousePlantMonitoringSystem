/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "mcc_generated_files/mcc.h"
#include "SolenoidDriver.h"
#include "debug.h"

volatile uint32_t milliseconds              = 0;
volatile uint32_t solenoidTurnOff_target    = 0;
volatile bool bWDTRunning = 0;

static void SolenoidWDT_Tick(void);

/*
    Main application
*/
int main(void)
{
    /* Initializes MCU, drivers and middleware */
    __builtin_avr_delay_cycles(500);
    SYSTEM_Initialize();
    __builtin_avr_delay_cycles(2000);
    
    debugStr("\r\n\r\nWell hello there!\r\n");

    while (1)
    {
        SolenoidWDT_Tick();
        __builtin_avr_wdr();
    }
}

void SysTick(void)
{
    milliseconds++;
}

static void SolenoidWDT_Tick(void)
{
    if((Solenoid_GetState()) && (bWDTRunning) && (solenoidTurnOff_target < milliseconds))
    {
        Solenoid_SET_Off();
        Solenoid_ActLED_Off();
        bWDTRunning = false;
    }
}

/**
    End of File
*/