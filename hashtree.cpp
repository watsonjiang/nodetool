#include "hashtree.h"
#include "leveldb/db.h"
#include "leveldb/comparator.h"
#include "sha1.h"
#include <string>
#include <stdio.h>
#include "debug.h"
#include <stdlib.h> //for exit(1)
using namespace std;
/*
 * The key struct used in myshard to build hashtree looks like this:
 *  0            64            68                 ...
 *  | table_name | segment_num | row_key        |
 *  
 */

#define HASHKEY_PREFIX_LEN 72   //name + seg_num, 64bit padding.
struct HashKey
{
   char table_name[64];
   unsigned int seg_num;
   void * row_key_start;
};

struct hashtree_st 
{
   char * name;
   leveldb::DB * db;   
   leveldb::Comparator * cmp;
};

class SegmentComparator
                 :public leveldb::Comparator
{
   public:
      int Compare(const leveldb::Slice& a, const leveldb::Slice& b) const
      {
         HashKey*  keya = (HashKey*)a.data();
         HashKey*  keyb = (HashKey*)b.data();
         int i;
         if((i = strncmp(keya->table_name, keyb->table_name, 64)) != 0)
         {
            return i;
         }
         if((i = (keya->seg_num - keyb->seg_num)) != 0)
         {
            return i > 0 ? +1 : -1;
         }
         
         size_t j = a.size() > b.size() ? b.size() : a.size(); 
         char * rka = (char*) &keya->row_key_start;
         char * rkb = (char*) &keyb->row_key_start;
         int t = strncmp(rka, rkb, j - HASHKEY_PREFIX_LEN);
         return t;
      }

      const char* Name() const
      {
         return "SegmentComparator";
      }

      void FindShortestSeparator(string*, const leveldb::Slice&) const
      {
      }

      void FindShortSuccessor(string*) const
      {
      }
};

void hashtree_digest_to_hex(const hashtree_digest_t digest, 
                            char *output)
{
   int i,j;
   char *c = output;

   for (i = 0; i < SHA1_DIGEST_SIZE/4; i++) {
      for (j = 0; j < 4; j++) {
         sprintf(c,"%02X", digest[i*4+j]);
         c += 2;
      }
      sprintf(c, " ");
      c += 1;
   }
   *(c - 1) = '\0';
}

hashtree_t
hashtree_new(const char * name)
{
   hashtree_t t = (hashtree_t)malloc(sizeof(struct hashtree_st));
    
   leveldb::Options opt;
   t->cmp = new SegmentComparator();
   opt.comparator = t->cmp;
   opt.create_if_missing = true;
   leveldb::Status s = leveldb::DB::Open(opt, name, &t->db);
   if(!s.ok())
   {
      delete t->cmp;
      free(t);
      return NULL;
   }
   t->name = (char*)malloc(strlen(name) + 1);
   strcpy(t->name, name);
   return t;
}

int
hashtree_destroy(hashtree_t t)
{
   free(t->name);
   delete t->cmp;
   delete t->db;
   free(t);
}

int
hashtree_insert(hashtree_t t,
                const char* tname,
                const char * key,
                hashtree_digest_t hval)
{
   unsigned int ksize = strlen(key);
   leveldb::Slice sv((char*)hval, sizeof(hval));
   unsigned int k = HASHKEY_PREFIX_LEN + ksize;
   HashKey * tmp = (HashKey *)malloc(sizeof(char) * k);
   memset(tmp, 0, k);
   memcpy(tmp->table_name, tname, strlen(tname));
   SHA1_CTX ctx;
   SHA1_Init(&ctx);
   hashtree_digest_t digest = {0};
   SHA1_Update(&ctx, (uint8_t*)key, ksize);
   SHA1_Final(&ctx, digest);
   for(int i = 0; i < 20; i++) {
      if(digest[i] & 0x80 > 0)
      {
         tmp->seg_num = tmp->seg_num << 1;
         tmp->seg_num = tmp->seg_num + 1;   
      }
      else 
      {
         tmp->seg_num = tmp->seg_num << 1;
      }
   }
   memcpy((char*)&tmp->row_key_start, key, ksize); 
   leveldb::Slice sk((char*)tmp, k); 
   leveldb::Status s =
             t->db->Put(leveldb::WriteOptions(), sk, sv); 
   free(tmp);
   debug("hashtree_insert: %s %s %s\n", s.ok() ? "ok" : "fail", tname, key);
   return !s.ok();
}

