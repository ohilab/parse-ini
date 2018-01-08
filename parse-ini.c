/******************************************************************************
 * Copyright (C) 2017 Matteo Civale
 *
 * Authors:
 *  Matteo Civale
 *  Marco Giammarini <m.giammarini@warcomeb.it>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 ******************************************************************************/

#include "parse-ini.h"

#include "fat-mla/ff.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define PARSEINI_STRING_BUFFER_LENGTH    100

#define PARSEINI_STRING_SECTION_UNIX     "[%s]\n"
#define PARSEINI_STRING_SECTION_WIN      "[%s]\r\n"

static TCHAR ParseINI_buffer[PARSEINI_STRING_BUFFER_LENGTH];

static ParseINI_Errors ParseINI_cast2u8 (const char* string, uint8_t length, void* param)
{
    if (length > 3) return PARSEINIERRORS_PARAM_WRONG_LENGTH;

    if (dtu8(string,(uint8_t*)param,length) != ERRORS_UTILITY_CONVERSION_OK)
        return PARSEINIERRORS_PARAM_WRONG_CONVERSION;
    else
        return PARSEINIERRORS_OK;
}

static ParseINI_Errors ParseINI_cast2u16 (const char* string, uint8_t length, void* param)
{
    if (length > 5) return PARSEINIERRORS_PARAM_WRONG_LENGTH;

    if (dtu16(string,(uint16_t*)param,length) != ERRORS_UTILITY_CONVERSION_OK)
        return PARSEINIERRORS_PARAM_WRONG_CONVERSION;
    else
        return PARSEINIERRORS_OK;
}

static ParseINI_Errors ParseINI_cast2hour (const char* string, uint8_t length, void* param)
{
    Time_TimeType* hours = (Time_TimeType*)param;
    uint8_t hour, minute;
    uint8_t hourLength, minuteLength;
    char* ptr1;
    char* ptr2;

    // 2 char for hour, plus 1 char for separator and 2 char for minute
    if (length > 5) return PARSEINIERRORS_PARAM_WRONG_LENGTH;

    // Clear all the contents
    hours->hours = 0;
    hours->minutes = 0;
    hours->seconds = 0;

    ptr1 = string;
    // Fix the second pointer to the separator
    ptr2 = strchr(ptr1,':');
    // Clear the separator with end string
    *ptr2 = '\0';
    ptr2++;

    // Compute length for both string
    hourLength = strlen(ptr1);
    minuteLength = strlen(ptr2);
    if (*(ptr2+minuteLength-2) == '\r')
        minuteLength -= 2;
    else
        minuteLength -= 1;

    // Convert the string into numbers
    if (dtu8(ptr1,&hour,hourLength) != ERRORS_UTILITY_CONVERSION_OK)
        return PARSEINIERRORS_PARAM_WRONG_CONVERSION;
    if (dtu8(ptr2,&minute,minuteLength) != ERRORS_UTILITY_CONVERSION_OK)
        return PARSEINIERRORS_PARAM_WRONG_CONVERSION;
    // Check if the numbers are valid
    if ((hour < 24) && (minute < 60))
    {
        hours->hours = hour;
        hours->minutes = minute;
        return PARSEINIERRORS_OK;
    }
    else
    {
        return PARSEINIERRORS_PARAM_WRONG_CONVERSION;
    }
}

static ParseINI_Errors ParseINI_cast2ip (const char* string, uint8_t length, void* param)
{
    uint32_t* ip = (uint32_t*)param;
    uint8_t value;
    uint8_t valueLength;
    char* ptr1;
    char* ptr2;
    char* ptr3;
    uint8_t count = 0;

    // 3 char for each value, plus 3 char for separators
    if ((length > 15) || (length < 7)) return PARSEINIERRORS_PARAM_WRONG_LENGTH;

    // Check validity
    if (!Utility_isValidIp4Address(string)) return PARSEINIERRORS_PARAM_WRONG_LENGTH;

    // Clear all the contents
    *ip = 0;

    ptr3 = ptr1 = string;

    while (ptr3 = strchr(ptr3,'.'))
    {
        ptr3++;
        count++;
    }
    // Check if point into the string are three
    if (count != 3) return PARSEINIERRORS_PARAM_WRONG_FORMAT;

    // Check all field
    for (count = 0; count < 4; count++)
    {
        ptr2 = strchr(ptr1,'.');
        // Clear the separator with end string
        if (ptr2)
            *ptr2 = '\0';

        valueLength = strlen(ptr1);
        if (count == 3)
        {
            if (*(ptr1+valueLength-2) == '\r')
                valueLength -= 2;
            else if (*(ptr1+valueLength-1) == '\r')
                valueLength -= 1;
            else if (*(ptr1+valueLength-1) == '\n')
                valueLength -= 1;
        }

        // Convert the string into numbers
        if (dtu8(ptr1,&value,valueLength) != ERRORS_UTILITY_CONVERSION_OK)
        {
            *ip = 0; // Reset result
            return PARSEINIERRORS_PARAM_WRONG_CONVERSION;
        }
        *ip = value + ((*ip) << 8);

        // Move the second pointer forward
        ptr1 = ptr2 + 1;
    }

    return PARSEINIERRORS_OK;
}

