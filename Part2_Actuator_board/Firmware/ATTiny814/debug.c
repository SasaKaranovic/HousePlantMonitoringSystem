#include "debug.h"
#include "mcc_generated_files/include/usart0.h"

static const char HEX[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
#define HEXUPPER(x) (HEX[ ((x>>4)&0x0F) ])
#define HEXLOWER(x) (HEX[(x&0x0F)])

#ifdef _DEBUG_ENABLE_
void debugStr(char *data)
{
    while(*data)
    {
        while (!(USART0.STATUS & USART_DREIF_bm));
        USART0.TXDATAL = *data++;
    }
}

void debugHex(char data)
{
    char tmp[7] = { '0', 'x', '0', '0', '\r', '\n', 0};
    
    tmp[2] = HEXUPPER(data);
    tmp[3] = HEXLOWER(data);
      
    debugStr(tmp);
}

#endif