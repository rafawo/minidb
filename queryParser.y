%{
#include <stdio.h>
#include <string.h>
#include "querySemanticAnalyzer.h"
#include "queryParser.tab.h"

#define YYDEBUG 0 //0 = no debug ---- !0 = debug

extern int yylex();
extern int yyparse();
extern FILE *yyin;

void yyerror(const char *s);
void endParser();

char *tableName;

%}

%union {
  int dataType;
  Val_p Val;
	char *string_val;
	Column_p Column;
	ConditionNode_p ConditionNode;
	Aggregation_p Aggregation;
};

%token SELECT FROM WHERE GROUP BY INSERT INTO VALUES CREATE TABLE DROP
%token <Aggregation> AGGREGATION
%token <string_val> STRING RELOP OR AND
%token <Val> CHAR INT FLOAT DOUBLE
%token <dataType> DATATYPE

%type <string_val> table_name values
%type <Column> column
%type <Val> val
%type <ConditionNode> condition condition_and conditionF

%%

query: 
      { 
        tableName = NULL;
        s_initSemanticAnalysis();
        tc_instanceThrashCan();
      } 
      query_body query_end
;

query_body: { s_startSelectQuery(); } select_query |
            { s_startInsertQuery(); } insert_query |
            { s_startCreateQuery(); } create_query |
            { s_startDropQuery(); } drop_query
;

query_end: ';'
			 {
			   if(s_finalAnalysis() != -1)
			   {
  			   printf("\n\nParse SUCCESS!!\n\n");
  			   tc_emptyThrashCan();
			   }
			   else
			   {
			     printf("\n\nParse FAIL!!\n\n");
  			   endParser();
			   }
			   
			   return;
			 }
;

drop_query: DROP TABLE table_name
  {
    deleteTable($3);
  }
;

create_query: create_body '(' create_columns ')'
;

create_body: CREATE TABLE STRING
  {
    if(exists($3))
    {
      printf("Table \'%s\' already exists!!!\n", $3);
      endParser();
      return;
    }
    
    createTable($3);
    tableName = $3;
  }
;

create_columns: new_column ',' create_columns | new_column
;

new_column: STRING DATATYPE
  {
    addColumn(tableName, $1, $2);
  }
;

insert_query: INSERT INTO table_name VALUES '(' values ')'
  {
    if(insertRow($3, $6) == -1)
    {
      printf("Could not insert Row: \'%s\'\ninto Table: \'%s\'\n", $6, $3);
      endParser();
      return;
    }
  }
;

values:
  val ',' values
  {
    $$ = tc_malloc(1000*sizeof(char));
    SPrintVal($$, $1);
    strcat($$, $3);
  } 
  | val
  {
    $$ = tc_malloc(1000*sizeof(char));
    SPrintVal($$, $1);
  }
;

select_query:
       {
         s_startSelect();
       }
       select
			 {
			   s_endSelect();
			   s_startFrom();
			   //printf("SELECT\n");
			 }
			 from
			 {
			   s_endFrom();
			   s_startWhere();
			   //printf("FROM\n");
			 }
			 where
			 {
			   s_endWhere();
			   s_startgroupby();
			   //printf("WHERE\n");
			 }
			 group_by
			 {
			   s_endgroupby();
			   //printf("GROUP BY\n");
			 }
;
 
select: SELECT select_body
;

from: FROM tables
;

where: /*empty rule*/ | WHERE condition
;

group_by: /*empty rule*/ |
          {
            if(s_isaggregationquery() == 0)
            {
              printf("Please don't use group by in a query with "
                     "no aggregation function(s)\n");
              endParser();
              return;
            } 
          }
          GROUP BY columns
          {
            s_groupedby();
          }
;

columns: column ',' columns | column
;
    
select_body: '*' {s_allColumns();} | s_body
;

s_body: s_element ',' s_body | s_element
;

s_element: column | { s_incomingaggregation(); } aggregation_function
;

aggregation_function: AGGREGATION '(' column ')'
                      {
                        s_turnaggregationflag();

				                $1->column = $3;
				                
				                s_addAggregation($1);
				                s_endaggregation();
                      }
;

tables: table_name ',' tables | table_name
;

