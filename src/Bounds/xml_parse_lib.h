/*************************************************************************/
/* XML Parse Lib Header file - Public definitions.       		 */
/*  Library routines for parsing and generating	XML.			 */
/* 									 */
/* For Documentation and Usage Notes, see:				 */
/*				http://xmlparselib.sourceforge.net/	 */
/*									 */
/* Xml_Parse_Lib.c - MIT License:                                        */
/* Copyright (C) 2001, Carl Kindman					 */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*									 */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*									 */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR PARTICULAR PURPOSE AND NONINFRINGEMENT.  */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*									 */
/*  Carl Kindman 8-21-2001     carl_kindman@yahoo.com			 */
/************************************************************************/
#define XML_MAX_STRLEN 10000

 /*-----------------------------------*/
 /* Lower-level convenience routines. */
 /*-----------------------------------*/

 /* Get next xml-tag, attribtues, and contents from an xml-file. */
 void xml_parse( FILE* , char*, char*, int , int*);

 /* Added by Zhan Wei */
 /* Get next xml-tag, attribtues only from an xml-file. */
 void xml_parse_tag_only( FILE* , char*, int, int*);

 /* Pull-off tag's name. */
 void xml_grab_tag_name( char *tag, char *name, int maxlen );

 /* Pull-off next attribute name-value pair, if any, from tag-string. */
 void xml_grab_attrib( char *tag, char *name, char *value, int maxlen );