static ParseINI_Errors ParseINI_cast2mac (const char* string, uint8_t length, void* param)
{

}

static ParseINI_Errors ParseINI_cast2string (const char* string, uint8_t length, void* param)
{

}

ParseINI_Errors ParseINI_open (ParseINI_FileHandle dev)
{
    // Open file in reading mode
    if ((dev->fr = f_open(&(dev->file),dev->filePath,FA_READ)) != FR_OK)
        return PARSEINIERRORS_OPEN_FAIL;

    return PARSEINIERRORS_OK;
}

ParseINI_Errors ParseINI_close (ParseINI_FileHandle dev)
{
    // Close file
    if ((dev->fr = f_close(&(dev->file))) != FR_OK)
        return PARSEINIERRORS_CLOSE_FAIL;

    return PARSEINIERRORS_OK;
}

ParseINI_Errors ParseINI_get (ParseINI_FileHandle dev,
                              const char* section,
                              const char* parameter,
                              ParseINI_VarType type,
                              void* value)
{
    bool isSectionFound = FALSE;
    // Used to define section string with win format
    static char sectionTagWin[20];
    // Used to define section string with unix format
    static char sectionTagUnix[20];
    static char paramTag[20];
    static uint8_t paramLength = 0;
    // Pointer for reading file
    uint8_t* head;
    uint8_t* pointer;

    if (&(dev->file) == 0)
        return PARSEINIERRORS_FILE_NOT_OPEN;

    // Move read pointer to beginning of the file
    f_lseek(&(dev->file),0);

    // Generation of dummy strings for section tag
    sprintf(sectionTagWin,PARSEINI_STRING_SECTION_WIN, section);
    sprintf(sectionTagUnix,PARSEINI_STRING_SECTION_UNIX, section);

    while (head = f_gets(ParseINI_buffer,PARSEINI_STRING_BUFFER_LENGTH,&(dev->file)))
    {
        if (!strcmp(ParseINI_buffer,sectionTagWin) || !strcmp(ParseINI_buffer,sectionTagUnix))
        {
            isSectionFound = TRUE;
            break;
        }
    }
    // check if the section was found
    if (!isSectionFound) return PARSEINIERRORS_SECTION_NOT_FOUND;

    paramLength = sprintf(paramTag, "%s", parameter);
    // Search for the param
    while(head = f_gets(ParseINI_buffer, PARSEINI_STRING_BUFFER_LENGTH,&(dev->file)))
    {
        if (!strncmp(ParseINI_buffer,paramTag,paramLength))
        {
            // Point to the next char after the parameter name
            pointer = ParseINI_buffer + paramLength;

            // Trim blank and equal char
            while ((*pointer == ' ') || (*pointer == '=')) pointer++;

            // Compute the value length
            paramLength = strlen(pointer);
            if (*(pointer+paramLength-2) == '\r')
                paramLength -= 2;
            else
                paramLength -= 1;

            switch (type)
            {
            case PARSEINIVARTYPE_U8:
                return ParseINI_cast2u8(pointer,paramLength,value);
            case PARSEINIVARTYPE_U16:
            case PARSEINIVARTYPE_I16: // FIXME
                return ParseINI_cast2u16(pointer,paramLength,value);
            case PARSEINIVARTYPE_HOUR:
                return ParseINI_cast2hour(pointer,paramLength,value);
            case PARSEINIVARTYPE_IP:
                return ParseINI_cast2ip(pointer,paramLength,value);
            case PARSEINIVARTYPE_MAC:
                return ParseINI_cast2mac(pointer,paramLength,value);
            case PARSEINIVARTYPE_STRING:
                return ParseINI_cast2string(pointer,paramLength,value);
            case PARSEINIVARTYPE_FLOAT:
            default:
                return PARSEINIERRORS_WRONG_TYPE;
            }
        }
    }
    // The param was not found!
    return PARSEINIERRORS_PARAM_NOT_FOUND;
}
