#include <string>
#include <string.h>
#include <ht.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hashtree.h"
#include "debug.h"
using namespace std;

#define CONSOLE_TIMEOUT 1200      //seconds
#define CONSOLE_RET_ACK(fd) ht_write(fd, "ack\n", 4)
#define CONSOLE_RET_NACK(fd) ht_write(fd, "nack\n", 5)

/* from readline.c */
ssize_t ht_readline_ev(int fd, void *buf, size_t buflen, ht_event_t ev_extra);

typedef void (*cmd_func_t)(int fd, int argc, char** argv);


typedef struct console_cmd_st console_cmd_t;
struct console_cmd_st 
{
   const char * cmd_name;
   cmd_func_t cmd_func; 
};

void
console_cmd_rebuild(int fd, int argc, char** argv)
{
   
}

void
console_cmd_update(int fd, int argc, char** argv)
{
   if(argc != 1)
   {
      string ans("update: expect only 1 argument.\n");
      ht_write(fd, ans.c_str(), ans.length());
      CONSOLE_RET_NACK(fd);
      return;
   }
   ht_hand_out();
   char * tname = argv[0];
   hashtree_update(tname);
   ht_get_back();
   CONSOLE_RET_ACK(fd);
}

void 
console_cmd_get_lv_hash(int fd, int argc, char** argv)
{
   if(argc < 2)
   {
      string ans("get_lv_hash: expect at least 2 arguments\n");
      ht_write(fd, ans.c_str(), ans.length());
      CONSOLE_RET_NACK(fd);
      return;
   }
   if(argc > 4)
   {
      string ans("get_lv_hash: expect no more than 4 arguments\n");
      ht_write(fd, ans.c_str(), ans.length());
      CONSOLE_RET_NACK(fd);
      return;
   }
   char* tname = argv[0];
   int lv = atoi(argv[1]);
   int start = 0;
   unsigned int len = pow(1024, lv);
   if(argc >= 3)
   {
      start = atoi(argv[2]);
   }
   if(argc == 4)
   {
      len = atoi(argv[3]);
   }
   hashtree_digest_t* rst = (hashtree_digest_t*)malloc(sizeof(hashtree_digest_t) * len);
   hashtree_get_digest(rst, tname, lv, start, len);
   for(int i = 0; i < len; i++)
   {
      char buf[80] = {0};
      hashtree_digest_to_hex(rst[i], buf);
      ht_write(fd, buf, strlen(buf));
      ht_write(fd, "\n", 1);
   }
   free(rst);
   CONSOLE_RET_ACK(fd);

   return ;
}

void
console_cmd_get_seg(int fd, int argc, char** argv)
{
   if(argc != 2)
   {
      string ans("get_seg: expect two arguments.");
      ht_write(fd, ans.c_str(), ans.length());
      CONSOLE_RET_NACK(fd);
   }
   char* tname = argv[0];
   int idx = atoi(argv[1]);
   hashtree_segment_t seg = hashtree_get_segment(tname, idx);
   hashtree_segment_entry_t* s_it = (hashtree_segment_entry_t*) seg;
   int ENTRY_PREFIX_LEN = ((char*)&s_it->kstart - (char*)s_it);
   while(s_it->ksize != 0)
   {
      char hex_dg[80] = {0};
      hashtree_digest_to_hex(s_it->digest, hex_dg);
      char * buf = (char*) malloc(sizeof(char) * (s_it->ksize+1+1+80)); 
      sprintf(buf, "%s %s", &s_it->kstart, hex_dg);
      ht_write(fd, buf, strlen(buf));
      ht_write(fd, "\n", 1);
      free(buf);
      //move to next entry.
      s_it = (hashtree_segment_entry_t*)((char*)seg + ENTRY_PREFIX_LEN + s_it->ksize); 
   } 
   CONSOLE_RET_ACK(fd);
   return;
}

static console_cmd_t _cmd_list[] = {
   {
      "get_lv_hash",
      console_cmd_get_lv_hash
   },
   {
      "get_seg",
      console_cmd_get_seg
   },
   {
      "update",
      console_cmd_update
   },
   {
      "rebuild",
      console_cmd_rebuild
   }
};

