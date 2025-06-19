#include <stdint.h>
#include <stdbool.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"

#define MAX_CHARS 80
#define MAX_FIELDS 5

typedef struct _USER_DATA
{
    char buffer[MAX_CHARS + 1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
} USER_DATA;

//----------------------------------------
// Initialization
//----------------------------------------

void initHw()
{
    initSystemClockTo40Mhz();
    SYSCTL_RCGCGPIO_R = SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);
}

//----------------------------------------
// UART Input
//----------------------------------------

void getsUart0(USER_DATA *data)
{
    uint8_t count = 0;
    while (true)
    {
        char c = getcUart0();
        if ((c == 8 || c == 127) && count > 0)
        {
            count--;
        }
        else if (c == 13) // ENTER
        {
            data->buffer[count] = '\0';
            return;
        }
        else if (c >= 32 && count < MAX_CHARS)
        {
            data->buffer[count++] = c;
        }
    }
}

//----------------------------------------
// Field Parsing
//----------------------------------------

void parseFields(USER_DATA *data)
{
    data->fieldCount = 0;
    bool prevWasDelimiter = true;
    char type = '\0';

    for (int i = 0; data->buffer[i] != '\0'; i++)
    {
        char c = data->buffer[i];

        // Letter
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
        {
            type = 'a';
        }
        // Number
        else if (c >= '0' && c <= '9')
        {
            type = 'n';
        }
        else
        {
            data->buffer[i] = '\0'; // null-terminate delimiters
            prevWasDelimiter = true;
            continue;
        }

        if (prevWasDelimiter && data->fieldCount < MAX_FIELDS)
        {
            data->fieldPosition[data->fieldCount] = i;
            data->fieldType[data->fieldCount] = type;
            data->fieldCount++;
        }
        prevWasDelimiter = false;
    }
}

//----------------------------------------
// Utilities
//----------------------------------------

char* getFieldString(USER_DATA *data, uint8_t fieldNumber)
{
    if (fieldNumber < data->fieldCount)
        return &data->buffer[data->fieldPosition[fieldNumber]];
    return 0;
}

int32_t getFieldInteger(USER_DATA *data, uint8_t fieldNumber)
{
    char* str = getFieldString(data, fieldNumber);
    if (str == 0 || data->fieldType[fieldNumber] != 'n')
        return 0;

    int32_t result = 0;
    while (*str >= '0' && *str <= '9')
    {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

bool isCommand(USER_DATA *data, const char strCommand[], uint8_t minArguments)
{
    char* userCommand = getFieldString(data, 0);
    uint8_t i = 0;

    while (strCommand[i] != '\0' && userCommand[i] != '\0')
    {
        if (strCommand[i] != userCommand[i])
            return false;
        i++;
    }

    if (strCommand[i] != userCommand[i])
        return false;

    return (data->fieldCount - 1) >= minArguments;
}

//----------------------------------------
// Integer to String (itoa)
//----------------------------------------

void intToStr(int32_t value)
{
    char buffer[12]; // Enough for 32-bit int
    int i = 0;
    if (value == 0)
    {
        putcUart0('0');
        return;
    }

    if (value < 0)
    {
        putcUart0('-');
        value = -value;
    }

    while (value > 0)
    {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    }

    while (i > 0)
    {
        putcUart0(buffer[--i]);
    }
}

//----------------------------------------
// Main Program
//----------------------------------------

int main(void)
{
    USER_DATA data;
    bool valid;

    initHw();
    initUart0();
    setUart0BaudRate(115200, 40e6);

    while (true)
    {
        valid = false;

        getsUart0(&data);
        parseFields(&data);

        if (isCommand(&data, "set", 2))
        {
            int32_t a = getFieldInteger(&data, 1);
            int32_t b = getFieldInteger(&data, 2);
            int32_t sum = a + b;

            putsUart0("set: ");
            intToStr(sum);
            putsUart0("\n");
            valid = true;
        }
        else if (isCommand(&data, "alert", 1))
        {
            putsUart0("alert: ");
            putsUart0(getFieldString(&data, 1));
            putsUart0("\n");
            valid = true;
        }

        if (!valid)
        {
            putsUart0("Invalid Command\n");
        }
    }

    return 0;
}
