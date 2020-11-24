#ifndef QUERY_SEMANTIC_ANALYZER_H
#define QUERY_SEMANTIC_ANALYZER_H

#include "TableFunctions.h"
#include "SemanticTypes.h"
#include "ThrashCan.h"

/**
 * defines maximum size for strings
 */
#define MAX STR_MAX

/**
 * name of temporal table used at query engine
 */
#define TEMP "@temp_table"

/*! \file querySemanticAnalyzer.h
 * \brief File with the front end for the query semantic analyzer used at
 *        the syntax analyzer.
 *
 * Source file with the implementation of
 * semantic analyzer that's used in the
 * syntax analyzer.
 *
 * RULES:
 *
 * >You cannot make reference to a column
 *  which corresponding table is not present
 *  in the query.
 * >You cannot make reference to non existent
 *  tables.
 * >Operations in conditional expression should
 *  not "operate" condition with different data
 *  types.
 *
 * To make it easier, we'll use our database to do a
 * temporal table named "@temp_table".
 *
 */

//***********************************************************************************//

/**
 * Function that end the current semantic analysis.
 */
void s_endAnalysis();

/**
 * Function that initialize variables needed for the semantic
 * analyzer, like the columns and tables array and their
 * respective counters.
 */
void s_initSemanticAnalysis();

//***********************************************************************************//

/**
 * Indicates semantic analyzer that we are currently parsing a select query.
 */
void s_startSelectQuery();

/**
 * Indicates semantic analyzer that we are currently parsing an insert query.
 */
void s_startInsertQuery();

/**
 * Indicates semantic analyzer that we are currently parsing a create query.
 */
void s_startCreateQuery();

/**
 * Indicates semantic analyzer that we are currently parsing a drop query.
 */
void s_startDropQuery();

/**
 * Turns on flag to indicate we'll use all the columns of a table in the select clause.
 */
void s_allColumns();

/**
 * Turns on flag that indicates we are currently in the select clause.
 */
void s_startSelect();

/**
 * Turns off flag that indicates we have left the select clause.
 */
void s_endSelect();

/**
 * Turns on flag that indicates we are currently in the from clause.
 */
void s_startFrom();

/**
 * Turns off flag that indicates we have left the from clause.
 */
void s_endFrom();

/**
 * Turns on flag that indicates we are currently in the where clause.
 */
void s_startWhere();

/**
 * Turns off flag that indicates we have left the where clause.
 */
void s_endWhere();

/**
 * Turns on flag that indicates we are currently in the group by clause.
 */
void s_startgroupby();

/**
 * Turns off flag that indicates we have left the group by clause.
 */
void s_endgroupby();

/**
 * Turns on flag to specify the semantic analyzer
 * that current query is an aggregation query.
 */
void s_turnaggregationflag();

/**
 * Ask if current query is an aggregation query.
 * @return return 1 if current query is an aggregation query,
 *         return 0 otherwise.
 */
int s_isaggregationquery(); //asks if aggregation query

/**
 * Turns on flag to indicate the semantic analyzer if current query
 * is grouped by.
 */
void s_groupedby();

/**
 * Turns on flag that warns the semantic analyzer we are currently adding
 * a new aggregation column in order to add the column descriptor as a double.
 */
void s_incomingaggregation();

/**
 * Turns off flag to indicate that incoming aggregation was handled.
 */
void s_endaggregation();

//***********************************************************************************//

/**
 * Adds a column to temporal table.
 * @param table_name name of the incoming table.
 * @param column_name name of the incoming column.
 * @param _offset sets this incoming integer with the corresponding column offset.
 * @return return column's data type size.
 */
int s_addColumn(const char *table_name, const char* column_name, int *_offset);

/**
 * Adds a table to array of referenced tables (tables at from [JOIN])
 * @param table_name name of table to be added.
 */
void s_addTable(const char* table_name);

