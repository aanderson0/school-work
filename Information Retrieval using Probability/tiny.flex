/*
  File Name: tiny.flex
  JFlex specification for the TINY language
*/
   
%%
   
%class Lexer
%type Token
%line
%column
    
%eofval{
  return null;
%eofval};


/* A line terminator is a \r (carriage return), \n (line feed), or
   \r\n. */
LineTerminator = \r|\n|\r\n
   
/* White space is a line terminator, space, tab, or form feed. */
WhiteSpace     = [ \t\f]

/* A literal integer is is a number beginning with a number between
   one and nine followed by zero or more numbers between zero and nine
   or just a zero.  */
digit = [0-9]
number = {digit}+
   
/* A identifier integer is a word beginning a letter between A and
   Z, a and z, or an underscore followed by zero or more letters
   between A and Z, a and z, zero and nine, or an underscore. */
letter = [a-zA-Z]
identifier = {letter}+
   
%%
   
/*
   This section contains regular expressions and actions, i.e. Java
   code, that will be executed when the scanner matches the associated
   regular expression. */
   
"$DOC"                    { return new Token(Token.LABEL, yytext(), yyline, yycolumn); }
"$TITLE"                  { return new Token(Token.LABEL, yytext(), yyline, yycolumn); }
"$TEXT"                   { return new Token(Token.LABEL, yytext(), yyline, yycolumn); }
[A-Z]*{number}"-"{number}   { return new Token(Token.LABEL, yytext(), yyline, yycolumn); }
(({number}(","{number})*)*{letter}+({number}(","{number})*)*)+                 { return new Token(Token.WORD, yytext(), yyline, yycolumn); }
{number}(","{number})*                  { return new Token(Token.NUMBER, yytext(), yyline, yycolumn); }
("'"+|{letter}+|({number}(","{number})*)+)+           { return new Token(Token.APOSTROPHIZED, yytext(), yyline, yycolumn); }
("-"+|{letter}+|({number}(","{number})*)+|"'"+)+           { return new Token(Token.HYPHENATED, yytext(), yyline, yycolumn); }
{LineTerminator}            { return new Token(Token.NEWLINE, yytext(), yyline, yycolumn); }
{WhiteSpace}+      { /* skip whitespace */ }   
.                  { return new Token(Token.PUNCTUATION, yytext(), yyline, yycolumn); }
