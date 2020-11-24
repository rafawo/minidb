#ifndef THRASHCAN_C
#define THRASHCAN_C

#include <stdlib.h>
#include <string.h>

//This file is to implement a pseudo garbage collector
//to try to avoid as much as possible memory errors/bugs/leaks

static int instance = 0;

static void **pointerSet;
static unsigned int pointerCount;

void tc_instanceThrashCan()
{
  if(instance == 0)
  {
    pointerSet = NULL;
    pointerCount = 0;
    instance = 1;
  }
}

static void tc_replacePointer(void *old_ptr, void *new_ptr)
{
	if(instance == 0)
		return;

  int i;
  for(i = 0;i < pointerCount;++i)
  {
    if(pointerSet[i] == old_ptr)
    {
      pointerSet[i] = new_ptr;
      return;
    }
  }
}

static void tc_addPointer(void *pointer)
{
	if(instance == 0)
		return;
		
  //if pointer already exist don't add to avoid double free
  int i;
  for(i = 0;i < pointerCount;++i)
  {
    if(pointerSet[i] == pointer)
    {
      return;
    }
  }

  pointerCount += 1;
  void **temp = realloc(pointerSet, pointerCount*sizeof(void*));
  if(temp == NULL)
  {
    exit(1);
  }
  else
  {
    pointerSet = temp;
  } 
  pointerSet[pointerCount - 1] = pointer;
}

void* tc_malloc(int size)
{
	if(instance == 0)
		return NULL;

  void *pt = malloc(size);
  
  if(pt == NULL)
    exit(1); //Terminates process execution if couldn't
             //get memory from heap
    
  tc_addPointer(pt);
  
  return pt;
}

void* tc_realloc(void *ptr, int size)
{
	if(instance == 0)
		return NULL;

  if(ptr == NULL)
  {
    return tc_malloc(size);
  }
  
  void *pt = realloc(ptr, size);
  if(pt == NULL)
  {
    exit(1); //Terminates process execution if couldn't
             //get memory from the heap
  }
  
  //When we do realloc, we will replace old pointer (it's freed by realloc)
  tc_replacePointer(ptr, pt);
  
  return pt;
}

char* tc_strdup(const char *str)
{
	if(instance == 0)
		return NULL;
		
  char *ptr = strdup(str);
  if(ptr == NULL)
  {
    exit(1);
  }
  tc_addPointer(ptr);
  return ptr;
}

void tc_emptyThrashCan()
{
	if(instance == 0)
		return;
		
  int i;
  for(i = 0;i < pointerCount;i++)
  {
    free(pointerSet[i]);
  }
  
  free(pointerSet);
  
  pointerSet = NULL;
  pointerCount = 0;
  
  instance = 0;
}

#endif