/**
 * Adds given aggregation function to temporal table's layout.
 * @param aggregation aggregation function to be added as a column to temporal table.
 */
void s_addAggregation(Aggregation_p aggregation);

/**
 * Function that builds conditional tree for where evaluation by appending nodes.
 * @param node condition node to be added to the conditional tree.
 */
void s_addNode(ConditionNode_p node);

//***********************************************************************************//

/**
 * Tells if given table is refrenced at from clause.
 * @param table_name table name.
 * @return return 1 if table name at from clause, return 0 otherwise.
 */
int s_AtFrom(const char *table_name);

/**
 * Tells if given column is referenced at the group by clause.
 * @param column given column.
 * @return return 1 if given column at group by clause, return 0 otherwise.
 */
int s_AtGroupBy(Column_p column);

/**
 * Search the owner table of the fiven column name.
 * @param column_name column name to be searched.
 * @return return table owner of given column, return NULL if there
 *         was no table found.
 */
table_p getTableWithColumn(const char* column_name);

/**
 * Retrieves data from given column descriptor and given row and returns it as a string.
 * @param column column descriptor that contains table-number of column relation.
 * @param row linked list representation of a row created by the temporal table layout.
 * @return string representation of data found.
 */
char* s_getStrFromColumnRow(Column_p column, Row_p row);

/**
 * Duplicates given row representation.
 * @param row linked list representation of a row created by the temporal table layout.
 * @return row duplicate.
 */
Row_p s_rowdup(Row_p row);

/**
 * Free memory used by this row linked list.
 * @param row given row linked list.
 * @param level linked list size.
 */
void s_freeRows(Row_p row, int level);

/**
 * Assigns given row to a group.
 * @param row row instance.
 */
void s_assignGroupToRow(Row_p row);

/**
 * Insert row instance to temporal table.
 * @param row row instance.
 */
void s_insertRowToTempTable(Row_p row);

//***********************************************************************************//

//Evaluation of Aggregation Functions
/**
 * Evaluates average aggregation function to given set of row instances.
 * @param aggregation aggregation descriptor.
 * @param rows set of row instances.
 * @param RowCount row set size.
 * @param _AVG value wrapper where this aggregation result is stored.
 */
void s_evaluateAVG(Aggregation_p aggregation, Row_p *rows, const int RowCount, Val_p _AVG);

/**
 * Evaluates summation aggregation function to given set of row instances.
 * @param aggregation aggregation descriptor.
 * @param rows set of row instances.
 * @param RowCount row set size.
 * @param _SUM value wrapper where this aggregation result is stored.
 */
void s_evaluateSUM(Aggregation_p aggregation, Row_p *rows, const int RowCount, Val_p _SUM);

/**
 * Evaluates count aggregation function to given set of row instances.
 * @param aggregation aggregation descriptor.
 * @param rows set of row instances.
 * @param RowCount row set size.
 * @param _COUNT value wrapper where this aggregation result is stored.
 */
void s_evaluateCOUNT(Aggregation_p aggregation, Row_p *rows, const int RowCount, Val_p _COUNT);

/**
 * Evaluates maximum aggregation function to given set of row instances.
 * @param aggregation aggregation descriptor.
 * @param rows set of row instances.
 * @param RowCount row set size.
 * @param _MAX value wrapper where this aggregation result is stored.
 */
void s_evaluateMAX(Aggregation_p aggregation, Row_p *rows, const int RowCount, Val_p _MAX);

/**
 * Evaluates minimum aggregation function to given set of row instances.
 * @param aggregation aggregation descriptor.
 * @param rows set of row instances.
 * @param RowCount row set size.
 * @param _MIN value wrapper where this aggregation result is stored.
 */
void s_evaluateMIN(Aggregation_p aggregation, Row_p *rows, const int RowCount, Val_p _MIN);

