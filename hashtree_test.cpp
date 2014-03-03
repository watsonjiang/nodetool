#include "test.h"
#include "hashtree.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
using namespace std;
void
test_hashtree_insert()
{
   hashtree_digest_t digest = {0};
   for(int i = 0; i < 1; i++)
   {
      digest[0] = i;
      char key[10] = {0};
      sprintf(key, "a%d", i);
      int r = hashtree_insert("testdata", strlen(key), key, digest);
      TEST_ASSERT( 0 == r, "hashtree_insert fail.");
   }
}

void
test_hashtree_update()
{
   hashtree_update("testdata");
   hashtree_digest_t digest = {0};
   char output[80] = {0};
   hashtree_get_digest(&digest, "testdata", 
                       0, 0, 1);
   hashtree_digest_to_hex(digest, output);
   TEST_ASSERT((0 == strcmp(output, "8661F92E CA07F155 DE959DB4 4C6BEBDF BD927CBE")),"hashtree_update does not give root as expect.");
}
   
void
test_hashtree_remove()
{
   hashtree_digest_t digest = {0};
   for(int i = 0; i < 1; i++)
   {
      digest[0] = i;
      char key[10] = {0};
      sprintf(key, "a%d", i);
      int r = hashtree_remove("testdata", strlen(key), key);
      TEST_ASSERT( 0 == r, "hashtree_remove fail.");
   }
}

void
test_hashtree_update1()
{
   hashtree_update("testdata");
   hashtree_digest_t digest = {0};
   char output[80] = {0};
   hashtree_get_digest(&digest, "testdata", 
                       0, 0, 1);
   hashtree_digest_to_hex(digest, output);
   TEST_ASSERT((0 == strcmp(output, "55652341 B61BD613 0A9C7B47 8D86B9C7 989EE884")), "hashtree_update does not give root as expect.");
}
int
main()
{
   hashtree_init();
   test_hashtree_insert();
   test_hashtree_update();
   test_hashtree_remove();
   test_hashtree_update1();
   hashtree_destroy();
}
