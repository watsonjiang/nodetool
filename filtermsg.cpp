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

struct filter_list_st
{
   unsigned int capacity;
   unsigned int size;
   pthread_mutex_t lock;
   filter_entry_t * list;
};

filter_list_t
filter_list_new()
{
   filter_list_t l = (filter_list_t)malloc(sizeof(struct filter_list_st));
   l->capacity = 64;
   l->list = (filter_entry_t*)malloc(sizeof(filter_entry_t) * l->capacity);
   l->size = 0;
   pthread_mutex_init(&l->lock, NULL);
   return l;
}

int
filter_list_destroy(filter_list_t l)
{
   pthread_mutex_destroy(&l->lock);
   free(l->list);
   l->capacity = 0;
   l->size = 0;
   free(l);
}

static
void
_filter_list_enlarge(filter_list_t l)
{
   // NOTE: l should be locked before entering this function
   int tmp_size = l->capacity * 2;
   filter_entry_t* tmp = 
      (filter_entry_t*)malloc(sizeof(filter_entry_t) * tmp_size);
   memcpy(tmp, l->list, sizeof(filter_entry_t) * l->capacity);
   free(l->list);
   l->list = tmp;
   l->capacity = tmp_size;
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
filter_list_add(filter_list_t l, char * tname, 
                char * colname, filter_func_t func)
{
   pthread_mutex_lock(&l->lock); 
   while(l->size >= l->capacity)
   {
      _filter_list_enlarge(l);
   }
   char * t_tname = (char *)malloc(sizeof(char) * (strlen(tname) + 1));
   strcpy(t_tname, tname);
   char * t_colname = (char *)malloc(sizeof(char) * (strlen(colname) + 1));
   strcpy(t_colname, colname);
   l->list[l->size].colname = t_colname;
   l->list[l->size].tname = t_tname;
   l->list[l->size].func = func;
   l->size ++;
   qsort(l->list, l->size, sizeof(filter_entry_t), _entry_cmp);
   pthread_mutex_unlock(&l->lock);
   return 0; 
}

filter_func_t filter_list_get(filter_list_t l, char * tname, char * colname)
{
   pthread_mutex_lock(&l->lock);
   filter_entry_t key;
   key.tname = tname;
   key.colname = colname;

   filter_entry_t * rst = bsearch(&key, l->list, 
                                  l->size,
                                  sizeof(filter_entry_t), 
                                   _entry_cmp);
   pthread_mutex_unlock(&l->lock);
   return rst == NULL ? NULL : rst->func;
}

int
filter_list_rm(filter_list_t l, char * tname, char * colname)
{
   /* due to rm is not happen so offen, 
      instead of pysically remove the entry,
      set the func to NULL. */
   pthread_mutex_lock(&l->lock);
   filter_entry_t key;
   key.tname = tname;
   key.colname = colname;
   filter_entry_t * rst = bsearch(&key, l->list,
                                  l->size,
                                  sizeof(filter_entry_t),
                                  _entry_cmp);
   if(NULL != rst)
   {
      rst->func = NULL;
   }
   pthread_mutex_unlock(&l->lock);
   return 0;
}

std::string
filter_func_datetime(const std::string& val)
{
   if(val.empty())
   {
      return "0000-00-00 00:00:00";
   }
   return val;
}

std::string
filter_func_number(const std::string& val)
{
   if(val.empty())
   {
      return "0";
   }
   return val;
}

