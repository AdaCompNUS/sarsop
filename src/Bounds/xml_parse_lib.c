/*************************************************************************/
/* XML Parse Lib - A set of routines for parsing and generating	XML.	 */
/* 									 */
/* For Documentation and Usage Notes, see:				 */
/*				http://xmlparselib.sourceforge.net/	 */
/*									 */
/* Public Low-level functions:						 */
/*	xml_parse( fileptr, tag, content, maxlen, linenum );		 */
/*	xml_grab_tag_name( tag, name, maxlen );				 */
/*	xml_grab_attrib( tag, name, value, maxlen );			 */
/* Public Higher-level functions:					 */
/*	Xml_Read_File( filename );					 */
/*	Xml_Write_File( filename, xml_tree );				 */
/*									 */
/* Xml_Parse_Lib.c - MIT License:					 */
/*  Copyright (C) 2001, Carl Kindman					 */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the	 */
/* "Software"), to deal in the Software without restriction, including	 */
/* without limitation the rights to use, copy, modify, merge, publish,	 */
/* distribute, sublicense, and/or sell copies of the Software, and to	 */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:						 */
/* 									 */
/* The above copyright notice and this permission notice shall be 	 */
/* included in all copies or substantial portions of the Software.	 */
/* 									 */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 	 */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF	 */
/* MERCHANTABILITY, FITNESS FOR PARTICULAR PURPOSE AND NONINFRINGEMENT.	 */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,	 */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE	 */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.		 */
/* 									 */
/*  Carl Kindman 8-21-2001     carlkindman@yahoo.com			 */
/*  8-15-07 - Changed from strncpy to xml_strncpy for safety & speed.	 */
/*  10-2-07 - Changed to gracefully handle un-escaped ampersands.	 */
/*  11-19-08 - Added handling of escaped characters (&#xxxx).		 */
/*************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "xml_parse_lib.h"

/*.......................................................................
  . XML_NEXT_WORD - accepts a line of text, and returns with the        .
  . next word in that text in the third parameter, the original line    .
  . is shortened from the beginning so that the word is removed.        .
  . If the line encountered is empty, then the word returned will be    .
  . empty.                                                              .
  . NEXTWORD can parse on an arbitrary number of delimiters, and it 	.
  . returns everthing that was cut away in the second parameter.	.
  .......................................................................*/
void Xml_Next_Word( char *line, char *word, int maxlen, char *delim )
{
 int i=0, j=0, m=0, flag=1;

 while ((line[i]!='\0') && (flag))   /* Eat away preceding garbage */
  {
   j = 0;
   while ((delim[j]!='\0') && (line[i]!=delim[j]))  j = j + 1;
   if (line[i]==delim[j])  i++;  else  flag = 0;
  }
 maxlen--;
 while ((line[i]!='\0') && (m < maxlen) && (!flag))  /* Copy the word until the next delimiter. */
  {
   word[m++] = line[i++];
   if (line[i]!='\0')
    {
     j = 0;
     while ((delim[j]!='\0') && (line[i]!=delim[j]))  j = j + 1;
     if (line[i]==delim[j])  flag = 1;
    }
  }
 j = 0;			 /* Shorten line. */
 while (line[i]!='\0') line[j++] = line[i++];
 line[j] = '\0';	 /* Terminate the char-strings. */
 word[m] = '\0';
}
/********************************************************************************/
/* xml_strncpy - Copy src string to dst string, up to maxlen characters.	*/
/* Safer and faster than strncpy, because it does not fill destination string, 	*/
/* but only copies up to the length needed.  Src string should be		*/
/* null-terminated, and must-be if its allocated length is shorter than maxlen. */
/* Up to maxlen-1 characters are copied to dst string. The dst string is always */
/* null-terminated.  The dst string should be pre-allocated to at least maxlen  */
/* bytes.  However, this function will work safely for dst arrays that are less */
/* than maxlen, as long as the null-terminated src string is known to be	*/
/* shorter than the allocated length of dst, just like regular strcpy.		*/
/********************************************************************************/
void xml_strncpy( char *dst, const char *src, int maxlen )
{ 
  int j=0, oneless;
  oneless = maxlen - 1;
  while ((j < oneless) && (src[j] != '\0')) { dst[j] = src[j];  j++; }
  dst[j] = '\0';
}


void xml_remove_leading_trailing_spaces( char *word )
{
 int i=0, j=0;
 while ((word[i]!='\0') && ((word[i]==' ') || (word[i]=='\t') || (word[i]=='\n') || (word[i]=='\r')))
  i = i + 1;
 do { word[j++] = word[i++]; } while (word[i-1]!='\0');
 j = j - 2;
 while ((j>=0) && ((word[j]==' ') || (word[j]=='\t') || (word[j]=='\n') || (word[j]=='\r')))
  j = j - 1;
 word[j+1] = '\0';
}


