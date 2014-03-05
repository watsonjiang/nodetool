#include "test.h"
#include <string>
using namespace std;
extern string _filter_func_datetime(const string& val); 
void
test_filter_func_datetime()
{
   string a = "";
   TEST_ASSERT(_filter_func_datetime(a) == "0000-00-00 00:00:00", "_filter_func_datatime does not return expect value");
   a = "1970-01-01 00:00:00";
   TEST_ASSERT(_filter_func_datetime(a) == "1970-01-01 00:00:00", "_filter_func_datatime does not return expect value");
}


extern string _filter_func_number(const string& val);
void
test_filter_func_number()
{
   string a = "";
   TEST_ASSERT(_filter_func_number(a) == "0", "_filter_func_number does not return expect value.");
   a = "0";
   TEST_ASSERT(_filter_func_number(a) == "0", "_filter_func_number does not return expect value.");
}

int 
main()
{
   test_filter_func_datetime();
   test_filter_func_number();
}
