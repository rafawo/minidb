#ifndef TABLE_FUNCTIONS_C
#define TABLE_FUNCTIONS_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TableFunctions.h"

//Hash Table of tables
static table_p tables = NULL;

int exists(const char* table_name)
{
  //Consult in the hash table if table exists
  table_p table = NULL;
  HASH_FIND_STR(tables, table_name, table);
  if(table == NULL)
    return 0;
  else
    return 1;
}

void* createTable(const char* table_name)
{
  if(exists(table_name) ||
     strcmp(table_name, "table") == 0)
    return NULL;
  
  int i = 0;
  for(i = 0;table_name[i] != '\0';++i)
    if(table_name[i] == '/')
      return NULL;
  
  //Alloc memory for the new table
  table_p table = (table_p)calloc(1, sizeof(table_t));
  strcpy(table->name, table_name);
  table->columnCount = 0;
  table->row_size = 0;
  table->column_names = NULL;
  table->hdr = (header_p)malloc(sizeof(header_t));
  table->hdr->descriptor[0] = DESCRIPTOR_INIT;
  table->hdr->firstPage = NULL;
  
  //Add new table to the hash table
  HASH_ADD_STR(tables, name, table);
  
  return table; //return pointer to newly created table
}

static void delete(table_p table)
{
  //Delete all pages. . .
  page_p actual = table->hdr->firstPage;
  while(actual != NULL)
  {
    page_p temp = actual;
    actual = actual->nextPage;
    free(temp);
  }
  
  //Delete Header
  free(table->hdr);
  
  //Delete column_names
  free(table->column_names);
  
  //Delete table
  free(table);
}

int deleteTable(const char* table_name)
{
  table_p table = NULL;
  //Retrieve table in hash table
  HASH_FIND_STR(tables, table_name, table);
  
  if(table == NULL)
    return 0;
  
  HASH_DEL(tables, table); //remove table from hash table
  
  delete(table); //free all memory allocated for this table
}

void printTable(const char* table_name)
{
  table_p table = NULL;
  //Retrieve table from hash table
  HASH_FIND_STR(tables, table_name, table);
  
  if(table == NULL)
    return;

    printf("Table Name: %s\n", table->name);
    printf("Number of Columns: %d\n", table->columnCount);
    printf("Row Size: %d bytes\n", table->row_size);
    printf("\nColumn DataTypes:\n");
    
    /****** Printing Column Description *************************************/
    int i = 0;
    int j = 0;
    int stop = 0;
    int mask0 = 0, mask1 = 0;
    char dataTypeName[STR_MAX];
    int *data_types = (int*)malloc(sizeof(int)*table->columnCount);
    while(stop != 1)
    {
      mask0 = MASK0(table->hdr->descriptor[i]);
      mask1 = MASK1(table->hdr->descriptor[i]);
      
      if(mask0 == end) //If end of descriptor stop loop
      {
        stop = 1;
      }
      else
      {
        if(mask0 != start) //If not first descriptor 'start'
        {
          //Process element in mask0
          GetDataTypeName(mask0, dataTypeName);
          data_types[j] = mask0;
          printf("%s(%s) ",table->column_names[j++],dataTypeName);
        }
        if(mask1 == end) //If end of descriptor stop loop
        {
          stop = 1;
        }
        else
        {
          //Process element in mask1
          GetDataTypeName(mask1, dataTypeName);
          data_types[j] = mask1;
          printf("%s(%s) ",table->column_names[j++],dataTypeName);
        }
        
        i++;
      }
    }
    /****** End of Printing Column Description *******************************/
    
    /****** Printing Rows in Table (actual data) *****************************/
    
    page_p p = table->hdr->firstPage;
    int dataTypeSize = 0;
    char *address = NULL;
    int page_n = 0;
    while(p != NULL)
    {
      for(i = 0;i < 32;++i)
      {
        if(((p->bitmap)>>i & 1) == 1) //Found row
        {
          printf("\n");
          address = p->data;
          for(j = 0;j < table->columnCount;++j)
          {
            if(j != 0)
            {
              DataTypeSize(data_types[j - 1],dataTypeSize);
              address = (address + 32*dataTypeSize);
            }
            DataTypeSize(data_types[j], dataTypeSize);
            printf("\'");
            PrintData(data_types[j], &address[i*dataTypeSize]);
            printf("\' ");
          }
        }
      }
      p = p->nextPage;
      page_n++;
    }
    
    free(data_types);
    
    /****** End of Printing Rows in Table ************************************/
}