void
console_parse_cmd_line(char * line, int& argc, char**& argv)
{
   argc = 0;
   char* p = strtok(line, " ");
   while(p != NULL && argc < 10)
   {
      argv[argc] = p;
      argc++;
      p = strtok(NULL, " ");
   }
}

void*
console_session(void* argv)
{
   int fd = (int)argv;
   ht_event_t ev_tmout = NULL;
   char line[255] = {0};
   int n = 0;
   for(;;)
   {
      if(ev_tmout == NULL)
         ev_tmout = ht_event(HT_EVENT_TIME, ht_timeout(CONSOLE_TIMEOUT,0)); //wait for 20 seconds.
      else
         ev_tmout = ht_event(HT_EVENT_TIME|HT_MODE_REUSE, ev_tmout, ht_timeout(CONSOLE_TIMEOUT,0));
      n = ht_readline_ev(fd, line, 255, ev_tmout);
      if(n == -1 && ht_event_status(ev_tmout) == HT_STATUS_OCCURRED) 
      {
         //client does not send input data for 20 seconds.
         close(fd);
         ht_event_free(ev_tmout, HT_FREE_THIS);
         return NULL;
      }
      if(n <= 0) 
      {
         //client close.
         close(fd);
         ht_event_free(ev_tmout, HT_FREE_THIS);
         return NULL;
      }
      line[n-1] = 0; 

      int argc = 0;
      char ** argv = (char**) malloc(sizeof(char*) * 10); //max 10 args
      console_parse_cmd_line(line, argc, argv);
     
      if(argc == 0)
      {
         string ans = "unknown command.\n";
         ht_write(fd, ans.c_str(), ans.length());
         CONSOLE_RET_NACK(fd);
         free(argv);
         continue;
      }
      if(strcmp(argv[0], "exit") == 0)
      {
         close(fd);
         ht_event_free(ev_tmout, HT_FREE_THIS);
         free(argv);
         return NULL;
      }
      bool is_cmd_matched = false;
      for(int i = 0; i < sizeof(_cmd_list)/sizeof(console_cmd_t); i++)
      {
         if(strcmp(argv[0], _cmd_list[i].cmd_name) == 0)
         {
            is_cmd_matched = true;
            _cmd_list[i].cmd_func(fd, argc-1, &argv[1]);
            break;
         }
      }
      if(!is_cmd_matched)
      {
         string ans = "unknow command. \n";
         ht_write(fd, ans.c_str(), ans.length());
         CONSOLE_RET_NACK(fd);
      }
      free(argv);
   }
   return NULL;
}

struct _addr
{
   unsigned int port;
   void* ip_start;
};

void*
console_main_loop(void *argv)
{
   _addr* p = (_addr*) argv;
   ht_attr_t attr;
   attr = ht_attr_new();
   ht_attr_set(attr, HT_ATTR_STACK_SIZE, 64 * 1024);
   ht_attr_set(attr, HT_ATTR_JOINABLE, FALSE);
   sockaddr_in sar;
   protoent *pe;
   sockaddr_in peer_addr;
   int peer_len;
   int sa, sw;
   int port;
 
   pe = getprotobyname("tcp");
   sa = socket(AF_INET, SOCK_STREAM, pe->p_proto);
   sar.sin_family = AF_INET;
   inet_aton((char*)&p->ip_start, &sar.sin_addr);
   sar.sin_port = htons(p->port);
   int val = 1;
   setsockopt(sa, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)); 
   bind(sa, (sockaddr *)&sar, sizeof(sockaddr_in));
   listen(sa, 10);

   while(1){
      peer_len = sizeof(peer_addr);
      sw = ht_accept(sa, (sockaddr*)&peer_addr, &peer_len);
      ht_spawn(attr, console_session, (void*)sw); 
   }
}

void
console_start(char * ip, unsigned int port)
{
   _addr* p = (_addr*) malloc(sizeof(unsigned int) + strlen(ip) + 1);
   p->port = port;
   strcpy((char*)&p->ip_start, ip); 
   ht_attr_t attr;
   attr = ht_attr_new();
   ht_attr_set(attr, HT_ATTR_NAME, "console");
   ht_attr_set(attr, HT_ATTR_JOINABLE, TRUE);
   ht_t t = ht_spawn(attr, console_main_loop, (void*)p);

   ht_join(t, NULL); //wait until exit.
}
