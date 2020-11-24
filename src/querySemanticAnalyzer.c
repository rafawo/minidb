#ifndef QUERY_SEMANTIC_ANALYZER_C
#define QUERY_SEMANTIC_ANALYZER_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "querySemanticAnalyzer.h"

#include <emmintrin.h>

/*
Source file with the implementation of
semantic analyzer that's used in the
syntax analyzer.

RULES:

* You cannot make reference to a column
  which corresponding table is not present
  in the query.
* You cannot make reference to non existent
  tables.
* Operations in conditional expression should
  not "operate" condition with different data
  types.

To make it easier, we'll use our database to do a
temporal table named "@temp_table"

*/

#define INTRINSICS_LOOPS(type) ((32*sizeof(type)) / 16)

void s_endAnalysis();

//Flags for controlling query behaviour
int everything = 0;
int AtSelect = 0;
int AtFrom = 0;
int AtWhere = 0;
int AtGroupBy = 0;
int AggregationQuery = 0;
int GroupedBy = 0;
int incomingaggregation = 0;
int table_created = 0;

//All columns in this array of pointers to struct Column
//are the layout for the temporal table.
Column_p columns;
Aggregation_p *aggregations;
int ColumnCount = 0;
int AggregationCount = 0;
Column_p *columnsAtGroupBy;
int ColumnsAtGroupByCount = 0;

//Variable to set query type
int query_type = none;

//Tables in this array are the tables referenced at FROM
char (*tables)[MAX];
int TableCount = 0;

//Groups
Group_p Groups;
int GroupsCount = 0;

//Root
ConditionNode_p Root = NULL;

//Row representation of resulting table filter preparation
Row_p Row = NULL;
Row_p Last = NULL; //Variable to avoid traversing linked list each time a table is added

//Init Semantic Analyzer
void s_initSemanticAnalysis()
{
  //Make an instance of the thrashcan
  tc_instanceThrashCan();
  
  //Turn off all flags
  AtSelect = 0;
  AtFrom = 0;
  AtWhere = 0;
  AtGroupBy = 0;

  ColumnCount = 0;
  TableCount = 0;
  ColumnsAtGroupByCount = 0;
  AggregationCount = 0;
  GroupsCount = 0;

  everything = 0;
  AggregationQuery = 0;
  GroupedBy = 0;
  incomingaggregation = 0;
  
  query_type = none;

  //Set to NULL all global pointers
  Root = NULL;
  Row = NULL;
  Last = NULL;
  columns = NULL;
  tables = NULL;
  columnsAtGroupBy = NULL;
  Groups = NULL;
}

void s_startSelectQuery()
{
  query_type = select_query;
  createTable(TEMP); //add temporal table to miniDB
  table_created = 1;
}

void s_startInsertQuery()
{
  query_type = insert_query;
}

void s_startCreateQuery()
{
  query_type = create_query;
}

void s_startDropQuery()
{
  query_type = drop_query;
}

void s_addNode(ConditionNode_p node)
{
  Root = node;
}

//Select all columns from resulting table
void s_allColumns()
{
  everything = 1;
}

void s_startSelect()
{
  AtSelect = 1;
}

void s_endSelect()
{
  AtSelect = 0;
}

void s_startFrom()
{
  AtFrom = 1;
}

void s_endFrom()
{
  AtFrom = 0;
}

void s_startWhere()
{
  AtWhere = 1;
}

void s_endWhere()
{
  AtWhere = 0;
}

void s_startgroupby()
{
  AtGroupBy = 1;
}

void s_endgroupby()
{
  AtGroupBy = 0;
}

void s_turnaggregationflag()
{
  AggregationQuery = 1;
}

int s_isaggregationquery()
{
  return AggregationQuery;
}

void s_groupedby()
{
  GroupedBy = 1;
}

void s_incomingaggregation()
{
  incomingaggregation = 1;
}

void s_endaggregation()
{
  incomingaggregation = 0;
}

//Returns column data type
int s_addColumn(const char *table_name, const char* column_name, int *_offset)
{
  int offset = 0;
  int dataType = getColumnDataType(table_name, column_name, &offset);
  *_offset = offset;
  
  if(AtSelect)
  {
    //SELECT
    if(everything)
      return 0;
      
    if(dataType > 0)
    {
      ColumnCount++;
      columns = tc_realloc(columns, sizeof(Column_t)*ColumnCount);
      aggregations = tc_realloc(aggregations, sizeof(Aggregation_p)*ColumnCount);
      
      columns[ColumnCount - 1].TableName = tc_strdup(table_name);
      columns[ColumnCount - 1].ColumnName = tc_strdup(column_name);
      columns[ColumnCount - 1].dataType = dataType;
      columns[ColumnCount - 1].offset = offset;
      columns[ColumnCount - 1].table = getTable(table_name);
      aggregations[ColumnCount - 1] = NULL;
      
      //Add dataType to temporal Column
      addColumn(TEMP, column_name, ( incomingaggregation ==  1? double_t : dataType));
      
      return dataType;
    }
  }
  else if(AtWhere)
  {
    //WHERE
    //Columns referenced at WHERE section are referenced for
    //condition evaluation (Filter).
    
    //we don't need to add column . . .
    //If we are at this point of the code we know column and table does exist 
    //because of the columnExists(table_name, column_name) call prior to this call.
    
    //what we have to do is return column data type
    return dataType;
  }
  else if(AtGroupBy)
  {
    //Making reference to a Column at Group By
    ColumnsAtGroupByCount += 1;
    columnsAtGroupBy = tc_realloc(columnsAtGroupBy,
                                        sizeof(Column_p)*ColumnsAtGroupByCount);
    
    columnsAtGroupBy[ColumnsAtGroupByCount - 1] = tc_malloc(sizeof(Column_t));
    if(columnsAtGroupBy[ColumnsAtGroupByCount - 1] == NULL)
    {
      return 0;
    }
    
    columnsAtGroupBy[ColumnsAtGroupByCount - 1]->TableName = tc_strdup(table_name);
    columnsAtGroupBy[ColumnsAtGroupByCount - 1]->ColumnName = tc_strdup(column_name);
    columnsAtGroupBy[ColumnsAtGroupByCount - 1]->dataType = dataType;
    columnsAtGroupBy[ColumnsAtGroupByCount - 1]->offset = offset;
    columnsAtGroupBy[ColumnsAtGroupByCount - 1]->table = getTable(table_name);
    
    return dataType;
  }
  //No Column reference in FROM section
  
  return 0;
}

