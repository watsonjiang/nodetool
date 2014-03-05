#include "filtermsg.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
/* 
 * a module maintains a list of data filter.
 * data filter is a function used to normalize
 * the data.
 */

typedef struct filter_entry_st  filter_entry_t;
struct filter_entry_st
{
   char * tname;
   char * colname;
   filter_func_t func;
};
static pthread_mutex_t _list_lock;
static filter_list_t _filter_list = NULL;
static int _list_capacity = 64;
static int _list_size = 0;

static
void
_filter_list_init()
{
   _filter_list = (filter_list_t) malloc(sizeof(filter_entry_t) * _list_capacity);  
   _list_size = 0;
   pthread_mutex_init(&_list_lock, NULL);
}

static
void
_filter_list_enlarge()
{
   int tmp_size = _list_capacity * 2;
   filter_list_t tmp = (filter_list_t)malloc(sizeof(filter_entry_t) * tmp_size);
   memcpy(tmp, _filter_list, sizeof(filter_entry_t) * _list_capacity);
   free(_filter_list);
   _filter_list = tmp;
   _list_capacity = tmp_size;
}

int
_entry_cmp(const void * a, const void *b)
{
   filter_entry_t * keya = (filter_entry_t*)a;
   filter_entry_t * keyb = (filter_entry_t*)b;
   int t = 0;
   if((t = strcmp(keya->tname, keyb->tname)) == 0)
   {
      return strcmp(keya->colname, keyb->colname);
   }
   return t;
}

int
filter_list_add(char * tname, char * colname, filter_func_t func)
{
   pthread_mutex_lock(&_list_lock); 
   if(_filter_list == NULL)
   {
      _filter_list_init();
   }
   while(_list_size >= _list_capacity)
   {
      _filter_list_enlarge();
   }
   char * t_tname = (char *)malloc(sizeof(char) * (strlen(tname) + 1));
   strcpy(t_tname, tname);
   char * t_colname = (char *)malloc(sizeof(char) * (strlen(colname) + 1));
   strcpy(t_colname, colname);
   _filter_list[_list_size].colname = t_colname;
   _filter_list[_list_size].tname = t_tname;
   _filter_list[_list_size].func = func;
   _list_size ++;
   qsort(_filter_list, _list_size, sizeof(filter_entry_t), _entry_cmp);
   pthread_mutex_unlock(&_list_lock);
   return 0; 
}

filter_func_t filter_list_get(char * tname, char * colname)
{
   pthread_mutex_lock(&_list_lock);
   filter_entry_t key;
   key.tname = tname;
   key.colname = colname;

   filter_entry_t * rst = bsearch(&key, _filter_list, 
                                  _list_size,
                                  sizeof(filter_entry_t), 
                                   _entry_cmp);
   pthread_mutex_unlock(&_list_lock);
   return rst == NULL ? NULL : rst->func;
}

int
filter_list_rm(char * tname, char * colname)
{
   /* due to rm is not happen so offen, 
      instead of pysically remove the entry,
      set the func to NULL. */
   pthread_mutex_lock(&_list_lock);
   filter_entry_t key;
   key.tname = tname;
   key.colname = colname;
   filter_entry_t * rst = bsearch(&key, _filter_list,
                                  _list_size,
                                  sizeof(filter_entry_t),
                                  _entry_cmp);
   if(NULL != rst)
   {
      rst->func = NULL;
   }
   pthread_mutex_unlock(&_list_lock);
   return 0;
}


