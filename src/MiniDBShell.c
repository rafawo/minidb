#include <stdio.h>
#include "TableFunctions.h"
#include "queryParser.tab.c"
#include "lex.yy.c"

#define MAX STR_MAX //!< Maximum size for a string
#define CMDS 16 //!< Number of available commands

#define str(x) #x //!< string extension
#define COMMAND(x) { str(x), x ## _command } //!< macro to insert command instance

/*! \file MiniDBShell.c
 * \brief File with the implementation of MiniDB Shell.
 * 
 * The implementation of this file is responsible for the shell
 * interpreter of commands to give table manipulation without the
 * need of a query engine.
 * One of the supported commands is the query engine (query your_query).
 *
 * Since MiniDB can be executed with arguments from command line, command functions
 * support argument count and argument array manipulation. The functions contained in
 * this file are not intended to be called by other code, this are just for the 
 * main functionality.
 */

int createTable_command(int argc, char *argv[], int index);
int deleteTable_command(int argc, char *argv[], int index);
int addColumn_command(int argc, char *argv[], int index);
int insertRow_command(int argc, char *argv[], int index);
int clear_command(int argc, char *argv[], int index);
int printTable_command(int argc, char *argv[], int index);
int mentionTables_command(int argc, char *argv[], int index);
int saveTable_command(int argc, char *argv[], int index);
int loadTable_command(int argc, char *argv[], int index);
int saveDB_command(int argc, char *argv[], int index);
int loadDB_command(int argc, char *argv[], int index);
int connectTo_command(int argc, char *argv[], int index);
int disconnect_command(int argc, char *argv[], int index);
int query_command(int argc, char *argv[], int index);
int loadQuery_command(int argc, char *argv[], int index);
int help_command(int argc, char *argv[], int index);
void executeQuery(char *filename);

/**
 * Entry point for query parser.
 */
extern FILE * yyin;

/**
 * \brief
 * structure to define a command.
 */
struct cmd {
  /**
   * function name.x ## _command
   */
  char funcname[MAX];
  /**
   * function pointer to command function.
   */
  int (*function)(int argc, char *argv[], int index);
};

/**
 * Array of commands.
 */
struct cmd commands[CMDS] = 
{
  COMMAND(createTable),
  COMMAND(deleteTable),
  COMMAND(addColumn),
  COMMAND(insertRow),
  COMMAND(clear),
  COMMAND(printTable),
  COMMAND(mentionTables),
  COMMAND(saveTable),
  COMMAND(loadTable),
  COMMAND(connectTo),
  COMMAND(disconnect),
  COMMAND(query),
  COMMAND(loadQuery),
  COMMAND(saveDB),
  COMMAND(loadDB),
  COMMAND(help)
};

/**
 * Global variable that stores the name of the current working table (connectTo command).
 */
char table_name[MAX];

/**
 * Flag that specifies if connected to a table.
 */
int connected = 0;

/**
 * Executes a command if it exists in the command array.
 * @param command command to be executed.
 * @return return 0 if command is 'exit' or 'quit',
 *         return 1 if command was executed succesfully.
 */
int executeCommand(char *command)
{
  int i = 0;
  //iterate in command array in search of a matching command
  for(i = 0;i < CMDS;++i)
  {
    if(strcmp(command, commands[i].funcname) == 0)
    {
      //If matching command is found it is called with no arguments
      commands[i].function(0, NULL, 0);
      printf("\n");
      return 1;
    }
  }
  
  //Exit if command is exit or quit
  if(strcmp("exit", command) == 0 || strcmp("quit", command) == 0)
    return 0;
}

/**
 * Executes a set of commands given by the arguments when executed.
 * @param start index where commands start from.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @return returns the last index executed from argv.
 */
int executeCommands(int start, int argc, char *argv[])
{
  //This function correspond to -e type of arguments
  int i = start;
  int cmd = 0;
  
  //Keep iterating until another type of command is found
  for(i = start;i < argc;++i)
  {
    if(strcmp(argv[i], "-e") == 0 ||
       strcmp(argv[i], "-f") == 0 ||
       strcmp(argv[i], "-q") == 0 ||
       strcmp(argv[i], "-h") == 0)
    {
      return i - 1; //return index minus one so main can call
                    //the corresponding function.
    }
  
    //search for matching command
    for(cmd = 0;cmd < CMDS; ++cmd)
    {
      if(strcmp(argv[i], commands[cmd].funcname) == 0)
      {
        //sum to index the consumed arguments
        i += commands[cmd].function(argc, argv, i);
        continue;
      }
    }
    
    //Exit
    if(strcmp("exit", argv[i]) == 0 || strcmp("quit", argv[i]) == 0)
      return 0;
  }
  
  return i;
}