void s_addTable(const char* table_name)
{
  if(AtSelect)
  {
    //SELECT
    //Tables referenced at SELECT don't have to be added
    //since FROM decides who is in query and who is not.
  }
  else if(AtWhere)
  {
    //WHERE
    //Tables referenced at WHERE don't have to be added.
  }
  else if(AtFrom)
  {
    //FROM
    TableCount++;
    tables = tc_realloc(tables,sizeof(char[MAX])*TableCount);
    strcpy(tables[TableCount - 1],table_name);
    
    //Add row representation
    if(Last == NULL)
    {
      Row = tc_malloc(sizeof(Row_t));
      Row->table = getTable(table_name);
      Row->next = NULL;
      Row->rowID = 0;
      Row->page = NULL;
      Last = Row;
    }
    else
    {
      Last->next = tc_malloc(sizeof(Row_t));
      Last->next->table = getTable(table_name);
      Last->next->next = NULL;
      Last->next->rowID = 0;
      Last->next->page = NULL;
      Last = Last->next;
    }
  }
}

void s_addAggregation(Aggregation_p aggregation)
{
  //Add Aggregation Function
  aggregations[ColumnCount - 1] = aggregation;
  columns[ColumnCount - 1].dataType = double_t;
  AggregationCount += 1;  
}

int s_AtFrom(const char *table_name)
{
  int j;
  for(j = 0;j < TableCount;++j)
  {
    if(strcmp(table_name, tables[j]) == 0)
      return 1;
  }
  //Not Found
  printf("Table \'%s\' not in FROM!!\n", table_name);
  return 0;
}

int s_AtGroupBy(Column_p column)
{
  int i = 0;
  for(i = 0;i < ColumnsAtGroupByCount;++i)
  {
    int comparison = strcmp(column->TableName, columnsAtGroupBy[i]->TableName) == 0 &&
                     strcmp(column->ColumnName, columnsAtGroupBy[i]->ColumnName) == 0;
    if(comparison == 1)
      return 1;
  }
  //Not in from
  return 0;
}

char* s_getStrFromColumnRow(Column_p column, Row_p row)
{
  //column makes reference to the table-column from where we have to obtain the data
  //row tell us which row in which page we can obtain that data
  
  //Search in each row one that have the same table as the column
  //descriptor
  int dataTypeSize = 0;
  Row_p actual = row;
  char *temp = tc_malloc(sizeof(char)*20);
  while(actual != NULL)
  {
    if(actual->table == column->table)
    {
      DataTypeSize(column->dataType, dataTypeSize);
      SPrintData(temp,
                 column->dataType,
                 &actual->page->data[column->offset*32 + 
                                    actual->rowID*dataTypeSize]);
      break;
    }
    actual = actual->next;
  }
  
  return temp;
}

Row_p s_rowdup(Row_p row)
{
  //Alloc memory for new row node
  Row_p ret = tc_malloc(sizeof(Row_t));
  ret->table = row->table;
  ret->page = row->page;
  ret->rowID = row->rowID;
  if(row->next == NULL)
  {
    ret->next == NULL;
  }
  else
  {
    ret->next = s_rowdup(row->next); //recursive call to duplicate
                                     //complete linked list
  }
  
  return ret;
}

void s_freeRows(Row_p row, int level)
{
  if(row->next != NULL && level != TableCount)
  {
    s_freeRows(row->next, level + 1);
  }
  
  free(row);
}

void s_assignGroupToRow(Row_p row)
{
  char id[STR_MAX] = {'\0'};
  int i = 0;
  if(GroupedBy == 1)
  {
    for(i = 0;i < ColumnsAtGroupByCount;++i)
    {
      strcat(id, s_getStrFromColumnRow(columnsAtGroupBy[i], row));
    }
  }
  
  //Search if this id is already in a existent group
  int found = 0;
  int index = 0;
  for(i = 0;i < GroupsCount;++i)
  {
    if(strcmp(Groups[i].id, id) == 0)
    {
      //This row belongs to this existent Group
      index = i;
      found = 1;
      break;
    }
  }
  
  //This id does not exist, so we create a new Group
  if(found == 0)
  {
    GroupsCount += 1;
    Groups = tc_realloc(Groups, sizeof(Group_t)*GroupsCount);
    
    Groups[GroupsCount - 1].id = tc_strdup(id);
    Groups[GroupsCount - 1].RowCount = 0;
    Groups[GroupsCount - 1].rows = NULL;
    
    index = GroupsCount - 1;
    
  }
  
  //Add this row
  Groups[index].RowCount += 1;
  Groups[index].rows = tc_realloc(Groups[index].rows, sizeof(Row_p)*Groups[index].RowCount);
  
  Groups[index].rows[Groups[index].RowCount - 1] = s_rowdup(row);
}

void s_insertRowToTempTable(Row_p row)
{
  char temp[STR_MAX] = {'\0'};
  char temp2[1000] = {'\0'};

  int Null_n = 0;
  int j = 0;

  //The SELECT layout is in the columns specifications
  int i;
  int dataTypeSize = 0;
  for(i = 0;i < ColumnCount;++i)
  {
    //Search in each row one that have the same table as the column
    //descriptor
    Row_p actual = row;
    while(actual != NULL)
    {
      if(actual->table == columns[i].table)
      {
        DataTypeSize(columns[i].dataType, dataTypeSize);
        SPrintData(temp,
                   columns[i].dataType,
                   &actual->page->data[columns[i].offset*32 + 
                                      actual->rowID*dataTypeSize]);
        //***
        //strcat(temp2, temp);
        
        for(j = 0;Null_n < 1000 - 1 && j < STR_MAX && temp[j] != '\0';j++, Null_n++)
        {
          temp2[Null_n] = temp[j];
        }
        temp2[Null_n] = '\0';
        //***
        break;
      }
      actual = actual->next;
    }
  }

  insertRow(TEMP, temp2);
}

