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

#ifndef __PARSE_INI_H
#define __PARSE_INI_H

#define PARSEINI_LIBRARY_VERSION     "1.0.0"
#define PARSEINI_LIBRARY_VERSION_M   1
#define PARSEINI_LIBRARY_VERSION_m   0
#define PARSEINI_LIBRARY_VERSION_bug 0
#define PARSEINI_LIBRARY_TIME        0

/*
 * The user must define these label with ...
 * The name of the define are:
 *     #define PARSEINI_PATH_LENGTH_FILE     100
 *     #define PARSEINI_PATH_LENGTH_DIR      120
 */
#ifndef __NO_BOARD_H
#include "board.h"
#endif

#include "fat-mla/ff.h"

typedef enum _ParseINI_Errors
{
    /**< No Errors */
    PARSEINIERRORS_OK                    = 0x00,

    PARSEINIERRORS_OPEN_FAIL,
    PARSEINIERRORS_CLOSE_FAIL,
    PARSEINIERRORS_FILE_NOT_OPEN,

    PARSEINIERRORS_SECTION_NOT_FOUND,
    PARSEINIERRORS_PARAM_NOT_FOUND,
    PARSEINIERRORS_PARAM_WRONG_LENGTH,
    PARSEINIERRORS_PARAM_WRONG_CONVERSION,
    PARSEINIERRORS_PARAM_WRONG_FORMAT,
    PARSEINIERRORS_WRONG_TYPE,

} ParseINI_Errors;

/**
 *
 */
typedef enum _ParseINI_VarType
{

    PARSEINIVARTYPE_U8,
    PARSEINIVARTYPE_U16,
    PARSEINIVARTYPE_I16,
    PARSEINIVARTYPE_FLOAT,
    PARSEINIVARTYPE_STRING,
    PARSEINIVARTYPE_IP_ADDR,
    PARSEINIVARTYPE_MAC_ADDR,
    PARSEINIVARTYPE_HOUR,

} ParseINI_VarType;


/**
 *
 */
typedef struct _ParseINI_File
{
    FIL file;
    FATFS* fat;
    FRESULT fr;
    uint8_t filePath[PARSEINI_PATH_LENGTH_FILE];
    uint8_t dirPath[PARSEINI_PATH_LENGTH_DIR];
} ParseINI_File;

typedef ParseINI_File* ParseINI_FileHandle;

/**
 *
 */
ParseINI_Errors ParseINI_open (ParseINI_FileHandle dev);

/**
 *
 */
ParseINI_Errors ParseINI_close (ParseINI_FileHandle dev);

/**
 *
 * @param[in] dev
 * @param[in] section
 * @param[in] parameter
 * @param[in] type
 * @param[out] value
 * @return
 */
ParseINI_Errors ParseINI_get (ParseINI_FileHandle dev,
                              const char* section,
                              const char* parameter,
                              ParseINI_VarType type,
                              void* value);

#endif /* __PARSE_INI_H */
