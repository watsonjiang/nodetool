#ifndef __FILTER_MSG_H__
#define __FILTER_MSG_H__
#include <string>
typedef std::string (*filter_func_t)(const std::string &);
struct filter_entry_st;
typedef struct filter_list_st * filter_list_t;

extern filter_list_t filter_list_new();
extern int filter_list_destroy(filter_list_t l);
extern int filter_list_add(filter_list_t l,
                           char * tname, char * colname,
                           filter_func_t func);

extern int filter_list_rm(filter_list_t l,
                          char * tname, char * colname);

extern filter_func_t filter_list_get(filter_list_t l,
                                     char * tname,
                                     char * colname);

extern std::string filter_func_datetime(const std::string& val);
extern std::string filter_func_number(const std::string& val);
#endif   //__FILTER_MSG_H__
