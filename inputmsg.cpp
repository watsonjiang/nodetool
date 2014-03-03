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
            SHA1_Update(&ctx, (uint8_t*)it->second.c_str(), it->second.length());
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

static
void*
_receive_msg(void* argv)
{
   string xml="./msg_client.xml";
   MsgSubClientReceiver receiver(xml);
   receiver.run();
   return NULL;
}

void *
msg_puller(void* arv)
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
}


