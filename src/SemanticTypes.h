#ifndef SEMANTIC_TYPES_H
#define SEMANTIC_TYPES_H

#include "TableTypes.h"

/*! \file SemanticTypes.h
 * \brief Header with data types used at the semantic analyzer.
 * 
 * This file defines all the data types used at the semantic analyzer.
 */

/**
 * \brief
 * enumeration with the different query types.
 */
enum queryTypes {
  none,
  select_query,
  insert_query,
  create_query,
  drop_query
};

/**
 * \brief
 * typedef for the column_t pointer.
 */
typedef struct Column_t *Column_p;

/**
 * \brief
 * Structure that defines a column representation.
 */
typedef struct Column_t{
  //! Table name that contains this column descriptor.
  char *TableName;
  //! Column name.
  char *ColumnName;
  //! column's data type.
  int dataType;
  //! offset within the memory representation of columns set.
  int offset;
  //! table pointer to table owner of this column.
  table_p table;
}Column_t;

/**
 * \brief
 * typedef for the Row_t pointer.
 */
typedef struct Row_t *Row_p;

/**
 * \brief
 * Structure to represent a row (one per table in FROM).
 *
 * This structure is built like a linked list where each element correspond
 * to a table specified in the from clause.
 * An instance of this linked list structure represents one row from the temporal
 * table formed from the join.
 */
typedef struct Row_t{
  //! Pointer to table.
  table_p table;
  //! pointer to current page in table.
  page_p page;
  //! current row within a page, [0, 31].
  int rowID;
  //! pointer to next row (linked list).
  Row_p next;
}Row_t;

/**
 * \brief
 * typedef for the Val_t pointer.
 */
typedef struct Val_t *Val_p;

/**
 * \brief
 * Structure representation of a value (char, int, float, double).
 */
typedef struct Val_t{
  //! data type (1 = char, 2 = int, 3 = float, 4 = double).
  int dataType;
  //! union to choose from one data type within the set depending the dataType member.
  union Value_t{
    char char_val;
    int int_val;
    float float_val;
    double double_val;
  }Value;
}Val_t;

/**
 * \brief
 * Macro for printing in a given string the value of val.
 */
#define SPrintVal(str, val)\
do{\
  switch(val->dataType)\
  {\
    case 0: printf("NoDataType"); break;\
    case 1: sprintf(str,"\'%c\',",val->Value.char_val); break;\
    case 2: sprintf(str,"%d,",val->Value.int_val); break;\
    case 3: sprintf(str,"%f,",val->Value.float_val); break;\
    case 4: sprintf(str,"%f,",val->Value.double_val); break;\
    case 5: printf("NoDataType"); break;\
    default: printf("NoDataType"); break;\
  }\
}while(0)\

/**
 * \brief
 * typedef for the Aggregation_t pointer.
 */
typedef struct Aggregation_t *Aggregation_p;

/**
 * \brief
 * Structure for aggregation function.
 */
typedef struct Aggregation_t{
  //! Aggregation Type (avg, sum, count, max, min).
  int AggregationType;
  //! Column pointer (column from where the aggregation function will be evaluated).
  Column_p column;
}Aggregation_t;

/**
 * \brief
 * typedef for the Group_t pointer.
 */
typedef struct Group_t *Group_p;

/**
 * \brief
 * Structure for a group (set of rows).
 */
typedef struct Group_t{
  //! Group unique identifier
  char *id;
  //! Row set for this group (dynamic array of row instances)
  Row_p *rows;
  //! Row set count
  int RowCount;
}Group_t;

/**
 * \brief
 * Types of aggregation
 */
enum aggregationTypes {
  avg,   //Average
  sum,   //Total Sum
  count, //Row Count
  max,   //Maximum Value
  min    //Minimum Value
};

/**
 * \brief
 * Types of node
 */
enum nodeTypes {
  Logop,
  ColRelopVal,
  ColRelopCol
};

/**
 * \brief
 * typedef for the ConditionNode_t pointer
 */
typedef struct ConditionNode_t *ConditionNode_p;

/**
 * \brief
 * Structure that represent a node in the binary condition tree.
 *
 * This node representation have an internal union that contains
 * all the possible type of nodes.
 * There are three type of nodes:
 * Logop_Node: Node representing a logical operator (&&, ||).
 * ColRelopVal_Node: Node representing a relational pair Column-Value.
 * ColRelopCol_Node: Node representing a relational pair Column-Column.
 */
typedef struct ConditionNode_t{
  //! identifier of node type.
  int NodeType;
  //! specifies if this node is negated !(condition).
  int negation;
  //! Union of Node types.
  union Node_t{
    //! Relop Node.
    struct Logop_Node{
      //! logical operator [&&,||].
      char *_logop;
    }Logop_Node;
    
    //! Col Relop Val Node
    struct ColRelopVal_Node{
      //! first element of relational pair.
      Column_p _column;
      //! relational operator [<,>,<=,>=,==,!=].
      char *_relop;
      //! second element of relational pair.
      Val_p _val;
    }ColRelopVal_Node;
    
    //! Col Relop Col Node
    struct ColRelopCol_Node{
      //! first element of relational pair.
      Column_p _column1;
      //! relational operator [<,>,<=,>=,==,!=].
      char *_relop;
      //! second element of relational pair.
      Column_p _column2;
    }ColRelopCol_Node;
  }Node;
  
  //! Pointer to left node
  ConditionNode_p Left;
  //! Pointer to right node
  ConditionNode_p Right;
  
}ConditionNode_t;

#endif