void mentionTables(void)
{
  table_p table = NULL;
  for(table = tables;table != NULL;table = table->hh.next)
  {
    printf("%s, ", table->name);
  }
  printf("\n");
}

int addColumn(const char* table_name, const char* column_name, const int data_type)
{
  if(data_type <= start || data_type >= end)
    return -1;
  table_p table = NULL;
  
  HASH_FIND_STR(tables, table_name, table);
  
  if(table == NULL)
    return -1;
    
  //Add a new column to table
  table->columnCount++;
  table->column_names = (char(*)[STR_MAX])realloc(table->column_names,
                                sizeof(char[STR_MAX])*table->columnCount);
  strcpy(table->column_names[table->columnCount - 1], column_name);
  
  //Add new column to descriptor
  int index = (table->columnCount)>>1;
  
  if(MASK0(table->hdr->descriptor[index]) == end)
  {
    table->hdr->descriptor[index] &= 0xF8; //1111 1000
    table->hdr->descriptor[index] |= data_type;
    table->hdr->descriptor[index] &= 0xC7; //1100 0111
    table->hdr->descriptor[index] |= (end<<3);
  }
  else
  {
    table->hdr->descriptor[index] &= 0xC7;
    table->hdr->descriptor[index] |= (data_type<<3);
    table->hdr = (header_p)realloc(table->hdr,
                                         sizeof(header_t) + sizeof(char)*(index + 1));
    table->hdr->descriptor[index + 1] &= 0xF8;
    table->hdr->descriptor[index + 1] |= end;
  }
  
  //Realloc all pages
  int data_type_size = 0;
  DataTypeSize(data_type, data_type_size);
  table->row_size += data_type_size;
  if(table->hdr->firstPage != 0)
  {
    table->hdr->firstPage = (page_p)realloc(table->hdr->firstPage, sizeof(page_t) + (table->row_size*32));
    //realloc next page for each page except last page. . . (WUT?)
    page_p temp = table->hdr->firstPage;
    while(temp->nextPage != 0) //NULL
    {
      temp->nextPage = (page_p)realloc(temp->nextPage, sizeof(page_t) + (table->row_size*32));
      temp = temp->nextPage;
    }
  }
  return 0;
}

int insertRow(const char* table_name, void* row)
{
  if(row == NULL || strcmp((char*)row, "") == 0)
    return -1;
  
  table_p table = NULL;
  
  //Retrieve table from hash table
  HASH_FIND_STR(tables, table_name, table);
  
  if(table == NULL)
    return -1;
    
  //Found table
  //Now find first free Row to insert to table . . .
  page_p p = table->hdr->firstPage;
  page_p last = 0;

  page_p page = NULL;
  int index = 0;
  int stop = 0;
  while(p != NULL && stop != 1)
  {
    int i;
    for(i = 0;i < 32;++i)
    {
      if(((p->bitmap)>>i & 1) == 0)
      {
        //Found dirty row
        index = i;
        page = p;
        stop = 1;
        break;
      }
    }
    last = p;
    p = p->nextPage;
  }

  if(stop == 0)
  {
    //This means all the pages were full
    //we need to add new page
    page = (page_p)calloc(1, sizeof(page_t) + table->row_size*32);
    if(page == NULL)
      return -1;
    page->nextPage = NULL;
    page->bitmap = 0; //No row
    index = 0;
    
    if(last == NULL)
    {
      table->hdr->firstPage = page;
    }
    else
    {
      last->nextPage = page;
    }
  }

  //At this point we have to add row at page 'page' in index 'index'
  if(page == NULL)
    return -1;
  page->bitmap |= (1<<index); //marked row

  char *input = (char*)row;

  int i = 0;
  int j = 0;
  stop = 0;
  int mask0, mask1;
  int *data_types = (int*)malloc(sizeof(int)*table->columnCount);
  //Get data types for columns
  while(stop != 1)
  {
    mask0 = MASK0(table->hdr->descriptor[i]);
    mask1 = MASK1(table->hdr->descriptor[i]);
    
    if(mask0 == end) //If end of descriptor stop loop
    {
      stop = 1;
    }
    else
    {
      if(mask0 != start) //If not first descriptor 'start'
      {
        //Process element in mask0
        data_types[j++] = mask0;
      }
      if(mask1 == end) //If end of descriptor stop loop
      {
        stop = 1;
      }
      else
      {
        //Process element in mask1
        data_types[j++] = mask1;
      }
      i++;
    }
  }
  //End of getting data types for columns

  char temp[100]; //used for temporal value (between commas)
  int t = 0;
  char *address = page->data;
  int dataTypeSize = 0;
  for(i = 0, j = 0;input[i] != '\0' && t < 99;++i)
  {
    if(input[i] == '\'')
    {
      temp[t++] = input[i + 1];
      i += 2;
      continue;
    }
    if(input[i] == ',')
    {
      temp[t] = '\0';
      t = 0;
      if(j != 0)
      {
        DataTypeSize(data_types[j - 1],dataTypeSize);
        address = (address + 32*dataTypeSize);
      }
      if(j >= table->columnCount)
        continue;
      DataTypeSize(data_types[j], dataTypeSize);
      
      char *char_adr = 0;
      int *int_adr = 0;
      float *float_adr = 0;
      double *double_adr = 0;
      switch(data_types[j])
      {
        case 1: //char
          char_adr = (char*)address;
          char_adr[index] = temp[0];
        break;
        case 2: //int
          int_adr = (int*)address;
          int_adr[index] = atoi(temp);
        break;
        case 3: //float
          float_adr = (float*)address;
          float_adr[index] = strtof(temp, 0);
        break;
        case 4: //double
          double_adr = (double*)address;
          double_adr[index] = atof(temp);
        break;
      }
      j++;
    }
    else
    {
      temp[t++] = input[i];
    }
  }

  free(data_types);

  return 0;
}