void s_evaluateAVG(Aggregation_p aggregation, Row_p *rows, const int RowCount, Val_p _AVG)
{
  int i;
  int dataTypeSize = 0;
  char *data;
  int count = 0;
  
  _AVG->Value.double_val = 0; //Biggest Type set to 0 in union
  
  //Iterate in complete set of rows
  for(i = 0;i < RowCount;++i)
  {
    Row_p actual = rows[i];
    while(actual != NULL)
    {
      if(actual->table == aggregation->column->table)
      {
        data = (actual->page->data) + aggregation->column->offset*32;
        DataTypeSize(aggregation->column->dataType, dataTypeSize);
        data = data + actual->rowID*dataTypeSize;
        
        switch(aggregation->column->dataType)
        {
          case 0: break;
          case char_t: _AVG->Value.double_val += (double)(*(char*)data); count++; break;
          case int_t: _AVG->Value.double_val += (double)(*(int*)data); count++; break;
          case float_t: _AVG->Value.double_val += (double)(*(float*)data); count++; break;
          case double_t: _AVG->Value.double_val += *(double*)data; count++; break;
          default: break;
        }
        
        break;
      }
      actual = actual->next;
    }    
  }
  
  _AVG->Value.double_val /= count;
}

void s_evaluateSUM(Aggregation_p aggregation, Row_p *rows, const int RowCount, Val_p _SUM)
{
  int i;
  int dataTypeSize = 0;
  char *data;
  
  _SUM->Value.double_val = 0; //Biggest Type set to 0 in union
  
  //Iterate in complete set of rows
  for(i = 0;i < RowCount;++i)
  {
    Row_p actual = rows[i];
    while(actual != NULL)
    {
      if(actual->table == aggregation->column->table)
      {
        data = (actual->page->data) + aggregation->column->offset*32;
        DataTypeSize(aggregation->column->dataType, dataTypeSize);
        data = data + actual->rowID*dataTypeSize;
        
        switch(aggregation->column->dataType)
        {
          case 0: break;
          case char_t: _SUM->Value.double_val += (double)(*(char*)data); break;
          case int_t: _SUM->Value.double_val += (double)(*(int*)data); break;
          case float_t: _SUM->Value.double_val += (double)(*(float*)data); break;
          case double_t: _SUM->Value.double_val += *(double*)data; break;
          default: break;
        }
        
        break;
      }
      actual = actual->next;
    }    
  }
}

void s_evaluateCOUNT(Aggregation_p aggregation, Row_p *rows, const int RowCount, Val_p _COUNT)
{
  int i;
  int dataTypeSize = 0;
  
  _COUNT->Value.double_val = 0; //Biggest Type set to 0 in union
  
  //Iterate in complete set of rows
  for(i = 0;i < RowCount;++i)
  {
    Row_p actual = rows[i];
    while(actual != NULL)
    {
      if(actual->table == aggregation->column->table)
      {
        switch(aggregation->column->dataType)
        {
          case 0: break;
          case char_t: _COUNT->Value.double_val += (double)1; break;
          case int_t: _COUNT->Value.double_val += (double)1; break;
          case float_t: _COUNT->Value.double_val += (double)1; break;
          case double_t: _COUNT->Value.double_val += (double)1; break;
          default: break;
        }
        
        break;
      }
      actual = actual->next;
    }    
  }  
}

void s_evaluateMAX(Aggregation_p aggregation, Row_p *rows, const int RowCount, Val_p _MAX)
{
  int i;
  int dataTypeSize = 0;
  char *data;
  
  int found_first = 0;
  _MAX->Value.double_val = 0; //Biggest Type set to 0 in union
  
  //Iterate in complete set of rows
  for(i = 0;i < RowCount;++i)
  {
    Row_p actual = rows[i];
    while(actual != NULL)
    {
      if(actual->table == aggregation->column->table)
      {
        data = (actual->page->data) + aggregation->column->offset*32;
        DataTypeSize(aggregation->column->dataType, dataTypeSize);
        data = data + actual->rowID*dataTypeSize;
        
        switch(aggregation->column->dataType)
        {
          case 0: break;
          case char_t: 
            if(found_first == 0 || (double)(*(char*)data) > _MAX->Value.double_val)
            {
              _MAX->Value.double_val = (double)(*(char*)data);
            }
            found_first = 1;
            break;
            
          case int_t:
            if(found_first == 0 || (double)(*(int*)data) > _MAX->Value.double_val)
            {
              _MAX->Value.double_val = (double)(*(int*)data);
            }
            found_first = 1;
            break;
            
          case float_t:
            if(found_first == 0 || (double)(*(float*)data) > _MAX->Value.double_val)
            {
              _MAX->Value.double_val = (double)(*(float*)data);
            }
            found_first = 1;
            break;
            
          case double_t:
            if(found_first == 0 || *(double*)data > _MAX->Value.double_val)
            {
              _MAX->Value.double_val = *(double*)data;
            }
            found_first = 1;
            break;
            
          default: break;
        }
        
        break;
      }
      actual = actual->next;
    }    
  }  
}

void s_evaluateMIN(Aggregation_p aggregation, Row_p *rows, const int RowCount, Val_p _MIN)
{
  int i;
  int dataTypeSize = 0;
  char *data;
  
  int found_first = 0;
  _MIN->Value.double_val = 0; //Biggest Type set to 0 in union
  
  //Iterate in complete set of rows
  for(i = 0;i < RowCount;++i)
  {
    Row_p actual = rows[i];
    while(actual != NULL)
    {
      if(actual->table == aggregation->column->table)
      {
        data = (actual->page->data) + aggregation->column->offset*32;
        DataTypeSize(aggregation->column->dataType, dataTypeSize);
        data = data + actual->rowID*dataTypeSize;
        
        switch(aggregation->column->dataType)
        {
          case 0: break;
          case char_t: 
            if(found_first == 0 || (double)(*(char*)data) < _MIN->Value.double_val)
            {
              _MIN->Value.double_val = (double)(*(char*)data);
            }
            found_first = 1;
            break;
            
          case int_t:
            if(found_first == 0 || (double)(*(int*)data) < _MIN->Value.double_val)
            {
              _MIN->Value.double_val = (double)(*(int*)data);
            }
            found_first = 1;
            break;
            
          case float_t:
            if(found_first == 0 || (double)(*(float*)data) < _MIN->Value.double_val)
            {
              _MIN->Value.double_val = (double)(*(float*)data);
            }
            found_first = 1;
            break;
            
          case double_t:
            if(found_first == 0 || *(double*)data < _MIN->Value.double_val)
            {
              _MIN->Value.double_val = *(double*)data;
            }
            found_first = 1;
            break;
            
          default: break;
        }
        
        break;
      }
      actual = actual->next;
    }    
  }  
}