int
hashtree_remove(hashtree_t t,
                const char* tname,
                const char * key)
{
   unsigned int ksize = strlen(key);
   unsigned int k = HASHKEY_PREFIX_LEN + ksize;
   HashKey * tmp = (HashKey *)malloc(sizeof(char) * k);
   memset(tmp, 0, k);
   memcpy(tmp->table_name, tname, strlen(tname));
   SHA1_CTX ctx;
   SHA1_Init(&ctx);
   hashtree_digest_t digest = {0};
   SHA1_Update(&ctx, (uint8_t*)key, ksize);
   SHA1_Final(&ctx, digest);
   for(int i = 0; i < 20; i++) {
      if(digest[i] & 0x80 > 0)
      {
         tmp->seg_num = tmp->seg_num << 1;
         tmp->seg_num = tmp->seg_num + 1;   
      }
      else 
      {
         tmp->seg_num = tmp->seg_num << 1;
      }
   }
   memcpy((char*)&tmp->row_key_start, key, ksize); 
   leveldb::Slice sk((char*)tmp, k); 
   leveldb::Status s =
              t->db->Delete(leveldb::WriteOptions(), sk); 
   free(tmp);
   debug("hashtree_remove: %s %s %s\n", s.ok() ? "ok" : "fail", tname, key);
   return !s.ok();
}

static
int
_hashtree_put_lvx(hashtree_t t,
                  const char * tname, 
                  const hashtree_digest_t* src, 
                  int lv,
                  unsigned int start, 
                  unsigned int len)
{
   int i = 0;
   HashKey* key = (HashKey*) malloc(sizeof(char) * 128);
   strcpy(key->table_name, "_sys_internal_"); //use a special namespace for hash values.
   key->seg_num = 0;      //always 0 in this case.
   for(i = 0; i< len; i++)
   {
      char buf[59] = {0};
      sprintf(buf, "%s_%d_%d", tname,lv, start+i);
      strcpy((char*)&key->row_key_start, buf);
      unsigned int k = HASHKEY_PREFIX_LEN + strlen(buf);
      leveldb::Slice sk((char *)key,HASHKEY_PREFIX_LEN+strlen(buf));
      leveldb::Slice sv((char *)src[start+i], sizeof(hashtree_digest_t));
      t->db->Put(leveldb::WriteOptions(), sk, sv);
   }
}

static
int 
_hashtree_get_lvx(hashtree_digest_t * dst, 
                  hashtree_t t,
                  const char * tname,
                  const int lv, 
                  unsigned int start, 
                  unsigned int len)
{
   int i = 0;
   HashKey* key = (HashKey*) malloc(sizeof(char) * 128);
   strcpy(key->table_name, "_sys_internal_"); //use a special namespace for hash values.
   key->seg_num = 0;      //always 0 in this case.
   for(i = 0; i< len; i++)
   {
      char buf[59] = {0};
      sprintf(buf, "%s_%d_%d", tname,lv, start+i);
      strcpy((char*)&key->row_key_start, buf);
      leveldb::Slice sk((char *)key,HASHKEY_PREFIX_LEN + strlen(buf));
      string tmp;
      t->db->Get(leveldb::ReadOptions(), sk, &tmp);
      memcpy(dst[i], tmp.c_str(), sizeof(hashtree_digest_t));
   }
   return 0;
}

/*
 * Build the hashtree here. update() operates on a snapshot
 * of leveldb database, it does not block insert().
 * The tree is of height 3, lv0 has 1 node which is sha1sum of all lv1 nodes,
 * lv1 has 1024 nodes, each node is sha1sum of relative lv2 nodes.
 * The number of lv2 nodes is 1024 * 1024, each nodes is sha1sum of a segment.
 */