/**
 * Executes a set of commands given by the arguments when executed.
 * @param start index where commands start from.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @return returns the last index executed from argv.
 */
int executeCommandsFromFiles(int start, int argc, char *argv[])
{
  //This function correspond to -f type of arguments
  int i = start;
  int j = 0;
  
  FILE *file = NULL;
  int argumentCount = 0;
  char **commands;
  
  for(i = start;i < argc;++i)
  {
    //Keep traversing until other type of arguments is found
    if(strcmp(argv[i], "-e") == 0 || 
       strcmp(argv[i], "-f") == 0 ||
       strcmp(argv[i], "-q") == 0 ||
       strcmp(argv[i], "-h") == 0)
    {
      return i - 1; //return current index minus one so main can call the
                    //corresponding function.
    }
    
    file = fopen(argv[i], "r"); //Open file specified from argument list
    
    if(file == NULL)
      continue;
    
    //Store in an array all commands found within the file
    commands = malloc(sizeof(char*) * (argumentCount + 1));
    commands[argumentCount] = malloc(sizeof(char)*MAX);
    while(fscanf(file, "%s", commands[argumentCount]) != EOF)
    {
      argumentCount += 1;
      commands = realloc(commands, sizeof(char*) * (argumentCount + 1));
      commands[argumentCount] = malloc(sizeof(char)*MAX);
    }
    
    //Execute all the commands found in the file
    if(executeCommands(0, argumentCount, commands) == 0)
      return 0;
    
    //Free memory used for the commands array
    for(j = 0;j < argumentCount;++j)
    {
      free(commands[j]);
    }
    free(commands);
    argumentCount = 0;
    fclose(file); //Close current file
  }
  
  return i;
}

/**
 * Executes a set of query files given by the argument array.
 * @param start index where commands start from.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @return returns the last index executed from argv.
 */
int executeQueries(int start, int argc, char *argv[])
{
  //This function correspond to -q type of arguments
  int i = start;
  
  for(i = start;i < argc;++i)
  {
    //Keep iterating until other type of arguments are found
    if(strcmp(argv[i], "-e") == 0 ||
       strcmp(argv[i], "-f") == 0 ||
       strcmp(argv[i], "-q") == 0 ||
       strcmp(argv[i], "-h") == 0)
    {
      return i - 1; //Return index minus one so main can call to the
                    //corresponding function
    }
    
    executeQuery(argv[i]); //execute argument as a query file
  }
  
  return i;
}

/**
 * Entry point for MiniDB / MiniDB Shell.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @return return 0 if succesfull.
 */
int main(int argc, char *argv[])
{
  char input[MAX];
  int i;
  
  printf("Welcome!!!!\n\n");
  
  //This block of code will only be executed if this
  //executable is called with arguments
  if(argc >= 2)
  {
    //Flags that determine type of argument
    int execute = 1;   //default
    int fromFile = 0;
    int query = 0;
    
    //Iterate in argument list
    for(i = 1;i < argc;i++)
    {
      if(strcmp(argv[i], "-e") == 0)
      {
        //This flag indicates that the arguments are direct commands
        //with it's corresponding arguments
        execute = 1;
        fromFile = 0;
        query = 0;
        continue;
      }
      else if(strcmp(argv[i], "-f") == 0)
      {
        //This flag indicates that the arguments
        //are file names that contain a set of commands
        execute = 0;
        fromFile = 1;
        query = 0;
        continue;
      }
      else if(strcmp(argv[i], "-q") == 0)
      {
        //This flag indicates that the arguments are
        //file names that contain a set of queries
        execute = 0;
        fromFile = 0;
        query = 1;
        continue;
      }
      else if(strcmp(argv[i], "-h") == 0)
      {
        //This flag indicates that we have to show the help text
        help_command(0, NULL, 0);
        continue;
      }
      
      //Call to the corresponding function depending the type of
      //arguments and change the current index to the consumed arguments
      if(execute == 1)
      {
        i = executeCommands(i, argc, argv); //Execute command
      }
      else if(fromFile == 1)
      {
        i = executeCommandsFromFiles(i, argc, argv); //Parse file to execute commands
      }
      else if(query == 1)
      {
        i = executeQueries(i, argc, argv); //Parse file to execute queries
      }
      
      if(i == 0)
        return 0;
    }
  }
  
  //After interpreting the argument list code waits for
  //user input, that will be interpreted as commands
  
  int result = 0;
  do{
    printf("Enter Command: ");
    scanf("%s", input); //user input
    getchar();
    
    result = executeCommand(input); //execute command
  }while(result != 0);
  
  //Shell has finished, we delete all tables from memory and exit
  deleteAllTables();
  //system("clear");
  
  return 0;
}