//Return value resulting from evaluating aggregation function
/**
 * Evaluates given aggregation function to given set of row instances. This function calls
 * specific aggregation function for the value wrapper.
 * @param aggregation aggregation descriptor.
 * @param rows set of row instances.
 * @param RowCount row set size.
 * @return value wrapper where this aggregation result is stored.
 */
Val_p s_evaluateAggregation(Aggregation_p aggregation, Row_p *rows, const int RowCount);

//***********************************************************************************//

/**
 * Inserts to temporal table the resulting row of each group. Version with no
 * intrinsics implementation.
 */
void s_insertRowsFromGroupsNonInstrinsics();

/**
 * Inserts to temporal table the resulting row of each group. Version with
 * intrinsics implementation. << NOT YET IMPLEMENTED, DO NOT USE >>.
 */
void s_insertRowsFromGroupsInstrinsics();

/**
 * Inserts to temporal table the resulting row of each group. This functions
 * branches execution depending if current process is running in intrinsics
 * version or non intrinsics version.
 */
void s_insertRowsFromGroups();

//***********************************************************************************//

//Evaluation functions for the condition Tree

/**
 * Evaluates condition node of type Column__Relational Operator__Value.
 * @param data1 data address of column.
 * @param data2 value wrapper with value.
 * @param relop relational operator being used.
 * @param dataType data type of both data1 and data2.
 * @return evaluation of given condition (1 - true, 0 - false).
 */
int s_ColRelopValEvaluation(char *data1, Val_p data2, char *relop, int dataType);

/**
 * Evaluates condition node of type Column__Relational Operator__Column.
 * @param data1 data address of column 1.
 * @param data2 data address of column 2.
 * @param relop relational operator being used.
 * @param dataType data type of both data1 and data2.
 * @return evaluation of given condition (1 - true, 0 - false).
 */
int s_ColRelopColEvaluation(char *data1, char *data2, char *relop, int dataType);

/**
 * Evaluates given condition node.
 * @param node node to be evaluated.
 * @param row row instance.
 * @return evaluation of given node (1 - true, 0 - false).
 */
int s_evaluateNode(ConditionNode_p node, Row_p row);

/**
 * Evaluates given query. Front-end for s_evaluateNode.
 * @param node node to be evaluated.
 * @param row row instance.
 * @return evaluation of given node (1 - true, 0 - false).
 */
int s_evaluateQuery(ConditionNode_p node, Row_p row);

/**
 * Evaluates given row instance in the conditional tree.
 */
void s_evaluate(Row_p row);

//***********************************************************************************//

//Join functions

/**
 * Implements recursive join algorithm without intrinsics.
 * @param row row representation of current temporal table.
 */
void s_joinAndFilterNoIntrinsics(Row_p row);

/**
 * Implements recursive join algorithm with intrinsics.
 * << NOT YET IMPLEMENTED, DO NOT USE >>.
 * @param row row representation of current temporal table.
 */
void s_joinAndFilterIntrinsics(Row_p row);

//***********************************************************************************//

//Functions for the final analysis depending query type

/**
 * Evaluates select query.
 * @return return 1 if evaluated query succesfully, return 0 otherwise.
 */
int s_selectQuery();

/**
 * Evaluates insert query.
 * @return return 1 if evaluated query succesfully, return 0 otherwise.
 */
int s_insertQuery();

/**
 * Evaluates create query.
 * @return return 1 if evaluated query succesfully, return 0 otherwise.
 */
int s_createQuery();

/**
 * Evaluates drop query.
 * @return return 1 if evaluated query succesfully, return 0 otherwise.
 */
int s_dropQuery();

/**
 * End point for the semantic analyzer, depending the type of query calls it's
 * corresponding evaluation query function.
 * @return return 1 if evaluated query succesfully, return 0 otherwise.
 */
int s_finalAnalysis();

/**
 * Ends current semantic analysis.
 */
void s_endAnalysis();

//***********************************************************************************//

#endif
