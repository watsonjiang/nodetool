#include "sha1.h"
#include "ht.h"
#include <string>
#include <string.h>
#include <fstream>
#include "hashtree.h"
#include <pthread.h>
#include <queue>
#include "msg_client_framework/MsgSubClient.h"
#include "msg_common/PMsgConst.h"
#include <boost/algorithm/string/predicate.hpp>
#include "debug.h"
#include "filtermsg.h"
#include "my_global.h"
#include "mysql.h"
using namespace std;

typedef map<string, string> msg_t;

class MsgSubClientReceiver : public MsgSubClient
{
   public:
      MsgSubClientReceiver(const string& xml): MsgSubClient(xml) {}
      virtual bool handleMsg(const map<string, string>& msg)
      {
         string tname = msg.at(SQ_MSG_KEY_TABLE_NAME); 
         int op = atoi(msg.at(SQ_MSG_KEY_OP_TYPE).c_str());
         string row_key = msg.at(SQ_MSG_KEY_MSG_KEY);
         if(op == 1)
         {
            //a delete op
            int s = hashtree_remove(tname.c_str(), row_key.length(),
                                row_key.c_str());
            debug("handle_msg:s:%d del %s\n", s, row_key.c_str()); 
            return true;
         }
         SHA1_CTX ctx;
         SHA1_Init(&ctx);
         hashtree_digest_t digest = {0};
         for(msg_t::const_iterator it = msg.begin(); it != msg.end(); ++it)
         {
            if(boost::starts_with(it->first, "1_"))
            {
               continue;  //ignore system keys.
            }
            string val = it->second;
            printf("handle_msg: %s : %s\n", it->first.c_str(), it->second.c_str());
            //normalize the data using filter.
            filter_func_t f = filter_list_get(tname.c_str(), it->first.c_str());
            if(NULL != f)
            {
               val = f(val); 
            }
            //update the digest.
            SHA1_Update(&ctx, (uint8_t*)val.c_str(), val.length());
         }
         SHA1_Final(&ctx, digest);
         char buf[80] = {0};
         hashtree_digest_to_hex(digest, buf);
         int s = hashtree_insert(tname.c_str(), row_key.length(), 
                         row_key.c_str(), digest);
         debug("handle_msg:s:%d set %s : %s\n", s, row_key.c_str(), buf);
        
         return true;
      }
};

string
_filter_func_datetime(const std::string& val)
{
   if(val.empty())
   {
      return "0000-00-00 00:00:00";
   }
   return val;
}

string
_filter_func_number(const std::string& val)
{
   if(val.empty())
   {
      return "0";
   }
   return val;
}


static
void
_update_filter_list()
{
   MYSQL * con = mysql_init(NULL);
   if(NULL == con)
   {
      debug("update_filter_list: init mysql con fail. %s\n", mysql_error(con));
      return;
   }
   if(mysql_real_connect(con, "172.19.34.40", "myshard", "myshard",
                          "myshard_metadata1", 3306, NULL, 0) == NULL)
   {
      debug("update_filter_list: fail to connect mysql. %s\n", mysql_error(con));
      mysql_close(con);
      return;
   }
   if(mysql_query(con, 
            "select "
            "table_name, column_name, data_type "
            "from myshard_table_columns"))
   {
      debug("update_filter_list: fail to get data. %s\n", mysql_error(con));
      mysql_close(con);
      return;
   }
   MYSQL_RES *rst = mysql_store_result(con);
   if(NULL == rst)
   {
      debug("update_filter_list: %s\n", mysql_error(con));
      mysql_close(con);
      return;
   }
   MYSQL_ROW row;
   while((row=mysql_fetch_row(rst)))
   {
      //printf("%s : %s : %s\n", row[0], row[1], row[2]); 
      if(strcasecmp(row[2], "datetime"))
      {
         filter_list_add(row[0], row[1], _filter_func_datetime);
      }
      else if(strcasecmp(row[2], "bigint"))
      {
         filter_list_add(row[0], row[1], _filter_func_number);
      }
      else if(strcasecmp(row[2], "integer"))
      {
         filter_list_add(row[0], row[1], _filter_func_number);
      }
   }
   mysql_free_result(rst);
   mysql_close(con);
}

static
void*
_receive_msg(void* argv)
{
   string xml="./msg_client.xml";
   MsgSubClientReceiver receiver(xml);
   receiver.run();
   return NULL;
}

static
void *
_msg_puller(void* arv)
{
   /* read file */
//   ifstream file("data.txt");
//   string str;
//   SHA1_CTX ctx;
//   hashtree_digest_t digest;
//   while(getline(file, str))
//   {
//      size_t pos = str.find_first_of('\t');
//      string key = str.substr(0, pos-1);
//      string value = str.substr(pos+1);
//      SHA1_Init(&ctx);
//      SHA1_Update(&ctx, (const unsigned char *)value.c_str(), value.length());
//      SHA1_Final(&ctx, digest);
//      hashtree_insert("data", key.length(), key.c_str(),
//                      digest);
//   }

   pthread_t t;
   pthread_create(&t, NULL, _receive_msg, NULL); 
   while(1)
   {
      _update_filter_list();
      ht_sleep(10);
   }
}

void
msg_puller_start()
{
   ht_attr_t attr;
   attr = ht_attr_new();
   ht_attr_set(attr, HT_ATTR_NAME, "msg_puller");
   ht_attr_set(attr, HT_ATTR_STACK_SIZE, 64*1024);
   ht_attr_set(attr, HT_ATTR_JOINABLE, FALSE);
   ht_spawn(attr, _msg_puller, NULL);
}