void xml_escape_symbols( char *phrase, int maxlen )
{ /* Replace any ampersand (&), quotes ("), or brackets (<,>), with XML escapes. */
  int j=0, k, m, n;
  n = strlen(phrase);
  do
   {
    if (phrase[j]=='&') 
     {
      k = n + 4;  m = n;  n = n + 4;
      if (n > maxlen) {printf("xml_Parse: MaxStrLen %d exceeded.\n",maxlen); return;}
      do phrase[k--] = phrase[m--]; while (m > j);
      j++;  phrase[j++] = 'a';  phrase[j++] = 'm';  phrase[j++] = 'p';  phrase[j++] = ';';
     } else
    if (phrase[j]=='"') 
     {
      k = n + 5;  m = n;  n = n + 5;
      if (n > maxlen) {printf("xml_Parse: MaxStrLen %d exceeded.\n",maxlen); return;}
      do phrase[k--] = phrase[m--]; while (m > j);
      phrase[j++] = '&';  phrase[j++] = 'q';  phrase[j++] = 'u';  phrase[j++] = 'o';  phrase[j++] = 't';  phrase[j++] = ';';
     } else
    if (phrase[j]=='<') 
     {
      k = n + 3;  m = n;  n = n + 3;
      if (n > maxlen) {printf("xml_Parse: MaxStrLen %d exceeded.\n",maxlen); return;}
      do phrase[k--] = phrase[m--]; while (m > j);
      phrase[j++] = '&';  phrase[j++] = 'l';  phrase[j++] = 't';  phrase[j++] = ';';
     } else
    if (phrase[j]=='>') 
     {
      k = n + 3;  m = n;  n = n + 3;
      if (n > maxlen) {printf("xml_Parse: MaxStrLen %d exceeded.\n",maxlen); return;}
      do phrase[k--] = phrase[m--]; while (m > j);
      phrase[j++] = '&';  phrase[j++] = 'g';  phrase[j++] = 't';  phrase[j++] = ';';
     } else j++;
   }
  while (phrase[j] != '\0');
}


int xml_ishexadecimal( char ch, int *hex, int *sum )	/* Return true if character is a numeric or hexadeximal symbol, else zero. */
{							/* If numeric, capture value and set hex true if hex or false if base-10. */
 if (ch < '0') return 0;
 if (*hex)  *sum = 16 * *sum;  else  *sum = 10 * *sum;
 if (ch <= '9') { *sum = *sum + ch - 48;  return 1; }
 if (ch < 'A') return 0;
 if ((*hex) && (ch <= 'F')) { *sum = *sum + ch - 55;  return 1; }
 if ((ch == 'X') && (*hex != 1) && (*sum == 0)) { *hex = 1;  return 1; }
 if (ch < 'a') return 0;
 if ((*hex) && (ch <= 'f')) { *sum = *sum + ch - 87;  return 1; }
 if ((ch == 'x') && (*hex != 1) && (*sum == 0)) { *hex = 1;  return 1; } else return 0;
}


void xml_restore_escapes( char *phrase )
{ /* Replace any xml-escapes for (&), quotes ("), or brackets (<,>), with original symbols. */
  int j=0, k, m, n;

  n = strlen(phrase);
  if (n == 0) return;
  do
   {
    if (phrase[j]=='&') 
     {
      switch (phrase[j+1])
       {
        case 'a':   /* &amp; */
	  j++;  m = j;  k = j + 4;
	  if (k > n) {printf("xml_Parse: String ends prematurely after ampersand '%s'.\n",phrase); return;}
	  // if (strncmp( &(phrase[j]), "amp;", 4 ) != 0) {printf("xml_Parse: Unexpected &-escape '%s'.\n",phrase); return;}
	  n = n - 4;
	  do phrase[m++] = phrase[k++]; while (phrase[k-1] != '\0');
	 break;
        case 'q':   /* &quot; */
	  phrase[j++] = '"';
	  m = j;  k = j + 5;
	  if (k > n) {printf("xml_Parse: String ends prematurely after ampersand '%s'.\n",phrase); return;}
	  // if (strncmp( &(phrase[j]), "quot;", 5 ) != 0) {printf("xml_Parse: Unexpected &-escape '%s'.\n",phrase); return;}
	  n = n - 5;
	  do phrase[m++] = phrase[k++]; while (phrase[k-1] != '\0');
	 break;
        case 'l':   /* &lt; */
	  phrase[j++] = '<';
	  m = j;  k = j + 3;
	  if (k > n) {printf("xml_Parse: String ends prematurely after ampersand '%s'.\n",phrase); return;}
	  // if (strncmp( &(phrase[j]), "lt;", 3 ) != 0) {printf("xml_Parse: Unexpected &-escape '%s'.\n",phrase); return;}
	  n = n - 3;
	  do phrase[m++] = phrase[k++]; while (phrase[k-1] != '\0');
	 break;
        case 'g':   /* &gt; */
	  phrase[j++] = '>';
	  m = j;  k = j + 3;
	  if (k > n) {printf("xml_Parse: String ends prematurely after ampersand '%s'.\n",phrase); return;}
	  // if (strncmp( &(phrase[j]), "gt;", 3 ) != 0) {printf("xml_Parse: Unexpected &-escape '%s'.\n",phrase); return;}
	  n = n - 3;
	  do phrase[m++] = phrase[k++]; while (phrase[k-1] != '\0');
	 break;
	case '#':   /* &#0000; */
	  { int hex=0, sum = 0;
	   k = j + 2;
	   while ((k < j + 6) && (k < n) && (phrase[k] != ';') && (xml_ishexadecimal( phrase[k], &hex, &sum )))  k++;
	   if ((k > n) || (phrase[k] != ';'))
	    {printf("xml_Parse: String ends prematurely after ampersand '%s'.\n",phrase); return;}
	   phrase[j++] = sum;  m = j;  k++;
	   do phrase[m++] = phrase[k++]; while (phrase[k-1] != '\0');
          }
	 break;
	default: printf("xml_Parse: Unexpected char (%c) follows ampersand (&) in xml. (phrase='%s')\n", phrase[j+1], phrase );  j++;
       } 
     } else j++;
   }
  while (phrase[j] != '\0');
}