condition: condition OR condition_and
         {
          $$ = tc_malloc(sizeof(ConditionNode_t));
          
          $$->Left = $1;
          $$->Right = $3;
          $$->NodeType = Logop;
          $$->Node.Logop_Node._logop = $2;
          $$->negation = 0;
          
          s_addNode($$);
         }
         | condition_and
         {
          $$ = $1;
          s_addNode($$);
         }
;

condition_and: condition_and AND conditionF
             {
              $$ = tc_malloc(sizeof(ConditionNode_t));
              
              $$->Left = $1;
              $$->Right = $3;
              $$->NodeType = Logop;
              $$->Node.Logop_Node._logop = $2;
              $$->negation = 0;
              
              s_addNode($$);
             }
             | conditionF
             {
              $$ = $1;
              s_addNode($$);
             }
;

conditionF: '(' condition ')'
          {
            $$ = $2;
          }
				  | '!' '(' condition ')'
				  {
				    $3->negation = 1;
				    $$ = $3;
				  }
				  | column RELOP column
				  {
				    if(s_AtFrom($1->TableName) == 0)
				    {
				      endParser();
				      return;
				    }
				    if($1->dataType != $3->dataType)
				    {
				      printf("Column \'%s\' and \'%s\' are not of same data type!!\n",
				             $1->ColumnName, $3->ColumnName);
				      endParser();
				      return;
				    }
				    //Columns are of same data type . . .
				    $$ = tc_malloc(sizeof(ConditionNode_t));
				    
				    $$->Node.ColRelopCol_Node._column1 = $1;
				    $$->Node.ColRelopCol_Node._column2 = $3;
				    $$->NodeType = ColRelopCol;
				    $$->negation = 0;
				    $$->Node.ColRelopCol_Node._relop = $2;
				    
				    $$->Left = NULL;
				    $$->Right = NULL;
				  }
				  | column RELOP val
				  {
				    if(s_AtFrom($1->TableName) == 0)
				    {
				      endParser();
				      return;
				    }
				    if($1->dataType != $3->dataType)
				    {
				      printf("Column \'%s\' and value are not of same data type!!\n",
				             $1->ColumnName);
				      endParser();
				      return;
				    }
				    //Columns are of same data type . . .
				    $$ = tc_malloc(sizeof(ConditionNode_t));
				    
				    $$->NodeType = ColRelopVal;
				    $$->negation = 0;
				    $$->Node.ColRelopVal_Node._column = $1;
				    $$->Node.ColRelopVal_Node._val = $3;
				    $$->Node.ColRelopVal_Node._relop = $2;
				    
				    $$->Left = NULL;
				    $$->Right = NULL;
				  }
;

column: table_name '.' STRING
       {
         //Check if given column name correspond to table.
         if(columnExists($1, $3))
         {
           int offset = 0;
           $$ = tc_malloc(sizeof(Column_t));
           $$->TableName = $1;
           $$->ColumnName = $3;
           $$->dataType = s_addColumn($$->TableName, $$->ColumnName, &offset);
           $$->table = getTable($$->TableName);
           $$->offset = offset;
         }
         else
         {
           printf("Table \'%s\' or column \'%s\' not defined!!\n",$1,$3);
           endParser();
           return;
         }
       }
;

table_name: STRING
            {
              //Check if Table actually exists.
              if(exists($1))
              {
                s_addTable($1);
                $$ = $1;
              }
              else
              {
                printf("Table \'%s\' doesn't exist!!\n", $1);
                endParser();
                return;
              }
            }
;

val
   : CHAR
   {
    $$ = $1;
   }
	 | INT
	 {
    $$ = $1;
	 }
	 | FLOAT
	 {
    $$ = $1;
	 }
	 | DOUBLE
	 {
    $$ = $1;
	 }
;

%%

/*
int main(int argc, char *argv[])
{
  yyparse();
}
*/

void endParser()
{
  tc_emptyThrashCan();
  s_endAnalysis();
}

void yyerror(const char *s)
{
  if(tableName != NULL)
  {
    deleteTable(tableName);
  }
  printf("ERROR!!: \'%s\'\n", s);
  endParser();
  return;
}
