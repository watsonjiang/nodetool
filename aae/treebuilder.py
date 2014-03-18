#!/usr/bin/python

from pyaae import Hashtree
import pyaae
import hashlib

class TreeBuilder:
   def __init__(self, tree, fl, tname):
      self._name = tname
      self._tree = tree
      self._filter = fl

   def add_entry(self, key, digest):
      self._tree.insert(self._name, key, digest)
    
   def copy_tree(self, dst):
      for i in xrange(0, 1024 * 1024):
         t = dst.get_segment(self._name, i) 
         for k in t:
            dst.remove(self._name, k)
         t = self._tree.get_segment(self._name, i)
         for k in t:
            dst.insert(self._name, k, t[k])

   def build_tree_from_file(self, cols, keys, file_):
      pyaae.parse_dumpfile(file_, 
                   lambda x : _handle_record(self, cols, keys, x))

def _handle_record(treebuilder, cols, keys, val):
   kv = dict(zip(cols, val))  #I love this lang:)
   keys.sort()
   hk = '#'.join([kv[i] for i in keys])
   sorted(kv)
   h = hashlib.sha1()
   for k, v in kv.items():
      v = treebuilder._filter.norm_data(
                               treebuilder._name, k, v)
      h.update(v)
   treebuilder.add_entry(hk, h.digest())


    

def tree_cmp(tname, t1, t2):
   t1.update(tname)
   t2.update(tname)
   h1 = t1.get_digest(tname, 0) 
   h2 = t2.get_digest(tname, 0)
   if h1 == h2:
      print "The Same."
      return
   h1 = t1.get_digest(tname, 1, 0, 1024)
   h2 = t2.get_digest(tname, 1, 0, 1024)
   for i in xrange(0, 1024):
      if h1[i] != h2[i]:
         print i
         h3 = t1.get_digest(tname, 2, 1024*i, 1024)
         h4 = t2.get_digest(tname, 2, 1024*i, 1024)
         for j in xrange(0, 1024):
            if h3[j] != h4[j]:
               print j
               s1 = t1.get_segment(tname, 1024*i+j)
               s2 = t2.get_segment(tname, 1024*i+j)
               ks1 = set(s1)
               ks2 = set(s2)
               c = ks1.intersection(ks2)
               for k in c:
                  if s1[k] != s2[k]:
                     print k, s1[k].encode("hex"), s2[k].encode("hex")
               d = ks1.symmetric_difference(ks2)
               for k in d:
                  if k in ks1:
                     print k, ks1[k].encode("hex"), None
                  else:
                     print k, None, ks2[k].encode("hex")
   return 