Val_p s_evaluateAggregation(Aggregation_p aggregation, Row_p *rows, const int RowCount)
{
  Val_p val = tc_malloc(sizeof(Val_t));
  
  val->dataType = double_t;
  
  //Call to the corresponding evaluation function
  //depending on the aggregaton type
  switch(aggregation->AggregationType)
  {
    case avg: s_evaluateAVG(aggregation, rows, RowCount, val); break;
    case sum: s_evaluateSUM(aggregation, rows, RowCount, val); break;
    case count: s_evaluateCOUNT(aggregation, rows, RowCount, val); break;
    case max: s_evaluateMAX(aggregation, rows, RowCount, val); break;
    case min: s_evaluateMIN(aggregation, rows, RowCount, val); break;
    default: break;
  }
  
  return val;
}

void s_insertRowsFromGroupsNonInstrinsics()
{
  char rowStr[STR_MAX] = {'\0'};
  char temp[STR_MAX];

  //In the insertion of rows for the groups
  //we just insert ONE row per group

  //For each Group:
  int group_i = 0;
  for(group_i = 0;group_i < GroupsCount;++group_i)
  {
    Group_p group = &Groups[group_i];
    //We are in actual group, we have to insert Row based on select layout
    int i, dataTypeSize = 0;
    for(i = 0;i < ColumnCount;++i)
    {
      Column_p col = &columns[i];
      //We are in i'th position of SELECT layout
      //so if this position is an aggregate function we have to evaluate it
      if(aggregations[i] != NULL)
      {
        Val_p val = s_evaluateAggregation(aggregations[i], group->rows, group->RowCount);
        SPrintVal(temp, val);
      }
      //If not, retrieve from any row the value for this Column
      else
      {
        Row_p actual = group->rows[0];
        while(actual != NULL)
        {
          if(actual->table == col->table)
          {
            DataTypeSize(col->dataType, dataTypeSize);
            SPrintData(temp,
                       col->dataType,
                       &actual->page->data[col->offset*32 + 
                                          actual->rowID*dataTypeSize]);
            break;
          }
          actual = actual->next;
        }
      }
      
      strcat(rowStr, temp);
    }
    
    insertRow(TEMP, rowStr); //Just ONE row per group
    rowStr[0] = '\0';
  }
}

void s_insertRowsFromGroupsInstrinsics()
{
  //NOT IMPLEMENTED
}

void s_insertRowsFromGroups()
{
#ifdef INTRINSICS
  s_insertRowsFromGroupsInstrinsics();
#else
  s_insertRowsFromGroupsNonInstrinsics();
#endif
}

//------------------------------------------------------------------------------------------

int s_ColRelopValEvaluation(char *data1, Val_p data2, char *relop, int dataType)
{
  //Evaluate this node based on Relop logic
  if(strcmp(relop,"==") == 0)
  {
    switch(dataType)
    {
      case char_t:
        return (*(char*)data1) == data2->Value.char_val;
      case int_t:
        return (*(int*)data1) == data2->Value.int_val;
      case float_t:
        return (*(float*)data1) == data2->Value.float_val;
      case double_t:
        return (*(double*)data1) == data2->Value.double_val;
    }
  }
  else if(strcmp(relop,"!=") == 0)
  {
    switch(dataType)
    {
      case char_t:
        return (*(char*)data1) != data2->Value.char_val;
      case int_t:
        return (*(int*)data1) != data2->Value.int_val;
      case float_t:
        return (*(float*)data1) != data2->Value.float_val;
      case double_t:
        return (*(double*)data1) != data2->Value.double_val;
    }
  }
  else if(strcmp(relop,">=") == 0)
  {
    switch(dataType)
    {
      case char_t:
        return (*(char*)data1) >= data2->Value.char_val;
      case int_t:
        return (*(int*)data1) >= data2->Value.int_val;
      case float_t:
        return (*(float*)data1) >= data2->Value.float_val;
      case double_t:
        return (*(double*)data1) >= data2->Value.double_val;
    }
  }
  else if(strcmp(relop,"<=") == 0)
  {
    switch(dataType)
    {
      case char_t:
        return (*(char*)data1) <= data2->Value.char_val;
      case int_t:
        return (*(int*)data1) <= data2->Value.int_val;
      case float_t:
        return (*(float*)data1) <= data2->Value.float_val;
      case double_t:
        return (*(double*)data1) <= data2->Value.double_val;
    }
  }
  else if(strcmp(relop,">") == 0)
  {
    switch(dataType)
    {
      case char_t:
        return (*(char*)data1) > data2->Value.char_val;
      case int_t:
        return (*(int*)data1) > data2->Value.int_val;
      case float_t:
        return (*(float*)data1) > data2->Value.float_val;
      case double_t:
        return (*(double*)data1) > data2->Value.double_val;
    }
  }
  else if(strcmp(relop,"<") == 0)
  {
    switch(dataType)
    {
      case char_t:
        return (*(char*)data1) < data2->Value.char_val;
      case int_t:
        return (*(int*)data1) < data2->Value.int_val;
      case float_t:
        return (*(float*)data1) < data2->Value.float_val;
      case double_t:
        return (*(double*)data1) < data2->Value.double_val;
    }
  }
  
  return 0;
}

