#include "filtermsg.h"
#include "test.h"
#include <string>
#include <stdio.h>
using namespace std;

string dummy_func(const string & )
{
   return string("dummy");
}

void
test_filter_list_add_rm()
{
   char * tname = "testtable";
   char * colname = "testcolum";
   
   filter_list_add(tname, colname, dummy_func);
   filter_func_t f = filter_list_get(tname, colname);

   TEST_ASSERT(f == dummy_func, "filter_list_get does not return expect value");
   filter_list_rm(tname, colname);
   f = filter_list_get(tname, colname);
   TEST_ASSERT(f == NULL, "filter_list_rm does not work.");
 
}

void
test_filter_list_add_rm_1()
{
   char tname[100][20] = {0};
   char colname[100][20] = {0};
   for(int i = 0; i < 100; i++)
   {
      sprintf(tname[i], "t%d", i);
      for(int j = 0; j < 100; j++)
      {
         sprintf(colname[j], "c%d", j);
         filter_list_add(tname[i], colname[j], (filter_func_t) (i+j));
      }
   }
   for(int i = 0; i < 100; i++)
   {
      sprintf(tname[i], "t%d", i);
      for(int j = 0; j < 100; j++)
      {
         sprintf(colname[j], "c%d", j);
         filter_func_t r = filter_list_get(tname[i], colname[j]);
         TEST_ASSERT((r == (filter_func_t)(i+j)), "filter_list_get does not return exptect result.");
      }
   }
   for(int i = 0; i < 100; i++)
   {
      sprintf(tname[i], "t%d", i);
      for(int j = 0; j < 100; j++)
      {
         sprintf(colname[j], "c%d", j);
         filter_list_rm(tname[i], colname[j]);
         filter_func_t r = filter_list_get(tname[i], colname[j]);
         TEST_ASSERT(r == NULL, "filter_list_get does not return exptect result.");
      }
   }
}

int
main()
{
   test_filter_list_add_rm();
   test_filter_list_add_rm_1();

}
