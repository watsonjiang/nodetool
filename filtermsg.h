#ifndef __FILTER_MSG_H__
#define __FILTER_MSG_H__
#include <string>
typedef std::string (*filter_func_t)(const std::string &);
struct filter_entry_st;
typedef struct filter_entry_st * filter_list_t;

extern int filter_list_add(char * tname, char * colname,
                           filter_func_t func);

extern int filter_list_rm(char * tname, char * colname);

extern filter_func_t filter_list_get(char * tname,
                                     char * colname);

#endif   //__FILTER_MSG_H__
