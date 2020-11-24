#ifndef TABLE_FUNCTIONS_H
#define TABLE_FUNCTIONS_H

#include "TableTypes.h"

//Functions ------------------------------------------------------------------//

/*! \file TableFunctions.h
 * \brief File with the functions that permit table manipulation.
 *
 * This file implements all the funcionts needed to manipulate table creation,
 * modification, and deletion.
 *
 * All this functions are intended to be used by MiniDB Shell and the query engine
 * of MiniDB (querySemanticAnalyzer[.h/.c])
 */

/**
 * Tells if exists table with the given table name.
 * @param table_name table name.
 * @return returns 1 if table exists,
 *          returns 0 otherwise.     
 */
int exists(const char* table_name);

/**
 * Tells if exists given column in given table.
 * @param table_name table name.
 * @param column_name column name.
 * @return returns 1 if column exists in given table.
 *         returns 0 otherwise.                       
 */
int columnExists(const char* table_name, const char* column_name);

/**
 * Finds and return data type for given column in given table.
 * @param table_name table name.
 * @param column_name column name.
 * @param offset integer where column offset is stored.
 * @return returns enum integer for corresponding   
 *         data type for this column.                
 */
int getColumnDataType(const char* table_name, const char* column_name, int *offset);

/**
 * Returns table pointer that matches with given table name.
 * @param table_name table name.
 * @return returns NULL if table doesn't exist.      
 */
table_p getTable(const char* table_name);

/**
 * Creates a new table with the given name. 
 * @param table_name table name.
 * @return returns pointer to table if succesful 
 *         or null if couldn't create table.       
 */
void* createTable(const char* table_name);

/**
 * Deletes the given table. (Deep delete).
 * @param table_name table name.
 * @return return -1 if unsuccesful.              
 */
int deleteTable(const char* table_name);

/**
 * Prints the given table in a simple format.
 * @param table_name table name.
 */
void printTable(const char* table_name);

/**
 * Prints all current existant tables.
 */
void mentionTables(void);

/**
 * Adds new column to given table with given name and given data type.                 
 * @param table_name table name.
 * @param column_name column name.
 * @param data_type data type.
 * @return returns -1 if unsuccesful.                
 */
int addColumn(const char* table_name, const char* column_name, const int data_type);

/**
 * Inserts row into given table. It's the programmers responsability to give a 
 * coherent void* with the corresponding data types for this table.             
 * @param table_name table name.
 * @param row string that contains all elements to
 *        insert separated by a single comma without white space.
 * @return returns -1 if unsuccesful.             
 */
int insertRow(const char* table_name, void* row);

/**
 * Deletes row in a given table by ID.    
 * @param table_name table name.
 * @param rowID rowID, where rowID/32 is page and rowID%32 is page offset.
 * @return returns -1 if unsuccesful.             
 */
int deleteRowById(const char* table_name, int rowID);

/**
 * NOTE: 
 * Instead of writing and reading complete Table to the file, we are just going to 
 * write and read vital information of the table so  we can  use our  functions to 
 * build the complete table again.         
 * In order to save and load tables we are 
 * going to use the following format:      
 * 1)First STR_MAX bytes table name.        
 * 2)4 bytes for column count.              
 * 3)(ColumnCount*sizeof(column_t)) bytes  
 *   for column data types.                 
 * 4)Next  char[STR_MAX]  size  lines are  
 *   the row string representation.        
 *
 * Stores Table in disk.
 * @param table_name table name.
 * @return return -1 if unsuccesfull.                 
 */
int saveTable(const char* table_name);

/**
 * NOTE: 
 * Instead of writing and reading complete Table to the file, we are just going to 
 * write and read vital information of the table so  we can  use our  functions to 
 * build the complete table again.         
 * In order to save and load tables we are 
 * going to use the following format:      
 * 1)First STR_MAX bytes table name.        
 * 2)4 bytes for column count.              
 * 3)(ColumnCount*sizeof(column_t)) bytes  
 *   for column data types.                 
 * 4)Next  char[STR_MAX]  size  lines are  
 *   the row string representation.        
 *
 * Loads Table from disk.                 
 * @param table_name table name.
 * @return return -1 if unsuccesfull.
 */
int loadTable(const char* table_name);

/**
 * Stores DB in disk.
 * @param db_name database name.
 * @return return -1 if unsuccesfull.                     
 */
int saveDB(const char* db_name);

/**
 * Loads DB from disk.                    
 * @param db_name database name.
 * @return return -1 if unsuccesfull.
 */
int loadDB(const char* db_name);

/**
 * Deletes all existent tables.           
 * @return return -1 if unsuccesfull.
 */
int deleteAllTables();

#endif