int deleteRowById(const char* table_name, int rowID)
{
  table_p table = NULL;
  
  //Retrieve table from hash table
  HASH_FIND_STR(tables, table_name, table);
  
  if(table == NULL)
    return -1;
    
  //Deleting a row implies to unset bit from corresponding page bitmap
  int page = rowID / 32;
  int index = rowID % 32;
  
  int i = 0;
  page_p p = table->hdr->firstPage;
  while(p != 0 && i < page)
  {
    i++;
    p = p->nextPage;
  }
  
  if(p == NULL)
    return -1;
    
  p->bitmap &= ( ( (~0)<<(index + 1) ) | ( (1<<index) - 1 ) ); //Unset index bit
  
  return 0;
}

int saveTable(const char* table_name)
{ 
  table_p table = NULL;
  
  //Retrieve table from hash table
  HASH_FIND_STR(tables, table_name, table);
  
  if(table == NULL)
    return -1;
    
  FILE *fp = fopen(table_name, "wb");
  
  if(fp == NULL)
    return -1;
    
  //Found table to save!!
  
  //1) Write table name
  if(fwrite(&table->name, sizeof(char[STR_MAX]), 1, fp) == 0)
  {
    fclose(fp);
    return -1;
  }
  
  
  //2) Write Column Count
  if(fwrite(&table->columnCount, sizeof(int), 1, fp) == 0)
  {
    fclose(fp);
    return -1;
  }
  
  
  //3) Write Column Name and data type columnCount times
  
  //Obtain data types . . .
    int i = 0;
    int j = 0;
    int stop = 0;
    int mask0, mask1;
    int *data_types = (int*)malloc(sizeof(int)*table->columnCount);
    while(stop != 1)
    {
      mask0 = MASK0(table->hdr->descriptor[i]);
      mask1 = MASK1(table->hdr->descriptor[i]);
      
      if(mask0 == end) //If end of descriptor stop loop
      {
        stop = 1;
      }
      else
      {
        if(mask0 != start) //If not first descriptor 'start'
        {
          //Process element in mask0
          data_types[j++] = mask0;
        }
        if(mask1 == end) //If end of descriptor stop loop
        {
          stop = 1;
        }
        else
        {
          //Process element in mask1
          data_types[j++] = mask1;
        }
        i++;
      }
    } 
  //End of data type retrieval . . .
  
  int count = 0;
  for(count = 0;count < table->columnCount;count++)
  {
    //writing name . . .
    if(fwrite(&table->column_names[count],sizeof(char[STR_MAX]),1,fp) == 0)
    {
      fclose(fp);
      free(data_types);
      return -1;
    }
    //writing data type . . .
    if(fwrite(&data_types[count],sizeof(int),1,fp) == 0)
    {
      fclose(fp);
      free(data_types);
      return -1;
    }
  }
  
  
  //4) Rows
  
  char temp[STR_MAX] = {'\0'}; //used for temporal values
  char temp2[1000] = {'\0'};
  memset(temp2, 0, 1000);
  char *address = 0;
  int k = 0, dataTypeSize = 0;
  page_p p = table->hdr->firstPage;
  while(p != 0) //NULL
  {
    int j = 0;
    for(j = 0;j < 32;++j)
    {
      int formatIndex = 0;
      if(((p->bitmap)>>j & 1) == 1)
      {
        //Row to string
        address = p->data;
        for(k = 0;k < table->columnCount;++k)
        {
          if(k != 0)
          {
            DataTypeSize(data_types[k - 1],dataTypeSize);
            address = (address + 32*dataTypeSize);
          }
          DataTypeSize(data_types[k], dataTypeSize);
          //Process data
          SPrintData(temp,data_types[k],&address[j*dataTypeSize]);
          strcat(temp2,temp);
        }
        //Write Row
        if(fwrite(&temp2,sizeof(char[1000]),1,fp) == 0)
        {
          fclose(fp);
          free(data_types);
          return -1;
        }
        temp2[0] = '\0';
      }
    }
    p = p->nextPage;
  }
  
  free(data_types);
  
  
  fclose(fp);
  return 0;
}