/************************************************************************/
/* XML_GRAB_TAG_NAME - This routine gets the tag-name, and shortens the	*/
/*  xml-tag by removing it from the tag-string.  Use after calling 	*/
/*  xml_parse to get the next tag-string from a file.  			*/
/*  If the tag is just a closing-tag, it will return "/".		*/
/*  Use in combination with xml_grab_attribute to parse any following	*/
/*  attributes within the tag-string.					*/
/* Inputs:	tag - String as read by xml_parse.		 	*/
/*		malen - Maximum length of returned name that can be 	*/
/*			returned.  (Buffer-size.)			*/
/* Output:	name - Character string.				*/
/************************************************************************/
void xml_grab_tag_name( char *tag, char *name, int maxlen )
{
 int j; 
 Xml_Next_Word( tag, name, maxlen, " \t\n\r");
 j = strlen(name);
 if ((j > 1) && (name[j-1] == '/'))	/* Check for case where slash was attached to end of tag-name. */
  {
   name[j-1] = '\0';	/* Move slash back to tag. */
   j = 0;  do { tag[j+1] = tag[j];  j++; } while (tag[j-1] != '\0');
   tag[0] = '/';
  }
}



/************************************************************************/
/* XML_GRAB_ATTRIBVALUE - This routine grabs the next name-value pair	*/
/*  within an xml-tag, if any.  Use after calling xml_parse and 	*/
/*  xml_grab_tag_name, to get the following tag attribute string.  Then */
/*  call this sequentially to grab each 				*/
/*  		name = "value" 						*/
/*  attribute pair, if any, until exhausted.  If the tag is closed by 	*/
/*  "/", the last name returned will be "/" and the value will be empty.*/
/*  This routine expands any escaped symbols in the value-string before */
/*  returning. 								*/
/* Inputs:	tag - String as read by xml_parse.		 	*/
/*		malen - Maximum length of returned name or value that 	*/
/*			can be returned.  (Buffer-sizes.)		*/
/* Outputs:	name - Character string.				*/
/*		value - Character string.				*/
/************************************************************************/
void xml_grab_attrib( char *tag, char *name, char *value, int maxlen )
{ 
 int j=0, k=0, m;

 Xml_Next_Word( tag, name, maxlen, " \t=\n\r");	 /* Get the next attribute's name. */
 /* Now get the attribute's value-string. */
 /* Sequence up to first quote.  Expect only white-space and equals-sign. */
 while ((tag[j]!='\0') && (tag[j]!='\"'))
  {
   if ((tag[j]!=' ') && (tag[j]!='\t') && (tag[j]!='\n') && (tag[j]!='\r') && (tag[j]!='='))
    printf("xml error: unexpected char before attribute value quote '%s'\n", tag);
   j++;
  }
 if (tag[j]=='\0')  { value[0] = '\0';  tag[0] = '\0';  return; }
 if (tag[j++]!='\"')
  { printf("xml error: missing attribute value quote '%s'\n", tag); tag[0] = '\0'; value[0] = '\0'; return;}
 while ((tag[j]!='\0') && (tag[j]!='\"')) { value[k++] = tag[j++]; } 
 value[k] = '\0';
 if (tag[j]!='\"') printf("xml error: unclosed attribute value quote '%s'\n", tag);  else j++;
 xml_restore_escapes( value );
 /* Now remove the attribute (name="value") from the original tag-string. */
 k = 0;
 do tag[k++] = tag[j++]; while (tag[k-1] != '\0');
}



