%{
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "SemanticTypes.h"
#include "queryParser.tab.h"
#include "ThrashCan.h"
#include "TableTypes.h"
#include <endian.h>
#define YY_DECL extern int yylex()
%}

%option noyywrap

%%
[ \t\n\b\r] ;
select|SELECT {return SELECT;}
from|FROM {return FROM;}
where|WHERE {return WHERE;}
group|GROUP {return GROUP;}
by|BY {return BY;}
insert|INSERT {return INSERT;}
into|INTO {return INTO;}
values|VALUES {return VALUES;}
create|CREATE {return CREATE;}
table|TABLE {return TABLE;}
drop|DROP {return DROP;}
char|CHAR {yylval.dataType = char_t; return DATATYPE;}
int|INT {yylval.dataType = int_t; return DATATYPE;}
float|FLOAT {yylval.dataType = float_t; return DATATYPE;}
double|DOUBLE {yylval.dataType = double_t; return DATATYPE;}

"'"."'" {
  yylval.Val = tc_malloc(sizeof(Val_t));
  yylval.Val->dataType = 1;
  yylval.Val->Value.char_val = yytext[1];
  return CHAR;
}
("+"|"-")?([0-9]+\.)?[0-9]+f {
  yylval.Val = tc_malloc(sizeof(Val_t));
  yylval.Val->dataType = 3;
  float temp = atof(yytext);
  yylval.Val->Value.float_val = temp;
  return FLOAT;
}
("+"|"-")?[0-9]+\.[0-9]+d {
  yylval.Val = tc_malloc(sizeof(Val_t));
  yylval.Val->dataType = 4;
  yylval.Val->Value.double_val = atof(yytext);
  return DOUBLE;
}
("+"|"-")?[0-9]+\.[0-9]+ {
  yylval.Val = tc_malloc(sizeof(Val_t));
  yylval.Val->dataType = 4;
  yylval.Val->Value.double_val = atof(yytext);
  return DOUBLE;
}
("+"|"-")?[0-9]+ {
  yylval.Val = tc_malloc(sizeof(Val_t));
  yylval.Val->dataType = 2;
  yylval.Val->Value.int_val = atoi(yytext);
  return INT;
}

AVG|avg { 
  yylval.Aggregation = tc_malloc(sizeof(Aggregation_t));
  yylval.Aggregation->AggregationType = avg;
  return AGGREGATION;
}

SUM|sum { 
  yylval.Aggregation = tc_malloc(sizeof(Aggregation_t));
  yylval.Aggregation->AggregationType = sum;
  return AGGREGATION;
}

COUNT|count { 
  yylval.Aggregation = tc_malloc(sizeof(Aggregation_t));
  yylval.Aggregation->AggregationType = count;
  return AGGREGATION;
}

MAX|max { 
  yylval.Aggregation = tc_malloc(sizeof(Aggregation_t));
  yylval.Aggregation->AggregationType = max;
  return AGGREGATION;
}

MIN|min { 
  yylval.Aggregation = tc_malloc(sizeof(Aggregation_t));
  yylval.Aggregation->AggregationType = min;
  return AGGREGATION;
}

[a-zA-Z_][a-zA-Z0-9_]* { 
  yylval.string_val = tc_strdup(yytext); 
  return STRING;
}

("||")|or|OR {
  yylval.string_val = tc_strdup(yytext);
  return OR;
}

("&&")|and|AND {
  yylval.string_val = tc_strdup(yytext); 
  return AND;
}

("=="|"!="|">="|"<="|">"|"<") { 
  yylval.string_val = tc_strdup(yytext); 
  return RELOP;
}

. {return yytext[0];}
%%