int
hashtree_update(hashtree_t t, const char * tname)
{
   hashtree_digest_t lv0;
   hashtree_digest_t *lv1 = NULL;
   hashtree_digest_t *lv2 = NULL;
   lv1 = (hashtree_digest_t*)malloc(sizeof(hashtree_digest_t) * 1024);
   memset(lv1, 0, sizeof(hashtree_digest_t) * 1024);
   lv2 = (hashtree_digest_t*)malloc(sizeof(hashtree_digest_t) * 1024 * 1024);
   memset(lv2, 0, sizeof(hashtree_digest_t) * 1024 * 1024);
   //update lv2 hash node.
   // create a snapshot, so we don't break the insert().
   leveldb::ReadOptions opt;
   opt.snapshot = t->db->GetSnapshot();
   leveldb::Iterator *it = t->db->NewIterator(opt);
   unsigned int curr_seg_idx = 0;
   SHA1_CTX ctx;
   SHA1_Init(&ctx);
   HashKey target;
   strcpy(target.table_name, tname);
   target.seg_num = 0;
   target.row_key_start = 0;
   leveldb::Slice starget((char*)&target, sizeof(target));
   for(it->Seek(starget); it->Valid(); it->Next())
   {
      HashKey* key = (HashKey*)it->key().data();
      leveldb::Slice val = it->value();
      if(strcmp(tname, key->table_name) != 0)
      {
         break;
      }
      if(curr_seg_idx < key->seg_num)
      {
         SHA1_Final(&ctx, lv2[curr_seg_idx]);
         curr_seg_idx ++;
         while(curr_seg_idx < key->seg_num)
         {
            SHA1_Init(&ctx);
            SHA1_Final(&ctx, lv2[curr_seg_idx]);
            curr_seg_idx ++;
         }
         SHA1_Init(&ctx);
      }
      SHA1_Update(&ctx, 
                 (const unsigned char*)it->value().data(), 
                 it->value().size());
   }
   SHA1_Final(&ctx, lv2[curr_seg_idx]);
   curr_seg_idx ++;
   delete it;
   t->db->ReleaseSnapshot(opt.snapshot);
   while(curr_seg_idx < 1024 * 1024)
   {
      SHA1_Init(&ctx);
      SHA1_Final(&ctx, lv2[curr_seg_idx]);
      curr_seg_idx++;
   }
   _hashtree_put_lvx(t, tname, lv2, HASHTREE_LV2, 0, 1024*1024); 
   //update lv1 hash node.
   for(int i = 0; i < 1024; i++)
   {
      SHA1_Init(&ctx);   
      for(int j = 0; j < 1024; j++)
      {
         SHA1_Update(&ctx, lv2[i*1024+j], sizeof(hashtree_digest_t));   
      }
      SHA1_Final(&ctx, lv1[i]);
   }
   _hashtree_put_lvx(t, tname, lv1, HASHTREE_LV1, 0, 1024);
   //update lv0 hash node.
   SHA1_Init(&ctx);
   for(int i = 0; i < 1024; i++)
   {
      SHA1_Update(&ctx, lv1[i], sizeof(hashtree_digest_t));   
   }
   SHA1_Final(&ctx, lv0);
#if _DEBUG_
   char buf[80] = {0};
   hashtree_digest_to_hex(lv0, buf);
   debug("hashtree_update: %s digest %s\n", tname, buf);
#endif
   _hashtree_put_lvx(t, tname, &lv0, HASHTREE_LV0, 0, 1);
}

int
hashtree_get_digest(hashtree_digest_t * dst,
                    hashtree_t t, 
                    const char * tname,
                    const int lv,
                    const unsigned int start_idx,
                    const unsigned int len)
{
  return _hashtree_get_lvx(dst, t, tname, lv, start_idx, len); 
}

hashtree_segment_t
hashtree_get_segment(hashtree_t t, const char * tname, unsigned int idx)
{
   leveldb::ReadOptions opt;
   opt.snapshot = t->db->GetSnapshot();
   leveldb::Iterator *it = t->db->NewIterator(opt);
   HashKey start;
   strcpy(start.table_name, tname);
   start.seg_num = idx;
   start.row_key_start = NULL;
   leveldb::Slice starget((char*)&start, sizeof(HashKey)); 
   unsigned int alloc_size = 1024; 
   hashtree_segment_t seg = 
          (hashtree_segment_t)malloc(sizeof(char) * alloc_size); 
   hashtree_segment_entry_t tmp;
   int ENTRY_PREFIX_LEN = (char*)&tmp.kstart - (char*)&tmp;
   unsigned int offset = 0; 
   for(it->Seek(starget); it->Valid(); it->Next())
   {
      HashKey* key = (HashKey*)it->key().data();
      int rk_size = it->key().size() - HASHKEY_PREFIX_LEN;
      hashtree_digest_t* digest = (hashtree_digest_t*)it->value().data();
      if(key->seg_num != idx)
      {
         break;
      }
      unsigned int entry_size = ENTRY_PREFIX_LEN + rk_size;
      if(offset + entry_size >= alloc_size)
      {
         hashtree_segment_t t = 
             (hashtree_segment_t)malloc(sizeof(char) * alloc_size * 2);
         memcpy(t, seg, sizeof(char) * alloc_size);
         alloc_size = alloc_size * 2;
         seg = t;
      }
      hashtree_segment_entry_t* s_it = 
                    (hashtree_segment_entry_t*)((char*)seg+offset);
      memcpy(s_it->digest, digest, sizeof(hashtree_digest_t));
      s_it->ksize = rk_size;
      memcpy(&s_it->kstart, &key->row_key_start, s_it->ksize);
      offset = offset + entry_size; 
   }
   //finally, put a 'NULL' tag at the end.
   if(offset + sizeof(hashtree_segment_entry_t) >= alloc_size)
   {
      hashtree_segment_t t = 
         (hashtree_segment_t)malloc(sizeof(char) * alloc_size 
                                    + sizeof(hashtree_segment_entry_t));
      memcpy(t, seg, sizeof(char) * alloc_size);
      seg = t;
   }
   hashtree_segment_entry_t* s_it = 
      (hashtree_segment_entry_t*)((char*)seg + offset); //put an empty entry
   memset(s_it, 0, sizeof(hashtree_segment_entry_t));

   return seg;
}

void
hashtree_release_segment(hashtree_segment_t segment)
{
   free(segment);
}