/**
 * Command to connect to a table to simplify other commands.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int connectTo_command(int argc, char *argv[], int index)
{
  if(argc > 0)
  {
    printf("\n%s", argv[index]);
    
    if(index + 1 < argc)
    {
      printf(" %s", argv[index + 1]);
      strcpy(table_name, argv[index + 1]);
    }
  }
  else
  {
    printf("\nInput Table name: ");
    scanf("%s", table_name);
    getchar();
  }
  if(exists(table_name))
  {
    printf("\nConnected Succesfully!!\n");
    connected = 1;
  }
  else
  {
    printf("\nCannot connect to non-existent tables. . .\n");
    connected = 0;
  }
  
  return 1;
}

/**
 * Command to disconnecto from current connected table.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int disconnect_command(int argc, char *argv[], int index)
{
  printf("\nDisconnected from %s\n", table_name);
  connected = 0;
  return 0;
}

/**
 * Command to create a new table.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int createTable_command(int argc, char *argv[], int index)
{
  char name[MAX];

  if(connected)
  {
    printf("\nPlease disconnect from table . . .\n");
    return;
  }
  if(argc > 0)
  {
    printf("\n%s", argv[index]);
    if(index + 1 < argc)
    {
      printf(" %s", argv[index + 1]);
      strcpy(name, argv[index + 1]);
    }
  }
  else
  {
    printf("\nInput Table name: ");
    scanf("%s", name);
    getchar();
  }
  if(createTable(name) != NULL)
  {
    printf("\nCreated Succesfully!!\n");
  }
  else
  {
    printf("\nCould not be created\n");
  }
  
  return 1;
}

/**
 * Command to delete given table.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int deleteTable_command(int argc, char *argv[], int index)
{
  char name[MAX];
  
  if(connected)
  {
    printf("\nPlease disconnect from table . . .\n");
    return -1;
  }
  if(argc > 0)
  {
    printf("\n%s", argv[index]);
    if(index + 1 < argc)
    {
      printf(" %s", argv[index + 1]);
      strcpy(name, argv[index + 1]);
    }
  }
  else
  {
    printf("\nInput Table name: ");
    scanf("%s", name);
    getchar();
  }
  if(deleteTable(name) >= 0)
  {
    printf("\nDeleted Succesfully!!\n");
  }
  else
  {
    printf("\nCould not delete Table\n");
  }
  
  return 1;
}

/**
 * Command to print on stdout given table.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int printTable_command(int argc, char *argv[], int index)
{
  char name[MAX];
  
  if(connected)
  {
    printf("\n");
    printTable(table_name);
    printf("\n");
    return 0;
  }
  if(argc > 0)
  {
    if(index + 1 < argc)
      strcpy(name, argv[index + 1]);
  }
  else
  {
    printf("\nInput Table name: ");
    scanf("%s", name);
    getchar();
  }
  printTable(name);
  printf("\n");
  
  return 1;
}

/**
 * Command to print to stdout current existant tables.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int mentionTables_command(int argc, char *argv[], int index)
{
  if(connected)
  {
    printf("\nPlease disconnect from table . . .\n");
    return -1;
  }
  printf("\nShowing current existing tables\n");
  mentionTables();
  
  return 0;
}

/**
 * Command to add a new column description to given table.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int addColumn_command(int argc, char *argv[], int index)
{
  char *t_name;
  char name[MAX];
  char col_name[MAX];
  int data_type = 0;
  
  int align = 0;
  
  if(argc > 0)
  {
    printf("\n%s", argv[index]);
    if(!connected)
    {
      if(index + 1 < argc)
      {
        printf(" %s", argv[index + 1]);
        t_name = argv[index + 1];
      }
      if(index + 2 < argc)
      {
        printf(" %s", argv[index + 2]);
        strcpy(col_name, argv[index + 2]);
      }
      if(index + 3 < argc)
      {  
        printf(" %s", argv[index + 3]);
        data_type = atoi(argv[index + 3]);
      }
    }
    else
    {
      t_name = table_name;
      if(index + 1 < argc)
      {
        printf(" %s", argv[index + 1]);
        strcpy(col_name, argv[index + 1]);
      }
      if(index + 2 < argc)
      {
        printf(" %s", argv[index + 2]);  
        data_type = atoi(argv[index + 2]);
      }
        
      align = -1;
    }
  }
  else
  {
    if(connected == 0)
    {
      printf("\nInput Table name: ");
      scanf("%s", name);
      getchar();
      t_name = name;
    }
    else
    {
      t_name = table_name;
    }
    printf("\nInput new Column name: ");
    scanf("%s", col_name);
    getchar();
    printf("\nInput data type: ");
    scanf("%d",&data_type);
    getchar();
  }
  
  if(addColumn(t_name, col_name, data_type) >= 0)
  {
    printf("\nAdded Column Sucesfully!!\n");
  }
  else
  {
    printf("\nCould not add column . . .!!\n");
  }
  
  return 3 + align;
}

/**
 * Command to insert given row to given table.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int insertRow_command(int argc, char *argv[], int index)
{
  char *t_name;
  char name[MAX];
  char row[1000];
  
  int align = 0;
  
  if(argc > 0)
  {
    printf("\n%s", argv[index]);
    if(!connected)
    {
      if(index + 1 < argc)
      {
        printf(" %s", argv[index + 1]);
        t_name = argv[index + 1];
      }
      if(index + 2 < argc)
      {
        printf(" %s", argv[index + 2]);
        strcpy(row, argv[index + 2]);
      }
    }
    else
    {
      t_name = table_name;
      if(index + 1 < argc)
      {
        printf(" %s", argv[index + 1]);
        strcpy(row, argv[index + 1]);
      }
        
      align = -1;
    }
  }
  else
  {
    if(connected == 0)
    {
      printf("\nInput Table name: ");
      scanf("%s", name);
      getchar();
      t_name = name;
    }
    else
    {
      t_name = table_name;
    }
    
    printf("\nInput row: ");
    scanf("%s", row);
    getchar();
  }
  
  if(insertRow(t_name, &row) >= 0)
  {
    printf("\nInserted Row!!\n");
  }
  else
  {
    printf("\nRow Not Inserted!!\n");
  }
  
  return 2 + align;
}

/**
 * Command to clear current screen.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int clear_command(int argc, char *argv[], int index)
{
  system("clear");
  return 0;
}

/**
 * Command to store given table as a flat file.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int saveTable_command(int argc, char *argv[], int index)
{
  char *t_name;
  char name[MAX];
  
  int align = 0;
  
  if(argc > 0)
  {
    if(connected)
    {
      t_name = table_name;
      align = -1;
    }
    else if(index + 1 < argc)
      t_name = argv[index + 1];
  }
  else
  {
    if(connected == 0)
    {
      printf("\nInput Table name: ");
      scanf("%s", name);
      getchar();
      t_name = name;
    }
    else
    {
      t_name = table_name;
    }
  }
  
  if(saveTable(t_name) >= 0)
  {
    printf("\nTable was saved properly!!\n");
  }
  else
  {
    printf("\nCould not save table!!\n");
  }
  
  return 1 + align;
}

/**
 * Command to load a table from given flat file.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int loadTable_command(int argc, char *argv[], int index)
{
  char name[MAX];

  if(argc > 0)
  {
    if(index + 1 < argc)
      strcpy(name, argv[index + 1]);
  }
  else
  {
    if(connected)
    {
      printf("\nPlease disconnect from table . . .\n");
      return 0;
    }
    printf("\nInput Table name: ");
    scanf("%s", name);
    getchar();
  }
    
  if(loadTable(name) >= 0)
  {
    printf("\nTable was loaded properly!!\n");
  }
  else
  {
    printf("\nCould not load table!!\n");
  }
  
  return 1;
}

/**
 * Command to store to disk all current existant tables as a zip file.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int saveDB_command(int argc, char *argv[], int index)
{
  char name[MAX];
  
  if(argc > 0)
  {
    if(index + 1 < argc)
      strcpy(name, argv[index + 1]);
  }
  else
  {
    printf("\nInput DB name: ");
    scanf("%s", name);
    getchar();
  }
  
  if(saveDB(name) >= 0)
  {
    printf("\nDB was saved properly!!\n");
  }
  else
  {
    printf("\nCould not save DB!!\n");
  }
  
  return 1;
}

/**
 * Command to load given DB (zip file with tables as flat file).
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int loadDB_command(int argc, char *argv[], int index)
{
  char name[MAX];
  
  if(argc > 0)
  {
    if(index + 1 < argc)
      strcpy(name, argv[index + 1]);
  }
  else
  {
    printf("\nInput DB name: ");
    scanf("%s", name);
    getchar();
  }
  
  if(loadDB(name) >= 0)
  {
    printf("\nDB was loaded properly!!\n");
  }
  else
  {
    printf("\nCould not load DB!!\n");
  }
  
  return 1;
}

/**
 * Command that executes a query within a file or given by stdin.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int query_command(int argc, char *argv[], int index)
{
  yyrestart(stdin);
  YY_FLUSH_BUFFER; //Flush buffer for lexical analyzer
	yyparse();
	
	return 0;
}

/**
 * Command to load a query from a file.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int loadQuery_command(int argc, char *argv[], int index)
{
  char name[MAX];
  
  if(argc > 0)
  {
    if(index + 1 < argc)
      strcpy(name, argv[index + 1]);
  }
  else
  {
    printf("\nInput query file: ");
    scanf("%s", name);
    getchar();
  }
  
  //Name is file name
  executeQuery(name);
  
  return 1;
}

/**
 * Function that fills given buffer with .
 * @param buffer buffer in which query is stored.
 * @param file file that contains queries.
 * @param size buffer size (to control buffer overflow).
 * @return return 0 if no more queries to fill from, 
 *         return 1 if there's still more queries to fill from,
 *         return -1 if there's a buffer overflow.
 */
