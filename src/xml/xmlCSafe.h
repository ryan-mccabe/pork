/**
 ****************************************************************************
 * <P> XML.c - implementation file for basic XML parser written in ANSI C++
 * for portability. It works by using recursion and a node tree for breaking
 * down the elements of an XML document.  </P>
 *
 * @version     V1.15
 *
 * @author      Frank Vanden Berghen
 * based on original implementation by Martyn C Brown
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 */

#ifndef __XML_C_SAFE_H
#define __XML_C_SAFE_H

#include <stdlib.h>

#ifdef WIN32
#include <tchar.h>
#endif

// Some common types for char set portable code
#ifdef _UNICODE
    #ifdef __linux__
        #include <wchar.h>
        #define _T(c) L ## c
    #endif
    #ifndef LPCTSTR
        #define LPCTSTR const wchar_t *
    #endif /* LPCTSTR */
    #ifndef LPTSTR
        #define LPTSTR wchar_t *
    #endif /* LPTSTR */
    #ifndef TCHAR
        #define TCHAR wchar_t
    #endif /* TCHAR */
#else
    #ifndef WIN32
        #define _T(c) c
    #endif
    #ifndef LPCTSTR
        #define LPCTSTR const char *
    #endif /* LPCTSTR */
    #ifndef LPTSTR
        #define LPTSTR char *
    #endif /* LPTSTR */
    #ifndef TCHAR
        #define TCHAR char
    #endif /* TCHAR */
#endif
#ifndef FALSE
    #define FALSE 0
#endif /* FALSE */
#ifndef TRUE
    #define TRUE 1
#endif /* TRUE */

// Enumeration for XML parse errors.
typedef enum XMLError
{
    eXMLErrorNone = 0,
    eXMLErrorMissingEndTag,
    eXMLErrorEmpty,
    eXMLErrorFirstNotStartTag,
    eXMLErrorMissingTagName,
    eXMLErrorMissingEndTagName,
    eXMLErrorNoMatchingQuote,
    eXMLErrorUnmatchedEndTag,
    eXMLErrorUnexpectedToken,
    eXMLErrorInvalidTag,
    eXMLErrorNoElements,
    eXMLErrorFileNotFound,
    eXMLErrorTagNotFound
} XMLError;

// Enumeration used to manage type of data. Use in conjonction with structure XMLNodeContents
typedef enum XMLElementType
{
    eNodeChild=0,
    eNodeAttribute=1,
    eNodeText=2,
    eNodeClear=3,
    eNodeNULL=4
} XMLElementType;

// Structure used to obtain error details if the parse fails.
typedef struct XMLResults
{
    enum XMLError error;
    int  nLine,nColumn;
} XMLResults;

// Structure for XML clear (unformatted) node (usually comments)
typedef struct {
    LPCTSTR lpszOpenTag; LPCTSTR lpszValue; LPCTSTR lpszCloseTag;
} XMLClear;

// Structure for XML attribute.
typedef struct {
    LPCTSTR lpszName; LPCTSTR lpszValue;
} XMLAttribute;

#endif
