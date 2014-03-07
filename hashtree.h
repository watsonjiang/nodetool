#ifndef __MYSHARD_HASHTREE_H__
#define __MYSHARD_HASHTREE_H__
typedef unsigned char hashtree_digest_t[20];
typedef struct hashtree_segment_entry_st hashtree_segment_entry_t;
struct hashtree_segment_entry_st
{
   hashtree_digest_t digest;
   unsigned int ksize;
   void * kstart;
};

typedef void* hashtree_segment_t;  //a big mem block of entries.
                                   //end with a empty entry.

enum {
   HASHTREE_LV0,
   HASHTREE_LV1,
   HASHTREE_LV2
};

typedef struct hashtree_st* hashtree_t;
extern hashtree_t hashtree_new(const char *);
extern int hashtree_insert(hashtree_t t,
                           const char * tname,
                           const char * key,
                           hashtree_digest_t hval);
extern int hashtree_remove(hashtree_t t,
                           const char * tname,
                           const char * key);
extern int hashtree_update(hashtree_t t,
                           const char * tname);
extern int hashtree_get_digest(hashtree_digest_t * dst, 
                               hashtree_t t,
                               const char * tname, 
                               const int lv, 
                               const unsigned int start_idx, 
                               const unsigned int len);
extern hashtree_segment_t hashtree_get_segment(hashtree_t t,
                                               const char * tname, 
                                               unsigned int idx);
extern void hashtree_release_segment(hashtree_segment_t segment);
extern void hashtree_digest_to_hex(const hashtree_digest_t digest, 
                                   char * output);
extern int hashtree_destroy(hashtree_t t);
#endif //__MYSHARD_HASHTREE_H__