int fillBufferFromFile(char *buffer, FILE *file, const int size)
{
  int i = 0;
  int readBytes = sizeof(char);
  char c = ' ';
  //Ignore first set of whitespaces from file
  while((c == ' ' || c == '\n' || c == '\t') && readBytes == sizeof(char))
  {
    readBytes = fread(&c, sizeof(char), 1, file);
  }
  
  if(readBytes != sizeof(char))
   return 1;
  
  char last = '\0';
  int _continue = 0;
  //Append to buffer all text found until ';' is found
  //This will eventually store a single query from the file in the buffer
  while(_continue == 1 || (c != ';' && readBytes == sizeof(char)))
  {
    if(i >= size)
      return -1;
    last = c;
    buffer[i++] = c;
    fread(&c, sizeof(char), 1, file);
    if(_continue && c != '\'')
    {
      break;
    }
    _continue = last == '\'' && c == ';';
  }
  
  if(readBytes != sizeof(char))
    return 1;
  
  buffer[i++] = ';';
  buffer[i] = '\0';
  
  return 0;
}

/**
 * Functions that executes queries from given file.
 * @param filename name of the file that contains queries.
 */
void executeQuery(char *filename)
{
  const int size = 100000; //Buffer size

  FILE *file;
  char *buffer;
  int fileSize = 0;

  file = fopen(filename, "r");
  
  if(file == NULL)
  {
    printf("\nFile \'%s\' doesn't exist!!!\n\n", filename);
    return;
  }

  buffer = malloc(size);

  if(fillBufferFromFile(buffer, file, size) == -1)
    return;

  int finished = 0;
  while(!finished)
  {
    printf("Query:\n%s\n", buffer);
    YY_FLUSH_BUFFER;
    yy_scan_string(buffer);
    yyparse(); //Call the query engine (parse and execute)
    
    //Fill buffer again
    finished = fillBufferFromFile(buffer, file, size);
    if(finished == -1)
      return;
  }
  
  free(buffer);
  fclose(file);
}

/**
 * Command to show help text.
 * @param argc argument count (comes from command line execution).
 * @param argv argument array (comes from command line execution).
 * @param index index from argument array where this command starts.
 * @return return number of arguments consumed for this command after index.
 */
int help_command(int argc, char *argv[], int index)
{
  FILE *file;
  file = fopen("help.dat", "r");
  
  if(file == NULL)
  {
    printf("\nhelp.dat not found!!!\n");
    return 0;
  }
  
  char c;
  while ((c = getc(file)) != EOF)
      putchar(c);
  
  fclose(file);
  
  return 0;
}