int loadTable(const char* table_name)
{
  FILE *fp = fopen(table_name, "rb");
  
  if(fp == NULL)
    return -1;
    
  char name[STR_MAX];
  
  //1) Read Table Name
  if(fread(&name, sizeof(char[STR_MAX]),1,fp) == 0)
  {
    fclose(fp);
    return -1;
  }
  
  
  //Create Table
  if(createTable(name) < 0)
  {
    fclose(fp);
    return -1;
  }
  
  //2) Read Column Count
  int columnCount = 0;
  if(fread(&columnCount,sizeof(int),1,fp) == 0)
  {
    fclose(fp);
    return -1;
  }

  
  //3) Read Column Names and data Types columnCount times
  int i = 0;
  char colName[STR_MAX];
  int dataType = 0;
  for(i = 0;i < columnCount;++i)
  {
    //ColumnName
    if(fread(&colName,sizeof(char[STR_MAX]),1,fp) == 0)
    {
      fclose(fp);
      return -1;
    }
    
    //DataType
    if(fread(&dataType,sizeof(int),1,fp) == 0)
    {
      fclose(fp);
      return -1;
    }
    
    //Add Column to table
    if(addColumn(name,colName,dataType) < 0)
    {
      fclose(fp);
      return -1;
    }
  }
  
  
  //4) Read rows
  
  char row[1000];
  while(fread(&row,sizeof(char[1000]),1,fp) != 0)
  {
    row[999] = '\0';
    insertRow(name,row);
  }
  
  
  fclose(fp);
  return 0;
}

static char* getDBName(const char *str)
{
  char *ret = malloc(sizeof(char) * STR_MAX);
  
  int i = 0;
  int j = 0;
  for(j = 0;str[j] != '\0';j++)
  {
    ret[i++] = str[j];
    if(str[j] == '/')
      i = 0;
  }
  
  ret[i] = '\0';
  
  return ret;
}

void mystrcat(char *dest, char *src)
{
  //This string concatenation assumes there's
  //enough space to concatenate src in dest
  while(*dest++ != '\0');
  dest--;
  for(;*src != '\0';*dest++ = *src++);
  *dest = '\0';
}