int s_ColRelopColEvaluation(char *data1, char *data2, char *relop, int dataType)
{
  //Evaluate this node based on Relop logic
  if(strcmp(relop,"==") == 0)
  {
    switch(dataType)
    {
      case char_t:
        return (*(char*)data1) == (*(char*)data2);
      case int_t:
        return (*(int*)data1) == (*(int*)data2);
      case float_t:
        return (*(float*)data1) == (*(float*)data2);
      case double_t:
        return (*(double*)data1) == (*(double*)data2);
    }
  }
  else if(strcmp(relop,"!=") == 0)
  {
    switch(dataType)
    {
      case char_t:
        return (*(char*)data1) != (*(char*)data2);
      case int_t:
        return (*(int*)data1) != (*(int*)data2);
      case float_t:
        return (*(float*)data1) != (*(float*)data2);
      case double_t:
        return (*(double*)data1) != (*(double*)data2);
    }
  }
  else if(strcmp(relop,">=") == 0)
  {
    switch(dataType)
    {
      case char_t:
        return (*(char*)data1) >= (*(char*)data2);
      case int_t:
        return (*(int*)data1) >= (*(int*)data2);
      case float_t:
        return (*(float*)data1) >= (*(float*)data2);
      case double_t:
        return (*(double*)data1) >= (*(double*)data2);
    }
  }
  else if(strcmp(relop,"<=") == 0)
  {
    switch(dataType)
    {
      case char_t:
        return (*(char*)data1) <= (*(char*)data2);
      case int_t:
        return (*(int*)data1) <= (*(int*)data2);
      case float_t:
        return (*(float*)data1) <= (*(float*)data2);
      case double_t:
        return (*(double*)data1) <= (*(double*)data2);
    }
  }
  else if(strcmp(relop,">") == 0)
  {
    switch(dataType)
    {
      case char_t:
        return (*(char*)data1) > (*(char*)data2);
      case int_t:
        return (*(int*)data1) > (*(int*)data2);
      case float_t:
        return (*(float*)data1) > (*(float*)data2);
      case double_t:
        return (*(double*)data1) > (*(double*)data2);
    }
  }
  else if(strcmp(relop,"<") == 0)
  {
    switch(dataType)
    {
      case char_t:
        return (*(char*)data1) < (*(char*)data2);
      case int_t:
        return (*(int*)data1) < (*(int*)data2);
      case float_t:
        return (*(float*)data1) < (*(float*)data2);
      case double_t:
        return (*(double*)data1) < (*(double*)data2);
    }
  }
  
  return 0;
}

int s_evaluateNode(ConditionNode_p node, Row_p row)
{
  int ret = 0;
  //Leaf ConditionNode. Possible node types {ColRelopCol, ColRelopVal}
  if(node->NodeType == ColRelopCol)
  {
    Column_p column1 = node->Node.ColRelopCol_Node._column1;
    Column_p column2 = node->Node.ColRelopCol_Node._column2;
    
    //Get data
    char *data1 = NULL;
    char *data2 = NULL;
    
    //Search for corresponding table in row for column1 and column2
    Row_p row1 = NULL, row2 = NULL;
    Row_p actual = row;
    while(actual != NULL)
    {
      if(actual->table == column1->table)
        row1 = actual;
      if(actual->table == column2->table)
        row2 = actual;
      
      if(row1 != NULL && row2 != NULL)
        break;
        
      actual = actual->next;
    }
    
    if(row1 == NULL || row2 == NULL)
      return 0;
    
    int dataTypeSize = 0;
    
    //Got corresponding Row_p
    data1 = (row1->page->data) + column1->offset*32; //Base of corresponding column
    DataTypeSize(column1->dataType, dataTypeSize);
    data1 = data1 + row1->rowID*dataTypeSize; //Offset on actual column
#ifdef DEBUG
    PrintData(column1->dataType, data1);
    printf("\n");
#endif
    
    data2 = (row2->page->data) + column2->offset*32; //Base of corresponding column
    DataTypeSize(column2->dataType, dataTypeSize);
    data2 = data2 + row2->rowID*dataTypeSize; //Offset on actual column
#ifdef DEBUG
    PrintData(column2->dataType, data2);
    printf("\n");
#endif
    ret = s_ColRelopColEvaluation(data1,
                                  data2,
                                  node->Node.ColRelopCol_Node._relop,
                                  column2->dataType);
    
    return (node->negation == 1)? !ret : ret;
  }
  else if(node->NodeType == ColRelopVal)
  {
    Column_p column = node->Node.ColRelopVal_Node._column;
    Val_p val = node->Node.ColRelopVal_Node._val;
    
    //Get Data
    char *data = NULL;
    
    //Search for corresponding table in row for column1 and column2
    Row_p _row = NULL;
    Row_p actual = row;
    while(actual != NULL)
    {
      if(actual->table == column->table)
        _row = actual;

      if(_row != NULL)
        break;
        
      actual = actual->next;
    }
    
    if(_row == NULL)
      return 0;
    
    int dataTypeSize = 0;
    
    //Got corresponding Row_p
    data = (_row->page->data) + column->offset*32; //Base of corresponding column
    DataTypeSize(column->dataType, dataTypeSize);
    data = data + _row->rowID*dataTypeSize; //Offset on actual column
#ifdef DEBUG
    PrintData(column->dataType, data);
    printf("\n");
#endif    

    ret = s_ColRelopValEvaluation(data,
                                  val,
                                  node->Node.ColRelopVal_Node._relop,
                                  val->dataType);
    
    return (node->negation == 1)? !ret : ret;
  }
  else
  {
    return ret;
  }
}

int s_evaluateQuery(ConditionNode_p node, Row_p row)
{
  if(node == NULL)
    return 0;
  
  if(node->Left == NULL || node->Right == NULL)
  {
    return s_evaluateNode(node, row);
  }
  
  //Not-Leaf ConditionNode. It HAS to be Logop node type
  if(node->NodeType != Logop)
    return 0;
    
  //Evaluate this node based on Logop logic
  int left = 0, right = 0;
  left = s_evaluateQuery(node->Left, row);
  right = s_evaluateQuery(node->Right, row);
  int temp = 0;
  if(strcmp(node->Node.Logop_Node._logop, "&&") == 0)
  {
    temp = left && right;
    return (node->negation == 1)? !temp : temp;
  }
  else if(strcmp(node->Node.Logop_Node._logop, "||") == 0)
  {
    temp = left || right;
    return (node->negation == 1)? !temp : temp;
  }
  else
  {
    return 0;
  }
}

void s_evaluate(Row_p row)
{
  int passed = 0;
  
  if(Root == NULL) //If there's no condition to evaluate (empty where clause)
    passed = 1;
  else if(s_evaluateQuery(Root, row)) //or the evaluation was successful
    passed = 1;
  
  //We do the evaluation of a row based on the tree created at the query
  if(passed) 
  {
    //Row has passed succesfully filter

    if(s_isaggregationquery() == 1)
    {
      //We need to add this row to the corresponding group
      //So first we retrieve id for this row . . .
      s_assignGroupToRow(row);
    }
    else
    {
      //We need to add this row to
      //the temp table with the specified layout (SELECT)
      s_insertRowToTempTable(row);
    }
  }
  
  return;
}