/****************************************************************/
/* XML_PARSE - This routine finds the next <xxx> tag, and grabs	*/
/*	it, and then grabs whatever follows, up to the next tag.*/
/*	It returns the tag and its following contents.		*/
/*	It cleans any trailing white-space from the contents.	*/
/*  This routine is intended to be called iteratively, to parse	*/
/*  XML-formatted data.  Specifically, it pulls tag-string of	*/
/*  each tag (<...>) and content-string between tags (>...<).	*/
/* Inputs:							*/
/*	fileptr - Opened file pointer to read from.		*/
/*	malen - Maximum length of returned tag or content that  */
/*		can be returned.  (Buffer-sizes.)		*/
/* Outputs:							*/
/*	tag - Char string of text between next <...> brackets. 	*/
/*	content - Char string of text after > up to next < 	*/
/*		  bracket. 					*/
/****************************************************************/
void xml_parse( FILE *fileptr, char *tag, char *content, int maxlen, int *lnn )
{
 int i;  char ch;

 /* Get up to next tag. */
 do { ch = getc(fileptr);  if (ch=='\n') (*lnn)++; } while ((!feof(fileptr)) && (ch != '<'));

 i = 0; 	/* Grab this tag. */
 do 
  { do { tag[i] = getc(fileptr);  if (tag[i]=='\n') tag[i] = ' '; }
    while ((tag[i]=='\r') && (!feof(fileptr)));  i=i+1; 
    if ((i==3) && (tag[0]=='!') && (tag[1]=='-') && (tag[2]=='-'))
     { /*Filter_comment.*/
       i = 0;
       do { ch = getc(fileptr); if (ch=='-') i = i + 1; else if ((ch!='>') || (i==1)) i = 0; } 
       while ((!feof(fileptr)) && ((i<2) || (ch!='>')));
       do { ch = getc(fileptr);  if (ch=='\n') (*lnn)++; } while ((!feof(fileptr)) && (ch != '<'));
       i = 0;
     } /*Filter_comment.*/
  } while ((!feof(fileptr)) && (i < maxlen) && (tag[i-1] != '>'));
 if (i==0) i = 1;
 tag[i-1] = '\0';

 i = 0; 	/* Now grab contents until next tag. */
 do
  { do  content[i] = getc(fileptr);  while ((content[i]=='\r') && (!feof(fileptr)));
    if (content[i]==10) (*lnn)++; i=i+1;
  }
 while ((!feof(fileptr)) && (i < maxlen) && (content[i-1] != '<'));
 ungetc( content[i-1], fileptr );
 if (i==0) i = 1;
 content[i-1] = '\0';

 /* Clean-up contents by removing trailing white-spaces, and restoring any escaped characters. */
 xml_remove_leading_trailing_spaces( tag );
 xml_remove_leading_trailing_spaces( content );
 xml_restore_escapes( content );
}

//added by Zhan Wei

void xml_parse_tag_only( FILE *fileptr, char *tag, int maxlen, int *lnn )
{
 int i;  char ch;

 /* Get up to next tag. */
 do { ch = getc(fileptr);  if (ch=='\n') (*lnn)++; } while ((!feof(fileptr)) && (ch != '<'));

 i = 0; 	/* Grab this tag. */
 do 
  { do { tag[i] = getc(fileptr);  if (tag[i]=='\n') tag[i] = ' '; }
    while ((tag[i]=='\r') && (!feof(fileptr)));  i=i+1; 
    if ((i==3) && (tag[0]=='!') && (tag[1]=='-') && (tag[2]=='-'))
     { /*Filter_comment.*/
       i = 0;
       do { ch = getc(fileptr); if (ch=='-') i = i + 1; else if ((ch!='>') || (i==1)) i = 0; } 
       while ((!feof(fileptr)) && ((i<2) || (ch!='>')));
       do { ch = getc(fileptr);  if (ch=='\n') (*lnn)++; } while ((!feof(fileptr)) && (ch != '<'));
       i = 0;
     } /*Filter_comment.*/
  } while ((!feof(fileptr)) && (i < maxlen) && (tag[i-1] != '>'));
 if (i==0) i = 1;
 tag[i-1] = '\0';

 xml_remove_leading_trailing_spaces( tag );
 //don't grab the content, let user do low-level stream parsing
}

/* ============================================================== */
/* End of Re-Usable XML Parser Routines.         		  */
/* ============================================================== */

