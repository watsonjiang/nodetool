#include "sha1.h"
#include <string>
#include <string.h>
#include "hashtree.h"
#include <pthread.h>
#include "framework/IMyshardPub.h"
#include "msg_common/PMsgConst.h"
#include <boost/algorithm/string/predicate.hpp>
#include "debug.h"
#include "filtermsg.h"
using namespace std;

typedef map<string, string> msg_t;

class MsgSubClientReceiver : public IMyshardPubHandler
{
   private:
      hashtree_t _hashtree;
      filter_list_t _filter;
   public:
      MsgSubClientReceiver(hashtree_t t, filter_list_t f)
           : _hashtree(t), _filter(f) 
      {
      }

      int32_t onMapMsgHandle(map<string, string>& msg)
      {
         string tname = msg.at(SQ_MSG_KEY_TABLE_NAME); 
         int op = atoi(msg.at(SQ_MSG_KEY_OP_TYPE).c_str());
         string row_key = msg.at(SQ_MSG_KEY_MSG_KEY);
         if(op == 1)
         {
            //a delete op
            //TODO::int s = hashtree_remove(tname.c_str(), row_key.length(),
            //                    row_key.c_str());
            //debug("handle_msg:s:%d del %s\n", s, row_key.c_str()); 
            return true;
         }
         aae_SHA1_CTX ctx;
         aae_SHA1_Init(&ctx);
         hashtree_digest_t digest = {0};
         for(msg_t::const_iterator it = msg.begin(); it != msg.end(); ++it)
         {
            if(boost::starts_with(it->first, "1_"))
            {
               continue;  //ignore system keys.
            }
            string val = it->second;
            debug("handle_msg: %s : %s\n", it->first.c_str(), it->second.c_str());
            //normalize the data using filter.
            filter_list_normalize_data(_filter, tname.c_str(), it->first.c_str(), val);
            //update the digest.
            aae_SHA1_Update(&ctx, (uint8_t*)val.c_str(), val.length());
         }
         aae_SHA1_Final(&ctx, digest);
         char buf[80] = {0};
         hashtree_digest_to_hex(digest, buf);
         int s = hashtree_insert(_hashtree, tname.c_str(),
                                 row_key.c_str(), digest);
         //debug("handle_msg:s:%d set %s : %s\n", s, row_key.c_str(), buf);
        
         return 0;
      }
};

struct _arg_st
{
   char * xml;
   hashtree_t hashtree;
   filter_list_t filter;
};

static
void*
_msg_receive_loop(void* argv)
{
   _arg_st* arg = (_arg_st*) argv;
   MsgSubClientReceiver receiver(arg->hashtree, arg->filter);
   free(arg);
   myshard_pub_main_run(arg->xml, &receiver);
   return NULL;
}

void
msg_puller_start(const char * xmlfile, hashtree_t t, filter_list_t f)
{
   _arg_st* arg = (_arg_st*) malloc(sizeof(_arg_st));
   arg->xml = xmlfile;
   arg->hashtree = t;
   arg->filter = f;
   pthread_t tid;
   pthread_create(&tid, NULL, _msg_receive_loop, (void*)arg); 
}