void s_joinAndFilterNoIntrinsics(Row_p row)
{
  //******* Iterative version *******************************************
  
  int finished = 0;
  int _continue = 0;
  Row_p temp = NULL;
  
  //Init pages
  for(temp = Row;temp != NULL;temp = temp->next)
  {
    temp->page = temp->table->hdr->firstPage;
    if(temp->page == NULL)
      return;
  }
  
  do{
    //Evaluate if current row valid
    for(temp = Row;
        temp != NULL && (((temp->page->bitmap) >> (temp->rowID))&1) == 1;
        temp = temp->next);
    if(temp == NULL)
    {
      s_evaluate(Row);
    }
    
    //Calculate next valid row
    for(temp = Row, _continue = 1;temp != NULL && _continue == 1;temp = temp->next)
    {
      //Move to next valid row
      do{
        temp->rowID += 1;
      }while(temp->rowID < 32 && (((temp->page->bitmap) >> (temp->rowID))&1) == 0);
      
      _continue = temp->page->nextPage == NULL && temp->rowID >= 32;
      
      //Page change
      temp->page = temp->rowID >= 32? temp->page->nextPage : temp->page;
      
      //rowID overflow
      temp->rowID = temp->rowID >= 32? 0 : temp->rowID;
      
      //Page overflow
      temp->rowID = temp->page == NULL? 0 : temp->rowID;
      temp->page = temp->page == NULL? temp->table->hdr->firstPage : temp->page;
    }
    
    //Check if we have finished
    for(temp = Row;
        temp != NULL && temp->rowID == 0 && temp->page == temp->table->hdr->firstPage;
        temp = temp->next);
    
    finished = (temp == NULL); //true if all in initial conditions
  }while(!finished);
  
  /***********************************************************************/

  /******** Recursive version ********************************************
  
  //If row is NULL it means we have finished doing the recursive 
  //calls on the join.
  //(this will be called #T(X #T)* times where T is a table in the FROM)
  if(row == NULL)
  {
    s_evaluate(Row); //Filter
    return;
  }
    
  //At this point we have to recurse for each row that exists
  page_p page = row->table->hdr->firstPage;
  while(page != NULL)
  {
    row->rowID = 0;
    row->page = page;
    for(row->rowID = 0;row->rowID < 32;row->rowID++)
    {
      if((((page->bitmap) >> (row->rowID))&1) == 1)
      {
        //Row exists, enter again!!!
        s_joinAndFilterNoIntrinsics(row->next);
      }
    }
    page = page->nextPage;
  }
  
  /*************************************************************************/
}

int s_ColRelopValEvaluationIntrinsics(char *data1, Val_p data2, char *relop, int dataType)
{
	int bitmap = 0;

	//32 elements per page

	if(dataType == char_t) //move by 16
	{
			__m128i result;
			__m128i *d1 = (__m128i*)data1;
			__m128i d2 = _mm_set1_epi8(data2->Value.char_val);
			
			if(strcmp(relop,"==") == 0)
			{
				result = _mm_cmpeq_epi8(d1[0], d2);
				bitmap |= _mm_movemask_epi8(result);
				result = _mm_cmpeq_epi8(d1[1], d2);
				bitmap |= _mm_movemask_epi8(result) << 16;
			}
			else if(strcmp(relop,"!=") == 0)
			{
				result = _mm_cmpeq_epi8(d1[0], d2);
				bitmap |= ~(_mm_movemask_epi8(result)) & 65535;
				result = _mm_cmpeq_epi8(d1[1], d2);
				bitmap |= (~(_mm_movemask_epi8(result)) & 65535) << 16;
			}
			else if(strcmp(relop,">=") == 0)
			{
				result = _mm_or_si128(_mm_cmpeq_epi8(d1[0],d2),_mm_cmpgt_epi8(d1[0],d2));
				bitmap |= _mm_movemask_epi8(result);
				result = _mm_or_si128(_mm_cmpeq_epi8(d1[1],d2),_mm_cmpgt_epi8(d1[1],d2));
				bitmap |= _mm_movemask_epi8(result) << 16;
			}
			else if(strcmp(relop,"<=") == 0)
			{
				result = _mm_or_si128(_mm_cmpeq_epi8(d1[0],d2),_mm_cmplt_epi8(d1[0],d2));
				bitmap |= _mm_movemask_epi8(result);
				result = _mm_or_si128(_mm_cmpeq_epi8(d1[1],d2),_mm_cmplt_epi8(d1[1],d2));
				bitmap |= _mm_movemask_epi8(result) << 16;
			}
			else if(strcmp(relop,">") == 0)
			{
				result = _mm_cmpgt_epi8(d1[0], d2);
				bitmap |= _mm_movemask_epi8(result);
				result = _mm_cmpgt_epi8(d1[1], d2);
				bitmap |= _mm_movemask_epi8(result) << 16;
			}
			else if(strcmp(relop,"<") == 0)
			{
				result = _mm_cmplt_epi8(d1[0], d2);
				bitmap |= _mm_movemask_epi8(result);
				result = _mm_cmplt_epi8(d1[1], d2);
				bitmap |= _mm_movemask_epi8(result) << 16;
			}
	}
	else if(dataType == int_t || dataType == float_t) //move by 4
	{
			__m128 result;
			__m128 *d1 = (__m128*)data1;
			__m128 d2 = _mm_set1_ps(data2->Value.int_val);
			
			if(strcmp(relop,"==") == 0)
			{
				int i = 0;
				for(i = 0;i < 8;i++)
				{
					result = _mm_cmpeq_ps(d1[i],d2);
					bitmap |= _mm_movemask_ps(result) << (4*i);
				}
			}
			else if(strcmp(relop,"!=") == 0)
			{
				int i = 0;
				for(i = 0;i < 8;i++)
				{
					result = _mm_cmpneq_ps(d1[i],d2);
					bitmap |= _mm_movemask_ps(result) << (4*i);
				}
			}
			else if(strcmp(relop,">=") == 0)
			{
				int i = 0;
				for(i = 0;i < 8;i++)
				{
					result = _mm_cmpge_ps(d1[i],d2);
					bitmap |= _mm_movemask_ps(result) << (4*i);
				}
			}
			else if(strcmp(relop,"<=") == 0)
			{
				int i = 0;
				for(i = 0;i < 8;i++)
				{
					result = _mm_cmple_ps(d1[i],d2);
					bitmap |= _mm_movemask_ps(result) << (4*i);
				}
			}
			else if(strcmp(relop,">") == 0)
			{
				int i = 0;
				for(i = 0;i < 8;i++)
				{
					result = _mm_cmpgt_ps(d1[i],d2);
					bitmap |= _mm_movemask_ps(result) << (4*i);
				}
			}
			else if(strcmp(relop,"<") == 0)
			{
				int i = 0;
				for(i = 0;i < 8;i++)
				{
					result = _mm_cmplt_ps(d1[i],d2);
					bitmap |= _mm_movemask_ps(result) << (4*i);
				}
			}
	}
	else if(dataType == double_t) //move by 2
	{
			__m128d result;
			__m128d *d1 = (__m128d*)data1;
			__m128d d2 = _mm_set1_pd(data2->Value.double_val);
			
			if(strcmp(relop,"==") == 0)
			{
				int i = 0;
				for(i = 0;i < 16;i++)
				{
					result = _mm_cmpeq_pd(d1[i],d2);
					bitmap |= _mm_movemask_pd(result) << (2*i);
				}
			}
			else if(strcmp(relop,"!=") == 0)
			{
				int i = 0;
				for(i = 0;i < 16;i++)
				{
					result = _mm_cmpneq_pd(d1[i],d2);
					bitmap |= _mm_movemask_pd(result) << (2*i);
				}
			}
			else if(strcmp(relop,">=") == 0)
			{
				int i = 0;
				for(i = 0;i < 16;i++)
				{
					result = _mm_cmpge_pd(d1[i],d2);
					bitmap |= _mm_movemask_pd(result) << (2*i);
				}
			}
			else if(strcmp(relop,"<=") == 0)
			{
				int i = 0;
				for(i = 0;i < 16;i++)
				{
					result = _mm_cmple_pd(d1[i],d2);
					bitmap |= _mm_movemask_pd(result) << (2*i);
				}
			}
			else if(strcmp(relop,">") == 0)
			{
				int i = 0;
				for(i = 0;i < 16;i++)
				{
					result = _mm_cmpgt_pd(d1[i],d2);
					bitmap |= _mm_movemask_pd(result) << (2*i);
				}
			}
			else if(strcmp(relop,"<") == 0)
			{
				int i = 0;
				for(i = 0;i < 16;i++)
				{
					result = _mm_cmplt_pd(d1[i],d2);
					bitmap |= _mm_movemask_pd(result) << (2*i);
				}
			}
	}
	
	return bitmap;
}

