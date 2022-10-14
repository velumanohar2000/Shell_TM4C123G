// Serial Example
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"

//#define DEBUG

//Lab4 code
#define MAX_CHARS 80
#define MAX_FIELDS 5
typedef struct _USER_DATA
{
    char buffer[MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
} USER_DATA;

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();
    // Enable clocks
    SYSCTL_RCGCGPIO_R = SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);
}

void getsUart0(USER_DATA *data)
{
    int count = 0;
    int run = 1;
    while(run)
    {
        char c = getcUart0();                                       // get character from fifo
        if (count > 0 && (c == 127 || c == 8))                      // check if char is backspace
        {
            count --;
        }
        else if (c == 13)                                          //check if char is enter
        {
            data->buffer[count] = '\0';
            run = 0;                                              //exit loop
        }
        else if (c >= 32)                                         //if printable character store in array
        {
            data->buffer[count] = c;
            count++;
            if (count == MAX_CHARS)                              //if count is greater than Max char length then set null terminator
            {
                data->buffer[count] = '\0';
                run = 0;                                        //exit loop
            }
        }
    }
}

void parseFields(USER_DATA *data)
{
    data->fieldCount = 0;
    int i = 0;
    char c = data->buffer[i];
    char type;
    bool isAlphaNumeric = false;
    while(c != NULL)
    {
        if((c > 64 && c < 91) || (c > 96 && c < 123))
        {
            type = 'a';
            isAlphaNumeric = true;
        }
        else if ((c > 47 && c < 58))
        {
            type = 'n';
            isAlphaNumeric = true;
        }
        else
        {
            if (i > 0)
            {
                data->buffer[i] = '\0';
            }
            isAlphaNumeric = false;
            if (data->fieldCount == MAX_FIELDS)
            {
                return;
            }
        }
        if (isAlphaNumeric)
        {
           if (data->buffer[i-1] <= 47 || data->buffer[i-1] >= 123 || (data->buffer[i-1] >= 58 && data->buffer[i-1] <= 40) || (data->buffer[i-1] >= 91 && data->buffer[i-1] <= 96))
           {
               data->fieldType[data->fieldCount] = type;
               data->fieldPosition[data->fieldCount] = i;
               data->fieldCount++;
           }
        }
        c = data->buffer[++i];
    }
}

char* getFieldString(USER_DATA* data, uint8_t fieldNumber)
{
    char* str = &data->buffer[data->fieldPosition[fieldNumber]];
    if (fieldNumber < data->fieldCount)
    {

        return str;
    }
    else
    {
        return NULL;
    }

}

int32_t getFieldInteger(USER_DATA* data, uint8_t fieldNumber)
{
    char * str = getFieldString(data, fieldNumber);

    if (str != NULL && data->fieldType[fieldNumber] == 'n')
    {
        int result = 0;
        int i;
        for (i = 0; str[i] != '\0'; ++i)
        {
            result = result * 10 + str[i] - '0';
        }
        return result;
    }
    else
    {
        return 0;
    }
}

bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments)
{
    char * str = getFieldString(data, 0);

    while((*strCommand != '\0' && *str != '\0' ) && *strCommand == *str)
        {
            strCommand++;
            str++;
        }
    if (*strCommand==*str && data->fieldCount-1 >= minArguments)
    {
        return true;
    }
    else
    {
        return false;
    }

}

int main(void)
{
    initHw();
    USER_DATA data;
    initUart0();
    // Setup UART0 baud rate
    setUart0BaudRate(115200, 40e6);
    bool valid;

    while(true)
    {
        valid = false;

        //Get the string from the user
        getsUart0(&data);

        //echo back to the user
        #ifdef DEBUG
        putsUart0(data.buffer);
        putsUart0("\n");
        #endif

        //Parse Fields
        parseFields(&data);
        //Echo Back parseField data

        #ifdef DEBUG
        //unit8_t i;
        putsUart0("\n");
        int i = 0;
        for (i = 0; i < data.fieldCount; i++)
        {
            putcUart0(data.fieldType[i]);
            putsUart0("\t");
            putsUart0(&data.buffer[data.fieldPosition[i]]);
            putsUart0("\n");
        }
        #endif

        // Command evaluation
        if (isCommand(&data, "set", 2))
        {
            int32_t add = getFieldInteger(&data,1);
            int32_t dat = getFieldInteger(&data,2);
            int32_t plus = dat + add;
            valid = true;
            putsUart0("set\n");
        }
        if (isCommand(&data, "alert", 1))
        {
            char*str = getFieldString(&data,1);
            valid = true;
            putsUart0("alert\n");
            putsUart0(str);
            putsUart0("\n");
        }
        if (!valid)
        {
            putsUart0("Invalid Command\n");
        }
    }
    return 0;
}