int saveDB(const char* db_name)
{
  #define SIZE 100000

  char command[SIZE] = {'\0'};
  char files[SIZE] = {'\0'};
  char metaDataFile[STR_MAX] = {'\0'};
  char *temp = getDBName(db_name);
  
  memset(command, 0, SIZE);
  sprintf(command, "zip %s.zip ", db_name);
  sprintf(metaDataFile, "%s_meta_data.mtd", temp);
  
  FILE *file = fopen(metaDataFile, "wb");
  if(file == NULL)
  {
    free(temp);
    return -1;
  }
  
  table_p table = NULL;
  
  int count = 0;
  
  for(table = tables;table != NULL;table = table->hh.next)
  {
    if(saveTable(table->name) != -1)
    { 
      count += strlen(table->name) + 2;
      if(count < STR_MAX - 1)
      {
        mystrcat(files, table->name);
        mystrcat(files, " ");
      }
    }
  }

  fwrite(files, sizeof(char), SIZE, file);
  fclose(file);

  mystrcat(command, metaDataFile);
  mystrcat(command, " ");
  mystrcat(command, files);
  system(command);
  memset(command, 0, SIZE);
  mystrcat(command, "rm ");
  mystrcat(command, files);
  mystrcat(command, metaDataFile);
  system(command);
  
  free(temp);
}

int loadDB(const char* db_name)
{
  deleteAllTables();
  
  #define SIZE 100000
  
  char command[SIZE] = {'\0'};
  char files[SIZE] = {'\0'};
  char fileName[STR_MAX] = {'\0'};
  char metaDataFile[STR_MAX] = {'\0'};
  char *temp = getDBName(db_name);
  
  sprintf(command, "unzip %s.zip", db_name);
  system(command);
  
  sprintf(metaDataFile, "%s_meta_data.mtd", temp);
  
  FILE *file = fopen(metaDataFile, "r");
  if(file == NULL)
  {
    free(temp);
    return -1;
  }
    
  fread(files, sizeof(char), SIZE, file);
  fclose(file);
  
  //Load Tables
  int i = 0;
  int j = 0;
  //printf("Files: %s\n", files);
  for(i = 0, j = 0;files[j] != '\0';++j, ++i)
  {
    if(files[j] == ' ')
    {
      fileName[i] = '\0';
      loadTable(fileName);
      i = -1;
    }
    else
    {
      fileName[i] = files[j];
    }
  }
  
  command[0] = '\0';
  strcat(command, "rm ");
  strcat(command, files);
  strcat(command, metaDataFile);
  system(command);
  
  free(temp);
}

int columnExists(const char* table_name, const char* column_name)
{
  table_p table = NULL;
  
  //Retrieve table from hash table
  HASH_FIND_STR(tables, table_name, table);
  
  if(table == NULL)
    return 0;
  
  int i;
  //Search in column array if given column exists
  for(i = 0;i < table->columnCount;++i)
  {
    if(strcmp(table->column_names[i],column_name) == 0)
      return 1;
  }
  return 0;
}

int getColumnDataType(const char* table_name, const char* column_name, int *offset)
{
  table_p table = NULL;
  
  HASH_FIND_STR(tables, table_name, table);
  
  if(table == NULL)
    return -1;
    
  int i = 0;
  int j = 0;
  int stop = 0;
  int mask0, mask1;
  int dataTypeSize = 0;
  *offset = 0;
  while(stop != 1)
  {
    mask0 = MASK0(table->hdr->descriptor[i]);
    mask1 = MASK1(table->hdr->descriptor[i]);

    if(mask0 == end) //If end of descriptor stop loop
    {
      stop = 1;
    }
    else
    {
      if(mask0 != start) //If not first descriptor 'start'
      {
        //Process element in mask0
        if(strcmp(table->column_names[j++], column_name) == 0)
          return mask0;
        
        DataTypeSize(mask0, dataTypeSize);
        *offset += dataTypeSize;
      }
      if(mask1 == end) //If end of descriptor stop loop
      {
        stop = 1;
      }
      else
      {
        //Process element in mask1
        if(strcmp(table->column_names[j++], column_name) == 0)
          return mask1;
          
        DataTypeSize(mask1, dataTypeSize);
        *offset += dataTypeSize;
      }
      i++;
    }
  } 
  return 0;
}

table_p getTable(const char* table_name)
{
  table_p table = NULL;
  
  //Retrieve table from hash table
  HASH_FIND_STR(tables, table_name, table);
  
  return table;
}

int deleteAllTables()
{
  table_p table;
  table_p tmp;

  //Remove and delete all tables from hash table
  HASH_ITER(hh, tables, table, tmp) {
    HASH_DEL(tables,table);
    delete(table);
  }
}

#endif