int s_ColRelopColEvaluationIntrinsics(char *data1, char *data2, char *relop, int dataType)
{
	int bitmap = 0;
	
	return bitmap;
}

int s_evaluateNodeIntrinsics(ConditionNode_p node, Row_p row)
{
  int ret = 0;
  //Leaf ConditionNode. Possible node types {ColRelopCol, ColRelopVal}
  if(node->NodeType == ColRelopCol)
  {
    Column_p column1 = node->Node.ColRelopCol_Node._column1;
    Column_p column2 = node->Node.ColRelopCol_Node._column2;
    
    //Get data
    char *data1 = NULL;
    char *data2 = NULL;
    
    //Search for corresponding table in row for column1 and column2
    Row_p row1 = NULL, row2 = NULL;
    Row_p actual = row;
    while(actual != NULL)
    {
      if(actual->table == column1->table)
        row1 = actual;
      if(actual->table == column2->table)
        row2 = actual;
      
      if(row1 != NULL && row2 != NULL)
        break;
        
      actual = actual->next;
    }
    
    if(row1 == NULL || row2 == NULL)
      return 0;
    
    int dataTypeSize = 0;
    
    //Got corresponding Row_p
    data1 = (row1->page->data) + column1->offset*32; //Base of corresponding column
    DataTypeSize(column1->dataType, dataTypeSize);
    
    data2 = (row2->page->data) + column2->offset*32; //Base of corresponding column
    DataTypeSize(column2->dataType, dataTypeSize);

    ret = s_ColRelopColEvaluationIntrinsics(data1,
                                  data2,
                                  node->Node.ColRelopCol_Node._relop,
                                  column2->dataType);
    
    return (node->negation == 1)? ~ret : ret;
  }
  else if(node->NodeType == ColRelopVal)
  {
    Column_p column = node->Node.ColRelopVal_Node._column;
    Val_p val = node->Node.ColRelopVal_Node._val;
    
    //Get Data
    char *data = NULL;
    
    //Search for corresponding table in row for column1 and column2
    Row_p _row = NULL;
    Row_p actual = row;
    while(actual != NULL)
    {
      if(actual->table == column->table)
        _row = actual;

      if(_row != NULL)
        break;
        
      actual = actual->next;
    }
    
    if(_row == NULL)
      return 0;
    
    int dataTypeSize = 0;
    
    //Got corresponding Row_p
    data = (_row->page->data) + column->offset*32; //Base of corresponding column
    DataTypeSize(column->dataType, dataTypeSize);

    ret = s_ColRelopValEvaluationIntrinsics(data,
                                  val,
                                  node->Node.ColRelopVal_Node._relop,
                                  val->dataType);
    
    return (node->negation == 1)? ~ret : ret;
  }
  else
  {
    return ret;
  }
}

int s_evaluateQueryIntrinsics(ConditionNode_p node, Row_p row)
{
	if(node == NULL)
    return 0;
  
  if(node->Left == NULL || node->Right == NULL)
  {
    return s_evaluateNodeIntrinsics(node, row);
  }
  
  //Not-Leaf ConditionNode. It HAS to be Logop node type
  if(node->NodeType != Logop)
    return 0;
    
  //Evaluate this node based on Logop logic
  int left = 0, right = 0;
  left = s_evaluateQuery(node->Left, row);
  right = s_evaluateQuery(node->Right, row);
  int temp = 0;
  if(strcmp(node->Node.Logop_Node._logop, "&&") == 0)
  {
    temp = left & right;
    return (node->negation == 1)? !temp : temp;
  }
  else if(strcmp(node->Node.Logop_Node._logop, "||") == 0)
  {
    temp = left | right;
    return (node->negation == 1)? ~temp : temp;
  }
  else
  {
    return 0;
  }
}

