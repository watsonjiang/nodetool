#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "ht.h"
#include "hashtree.h"
void console_parse_cmd_line(char* line, int& argc, char**& argv);
void
test_cmd_line_parse()
{
   int argc;
   char** argv = (char**) malloc(sizeof(char*) * 10);
   char line[] = "get_lv_hash hello";
   console_parse_cmd_line(line, argc, argv);
   TEST_ASSERT(2 == argc, "console_parse_cmd_line gives wrong argc.");
   TEST_ASSERT(0 == strcmp(argv[0], "get_lv_hash"), 
               "console_parse_cmd_line gives wrong output.");
   TEST_ASSERT(0 == strcmp(argv[1], "hello"),
               "console_parse_cmd_line gives wrong output.");
}

void update(int fd, int argc, char** argv);
void
test_update()
{
   char *p[10] = {0};
   p[0] = "sinfo";
   update(stdout, 1, p);
}

int
main()
{
   hashtree_init();
   ht_init();
   test_cmd_line_parse();
   test_update();
   hashtree_destroy();
}
