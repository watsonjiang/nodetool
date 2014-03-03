#include "ht.h"
#include "hashtree.h"
#include <stdlib.h>
#include <signal.h>

extern void * console_main(void* );
extern void * msg_puller(void* );

int 
main(int argc, char ** argv)
{
   /* initialize the hashtree module */
   hashtree_init();
   
   ht_attr_t attr;
   ht_init();
   signal(SIGPIPE, SIG_IGN);

   attr = ht_attr_new();
   ht_attr_set(attr, HT_ATTR_NAME, "msg_puller");
   ht_attr_set(attr, HT_ATTR_STACK_SIZE, 64*1024);
   ht_attr_set(attr, HT_ATTR_JOINABLE, FALSE);
   ht_spawn(attr, msg_puller, NULL);

   ht_attr_set(attr, HT_ATTR_NAME, "console");
   ht_attr_set(attr, HT_ATTR_JOINABLE, TRUE);
   ht_t t = ht_spawn(attr, console_main, NULL);

   ht_join(t, NULL);

   hashtree_destroy();
   return 0;
}
