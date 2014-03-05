#include "ht.h"
#include "hashtree.h"
#include <stdlib.h>
#include <signal.h>

extern void console_start();
extern void msg_puller_start();

int 
main(int argc, char ** argv)
{
   /* initialize the hashtree module */
   hashtree_init();
   
   ht_init();
   signal(SIGPIPE, SIG_IGN);

   /* start msg puller. it pull messages
      from my shard.*/
   msg_puller_start();

   /* start console. it open a port 
      to accept admin control.
      this function call never return.
    */
   console_start(); 

   hashtree_destroy();
   return 0;
}
