#ifndef TABLE_TYPES_H
#define TABLE_TYPES_H

#include "uthash.h"

/*! \file TableTypes.h
 * \brief File with the definition of the data types used for table representation in RAM.
 *
 * This table implementation is based
 * in columnwise storage.
 *
 * In order to do this we have to define
 * our memory layout.
 *
 * Memory Layout:
 *
 * | table_t | | header_t | -> ( | page_t | -> )*
 * 
 * Our bitmap size is 32 (4 bytes Integer)
 * The page data is distributed by columns.
 * Each 32 * dataTypeSize bytes correspond
 * to each column, making the other 32
 * another column. For example, in a table
 * with two columns, where the first column
 * is of type char and the second column
 * of type float, the first 32 * sizeof(char)
 * (32 bytes) are going to be the first
 * column and the next 32 * sizeof(float)
 * (1024 bytes) are going to be the second
 * column.
 * To build a row, you have to get the nth
 * element of each column data chunk.
 * For example, if we want the 3rd row of 
 * certain page, it is going to be built 
 * with the char found in the 3rd byte of the
 * data and the float is going to be found
 * in the 128th byte of the data.
 *
 * Row3 = *(char*)&data[3], *(float*)&data[128];
 * Row3 = ((char*)data)[3], ((float*)&data[32])[3];
 * 
 * That's why we need to know the columns
 * data types (descriptor in header).
 *
 */
 
#define STR_MAX 100 //!< Maximum size for string

/**
 * \brief
 * Macro to put on 'var' size in bytes of corresponding enum data type.
 */
#define DataTypeSize(x, var) \
do{\
  switch(x)\
  {\
    case 0: var = 0; break;\
    case 1: var = sizeof(char); break;\
    case 2: var = sizeof(int); break;\
    case 3: var = sizeof(float); break;\
    case 4: var = sizeof(double); break;\
    case 5: var = 0; break;\
    default: var = 0; break;\
  }\
}while(0)

/**
 * \brief
 * Macro to put on var the string representation of corresponding enum
 * var must be a char pointer.
 */
#define GetDataTypeName(x, var) \
do{\
  switch(x)\
  {\
    case 0: strcpy(var,"start"); break;\
    case 1: strcpy(var,"char"); break;\
    case 2: strcpy(var,"int"); break;\
    case 3: strcpy(var,"float"); break;\
    case 4: strcpy(var,"double"); break;\
    case 5: strcpy(var,"end"); break;\
    default: strcpy(var,"none"); break;\
  }\
}while(0)

/**
 * \brief
 * Macro to print data to its correct data type
 */
#define PrintData(data_type, data)\
do{\
  switch(data_type)\
  {\
    case 0: printf("NoDataType"); break;\
    case 1: printf("%c",*((char*)data)); break;\
    case 2: printf("%d",*((int*)data)); break;\
    case 3: printf("%f",*((float*)data)); break;\
    case 4: printf("%f",*((double*)data)); break;\
    case 5: printf("NoDataType"); break;\
    default: printf("NoDataType"); break;\
  }\
}while(0)\

/**
 * \brief
 * Macro that stores in 'str' given data.
 */
#define SPrintData(str, data_type, data)\
do{\
  switch(data_type)\
  {\
    case 0: printf("NoDataType"); break;\
    case 1: sprintf(str,"\'%c\',",*((char*)data)); break;\
    case 2: sprintf(str,"%d,",*((int*)data)); break;\
    case 3: sprintf(str,"%f,",*((float*)data)); break;\
    case 4: sprintf(str,"%f,",*((double*)data)); break;\
    case 5: printf("NoDataType"); break;\
    default: printf("NoDataType"); break;\
  }\
}while(0)\

/**
 * \brief enumeration for the supported data types
 *
 * We need '3' bits to make reference to an element of the enum.
 */
enum dataTypes {
  start,    //000
  char_t,   //001
  int_t,    //010
  float_t,  //011
  double_t, //100
  end       //101
};

/**
 * \brief
 * typedef for the table_t pointer
 */
typedef struct table_t* table_p;
/**
 * \brief
 * typedef for the header_t pointer
 */
typedef struct header_t* header_p;
/**
 * \brief
 * typedef for the metadata_t pointer
 */
typedef struct metadata_t* metadata_p;
/**
 * \brief
 * typedef for the page_t pointer
 */
typedef struct page_t* page_p;

/**
 * \brief
 * Structure that represents a table.
 */
typedef struct table_t{
  //! pointer to table header.
  header_p hdr;
  //! table name.
  char name[STR_MAX];             /* key */
  //! number of columns.
  int columnCount;
  //! size in bytes for a row.
  int row_size;
  //! dynamic array with all the column names.
  char (*column_names)[STR_MAX];
  //! hash table structure pointer.
  UT_hash_handle hh;              /* hashable structure */
}table_t;

#define DESCRIPTOR_INIT ((char)(0x28)) //!< Initialize byte with 0010 1000
#define MASK1(x) ((((int)(x)) & 0x38)>>3) //!< MASK: 0011 1000
#define MASK0(x) (((int)(x)) & 0x07) //!< MASK: 0000 0111

/**
 * \brief
 * Structure that represents table header.
 */
typedef struct header_t {
  //! Pointer to first table page.
  page_p firstPage;
  //! Table column data types descriptor.
  char descriptor[1];
}header_t;

/**
 * \brief
 * Structure that represents a table page. 
 */
typedef struct page_t{
  //! pointer to next page (linked list of pages).
  page_p nextPage;
  //! bitmap for rows [0,31]
  int bitmap;
  //! Pointer to first address of raw data.
  char data[1] __attribute__ ((aligned(16)));
}page_t;

#endif

//Hash Table functions
/*
HASH_ADD_INT (head, keyfield_name, item_ptr)
HASH_REPLACE_INT (head, keyfiled_name, item_ptr,replaced_item_ptr)
HASH_FIND_INT (head, key_ptr, item_ptr)
HASH_ADD_STR (head, keyfield_name, item_ptr)
HASH_REPLACE_STR (head,keyfield_name, item_ptr, replaced_item_ptr)
HASH_FIND_STR (head, key_ptr, item_ptr)
HASH_ADD_PTR (head, keyfield_name, item_ptr)
HASH_REPLACE_PTR (head, keyfield_name, item_ptr, replaced_item_ptr)
HASH_FIND_PTR (head, key_ptr, item_ptr)
HASH_DEL (head, item_ptr)
HASH_SORT (head, cmp)
HASH_COUNT (head)
*/
