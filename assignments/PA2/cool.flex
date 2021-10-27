/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>
#include <cctype>
/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble strin g constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */
static int commentLevel=0;//Counter for nested comments.
static int commentCaller;//In case that sometimes a comment is inside another comment(i.e. multi-level comment), we shall store the original start condition to distinguish between the two. 
static int stringCaller;
static std::string stringConstBuf;
static int strErrRecover=0;

%}

/*
 * Define names for regular expressions here.
 */

DARROW          =>
CLASS			[cC][lL][aA][sS][sS]
ELSE			[eE][lL][sS][eE]
FI 			    [fF][iI]	
IF				[iI][fF]
IN				[iI][nN]
INHERITS        [iI][nN][hH][eE][rR][iI][tT][sS]		
LET				[lL][eE][tT]
LOOP			[lL][oO][oO][pP]
POOL			[pP][oO][oO][lL]
THEN			[tT][hH][eE][nN]
WHILE			[wW][hH][iI][lL][eE]
CASE			[cC][aA][sS][eE]
ESAC			[eE][sS][aA][cC]
OF				[oO][fF]
NEW				[nN][eE][wW]
ISVOID			[iI][sS][vV][oO][iI][dD]
ASSIGN			<-
NOT				[nN][oO][tT]
LE				<=

%x COMMENT
%x STRING
%x STRING_ESCAPE

%%
 /*
  *	Whitespace characters
  */
[ \t\f\r\v] {}	
 /*
  * Illegal characters
  */
[\[\]\'\>\!\#\$\%\^\&\?\\\|\`] {
	cool_yylval.error_msg=yytext;
	return (ERROR);
}
\n {++curr_lineno; }
 /*
  *  Nested comments
  */
"(*" {
	commentCaller=INITIAL;
	BEGIN(COMMENT);
    ++commentLevel;
}
<COMMENT>"(*" {
    ++commentLevel;
}
<COMMENT>"*)" {
    if(--commentLevel==0) {
        BEGIN(commentCaller);
    }
}
"*)" {
	cool_yylval.error_msg="Unmatched *)";
	return (ERROR);
}
<COMMENT><<EOF>> {
	BEGIN(commentCaller);
	cool_yylval.error_msg="EOF in comment";
	return (ERROR);
}
<COMMENT>\n {
    ++curr_lineno;
}
<COMMENT>. {
	if(yytext[0] == '\n' ) {
		++curr_lineno;
	}
}
 /*
  *	Single-line comments
  */
--.* {/*cout<<"Single-line comment"<<endl;*/ }
 /*
  *  The multiple-character operators.
  */
{DARROW}		{ return (DARROW); }
{CLASS} { /*cout<<"MEET CLASS"<<endl;*/ return (CLASS); }
{ELSE} { return (ELSE); }
{FI} { return (FI); }
{IF} { return (IF); }
{IN} { return (IN); }
{INHERITS} { return (INHERITS); }
{LET} { return (LET); }
{LOOP} { return (LOOP); }
{POOL} { return (POOL); }
{THEN} { return (THEN); }
{WHILE} { return (WHILE); }
{CASE} { return (CASE); }
{ESAC} { return (ESAC); }
{OF} { return (OF); }
{NEW} { return (NEW); }
{ISVOID} { return (ISVOID); }
{ASSIGN} { return (ASSIGN); }
{NOT} { return (NOT); }
{LE} { return (LE); }

 
 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */
t[Rr][Uu][Ee] {
	cool_yylval.boolean =true;
	return (BOOL_CONST);
}
f[Aa][Ll][Ss][Ee] {
	cool_yylval.boolean=false;
	return (BOOL_CONST);
}
SELF_TYPE {
	cool_yylval.symbol = idtable.add_string("SELF_TYPE");
	return (TYPEID);
}
[A-Z][A-Za-z0-9_]* {
	cool_yylval.symbol = idtable.add_string(yytext,yyleng);
	return (TYPEID);
}