void s_evaluateInstrinsics(Row_p row)
{
	int everyone = 0;
	
	if(Root == NULL) //If there's no condition to evaluate (empty where clause)
    everyone = 1;

	//s_evaluateQueryIntrinsics returns a bitmap that represents all the rows
	//of a page that pass the condition
	
	int evaluation = s_evaluateQueryIntrinsics(Root, row);
	int result = evaluation & row->page->bitmap;
	
	//All the rows that pass and are valid rows have to be included to the temp table
	Row_p temp_row = s_rowdup(row);
	for(temp_row->rowID = 0;
			temp_row->rowID < 32;
			temp_row->rowID++)
	{
		if((everyone & ((row->page->bitmap>>temp_row->rowID)&1)) || 
		   (result>>temp_row->rowID)&1)
		{
		  if(s_isaggregationquery() == 1)
		  {
		    //We need to add this row to the corresponding group
		    //So first we retrieve id for this row . . .
		    s_assignGroupToRow(temp_row);
		  }
		  else
		  {
		    //We need to add this row to
		    //the temp table with the specified layout (SELECT)
		    s_insertRowToTempTable(temp_row);
		  }
		}
  }
}

void s_joinAndFilterIntrinsics(Row_p row)
{
         /*|_____________________________________|*\
         |*|-------------------------------------|*|
         |*|---Intrinsics types are as follow:---|*|
         |*|-------------------------------------|*|
         |*|-- char   -> __m128i (_mm_x_epi8)  --|*|
         |*|-- int    -> __m128i (_mm_x_epi32) --|*|
         |*|-- float  -> __m128  (_mm_x_ps)    --|*|
         |*|-- double -> __m128d (_mm_x_pd)    --|*|
         |*|-------------------------------------|*|
         \*****************************************/
  /*|____________________________________________________|*\
  |*|----------------------------------------------------|*|
  |*|-Number of data types that fit in one mmx register:-|*|
  |*|----------------------------------------------------|*|
  |*|------------------  char   -> 8  -------------------|*|
  |*|------------------  int    -> 4  -------------------|*|
  |*|------------------  float  -> 4  -------------------|*|
  |*|------------------  double -> 2  -------------------|*|
  |*|----------------------------------------------------|*|
  \********************************************************/
 
  //Row_p row have the layout of the FROM
  //in other words, each Row_t have the necessary
  //elements to do the join and filter.
  
  //Since Intrinsics work with contiguous memory
  //we cannot work with individual elements.
  
  //Instead of working row by row, we'll be working page per page.
 	
 	//Do Join algorithm here.
 	
 	//Call to s_evaluateInstrinsics for each page in row
 	
 	int finished = 0;
 	Row_p temp = NULL;
  
  //Init pages
  for(temp = Row;temp != NULL;temp = temp->next)
  {
    temp->page = temp->table->hdr->firstPage;
    if(temp->page == NULL)
      return;
  }
  
	temp = Row;
  
  do{
  	//Evaluate
  	s_evaluateInstrinsics(temp);
  	
  	//Generate next page combination
  	temp->page = temp->page->nextPage;
  	
  	//Check for final combination
		finished = temp->page == NULL; 	
  }while(!finished);
}

void s_joinAndFilter(Row_p row)
{
#ifdef INTRINSICS
  s_joinAndFilterIntrinsics(row);
#else
  s_joinAndFilterNoIntrinsics(row);
#endif
}

int s_selectQuery()
{
  //First check if columns referenced in the query are from real
  //referenced tables.
  
    //Checking tables referenced in SELECT
    int i, j;
    for(i = 0;i < ColumnCount;++i)
    {
      if(s_AtFrom(columns[i].TableName) == 0)
        return -1; //ERROR, not at FROM
    }
    
    if(s_isaggregationquery() == 1)
    {
      //We have to check if all columns referenced in SELECT
      //exist in the group by columns set
      for(i = 0;i < ColumnCount;++i)
      {
        if(aggregations[i] != NULL)
        {
          //This column is an aggregation column, ignore.
          continue;
        }
        //If this column is not in the Group By clause, error
        if(s_AtGroupBy(&columns[i]) == 0)
        {
          printf("\nColumn %s.%s not in a group by!!!\n", columns[i].TableName,
                                                        columns[i].ColumnName);
          return -1;
        }
      }
    }
    
    //We need to add all the tables' descriptors
    if(everything)
    {
      //--------------------
          AtSelect = 1;
          everything = 0;
      //--------------------
      int offset = 0;
      //for each table
      Row_p actual = Row;
      while(actual != NULL)
      {
        //for each column
        for(j = 0;j < actual->table->columnCount;++j)
        {
          s_addColumn(actual->table->name, actual->table->column_names[j], &offset);
        }
        actual = actual->next;
      }
      //--------------------
          AtSelect = 0;
          everything = 1;
      //--------------------
    }
    
    //Is not necessary to verify table reference in the WHERE
    //since it comes after the FROM; that validation occurs
    //in the WHERE clause.
  
  //Do join while applying filter so we can add to temp table
  //all the rows that pass the filter
  
    s_joinAndFilter(Row);
    
  //Aggregation Functions execution
    if(s_isaggregationquery() == 1)
    {
      //At this point we have the complete set for the groups.
      //Each group have reference to it's corresponding set of rows.
      //We have to evaluate corresponding aggregation functions
      
      s_insertRowsFromGroups(); //This function evaluates corresponding aggregation functions
    }
  
  //End of query execution
  
  printf("\n");
  printTable(TEMP);
  return 0;
}

int s_insertQuery()
{
  //Do nothing, this is handled at queryParser.y
  return 0;
}

int s_createQuery()
{
  //Do nothing, this is handled at queryParser.y
  return 0;

}

int s_dropQuery()
{
  //Do nothing, this is handled at queryParser.y
  return 0;
}

//Final Semantic Analysis steps and query execution.
//return 0 if error occurs
int s_finalAnalysis()
{
  //Final Semantic Analysis
  switch(query_type)
  {
    case select_query:
      if(s_selectQuery() == -1) return -1;
    break;
    case insert_query:
      if(s_insertQuery() == -1) return -1;
    break;
    case create_query:
      if(s_createQuery() == -1) return -1;
    break;
    case drop_query:
      if(s_dropQuery() == -1) return -1;
    break;
    default:
      //Do nothing
    break;
  }
  
  s_endAnalysis();
  return 1;
}

void s_endAnalysis()
{
  if(table_created == 1)
  {
    deleteTable(TEMP);
    table_created = 0;
  }
  
  Last = NULL;
  Root = NULL;
  Row = NULL;
  Groups = NULL;
  GroupsCount = 0;
  columns = NULL;
  ColumnCount = 0;
  tables = NULL;
  TableCount = 0;
  aggregations = NULL;
  AggregationCount = 0;
}

#endif