[a-z][A-Za-z0-9_]* {
	cool_yylval.symbol = idtable.add_string(yytext,yyleng);
	return (OBJECTID);
}
 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */

\" {
	stringCaller=INITIAL;
	stringConstBuf="";
//    cout<<"Begin String in LineNO."<<curr_lineno<<endl;
	BEGIN(STRING);
}

<STRING>[^\"\\\n\0]*\" {
	stringConstBuf+=std::string(yytext,yyleng-1);
    if(!strErrRecover) {
        if(stringConstBuf.size()<=1024) {
            cool_yylval.symbol=stringtable.add_string((char*)stringConstBuf.c_str(),stringConstBuf.size());
            BEGIN(stringCaller);
            //cout<<"LEN="<<stringConstBuf.size()<<endl;
            return (STR_CONST);
         } else {
            BEGIN(stringCaller);
            cool_yylval.error_msg="String constant too long";
            return (ERROR);
        }
    } else {
        BEGIN(stringCaller);
        strErrRecover=false;
    }
}

<STRING>[^\"\\\n\0]*\\ {
	stringConstBuf+=std::string(yytext,yyleng-1);
//    cout<<"Start STR_ESC "<<stringConstBuf<<"\""<<endl;
	BEGIN(STRING_ESCAPE);
}


<STRING>[^\"\\\n\0]* {
	stringConstBuf+=std::string(yytext,yyleng);
}

<STRING_ESCAPE>\n {
	stringConstBuf+='\n';
	++curr_lineno;
   // cout<<"STR_ESC "<<curr_lineno-1<<"->"<<curr_lineno<<endl;
	BEGIN(STRING);
}
<STRING_ESCAPE>n {
    // cout << "escape \\n !" << endl;
    stringConstBuf+='\n';
	BEGIN(STRING);
}

<STRING_ESCAPE>b {
    stringConstBuf+='\b';
    BEGIN(STRING);
}

<STRING_ESCAPE>t {
   stringConstBuf+='\t';
   BEGIN(STRING);
}

<STRING_ESCAPE>f {
	stringConstBuf+='\f';
	BEGIN(STRING);
}

<STRING_ESCAPE>\0 {
    cool_yylval.error_msg = "String contains escaped null character";
    stringConstBuf="";
    strErrRecover=true;
    BEGIN(STRING);
    return (ERROR);
}

<STRING_ESCAPE>. {
 	stringConstBuf+=yytext[0]; 
    BEGIN(STRING);
}


<STRING>[^\"\\\n\0]*\n {
    // push first
    // contains the last character for yytext does not include \n
    stringConstBuf+=std::string(yytext,yyleng);
    //cout<<"ERROR STR is \""<<stringConstBuf<<"\"\t"<<curr_lineno<<endl;
    //setup error later
    if(strErrRecover) {
        BEGIN(stringCaller);
        ++curr_lineno;
    } else {
            cool_yylval.error_msg = "Unterminated string constant";
            BEGIN(stringCaller);
            ++curr_lineno;
            return (ERROR);
    }
}

<STRING>[^\"\\\n\0]*\0 {
    cool_yylval.error_msg="String contains null character.";
    strErrRecover=true;
    return (ERROR);
}

<STRING_ESCAPE><<EOF>> {
    cool_yylval.error_msg = "EOF in string constant";
    BEGIN(STRING);
    return (ERROR);
}
<STRING><<EOF>> {
    cool_yylval.error_msg = "EOF in string constant";
    stringConstBuf="";
    BEGIN(stringCaller);
    return (ERROR);
}
 /*
  * Numeral constants
  *
  */
[0-9]+ {
	cool_yylval.symbol=inttable.add_string(yytext,yyleng);
	return (INT_CONST);
}

\_ {
    cool_yylval.error_msg="_";
    return (ERROR);
}

. {
    if(!isprint(yytext[0])) {
            cool_yylval.error_msg=yytext;
            return ERROR;	
    } else {
        //cool_yylval.symbol=yytext[0];
        return yytext[0];
    }
}
%%
