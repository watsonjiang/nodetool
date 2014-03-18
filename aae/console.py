#!/usr/bin/python

# a module implement myshard aae console 
# protocol
from twisted.internet.protocol import Factory
from twisted.protocols.basic import LineReceiver
from pyaae import Hashtree
from treebuilder import TreeBuilder
from os import listdir
from os.path import isfile, join, isdir
import shutil
from metadata import Metadata
import pyaae

def cmd_get_lv_hash(tree, args):
   tname = args[1]
   lv = int(args[2])
   if(len(args) > 3):
      start = int(args[3])
   else:
      start = 0
   if(len(args) > 4):
      length = int(args[4])
   else:
      length = 1
   digest_list = tree.get_digest(tname, lv, start, length)
   rsp = []
   for i in digest_list:
       rsp.append(i.encode("hex").upper())
   rsp.append("ack")
   return rsp

def cmd_get_seg(tree, args):
   tname = args[1]
   idx = int(args[2])
   kv_list = tree.get_segment(tname, idx)
   rsp = []
   for i,j in kv_list.items():
     rsp.append(" ".join((i, j.encode("hex").upper())))
   rsp.append("ack")
   return rsp
     

def cmd_update(tree, args):
   tname = args[1]
   tree.update(tname)
   return ["ack"]

def cmd_get_filter_list(fl):
   l = fl.get_list()
   rsp = []
   for i in l.keys():
      rsp.append(" ".join((i, str(l[i]))))
   rsp.sort()
   rsp.append("ack")
   return rsp

def cmd_rebuild_tree(tree, conf, args):
   if len(args) != 2 :
      return ["expect 1 argument.", "nack"]
   tname = args[1]
   #use tname.db as tmp db folder name
   dbname = ".".join((tname, "db"))
   if isdir(dbname): 
      shutil.rmtree(dbname)
   tmp_tree = Hashtree(dbname)
   tmp_tb = TreeBuilder(tmp_tree, tname)
   metadb = Metadata(conf.get_metadb_info(), conf.get_machine_room_no())
   cols = metadb.get_cols(tname)
   keys = metadb.get_keys(tname)
   files = [f for f in listdir("/tmp/b") if isfile(join("/tmp/b", f))]
   for f in files:
      tmp_tb.build_tree_from_file(cols, keys, join("/tmp/b", f)) 
   tmp_tb.copy_tree(tree) 

def cmd_debug(args):
   if len(args) != 2:
      return ["expect 1 argument.", "nack"]
   onoff = args[1]
   if onoff.lower() == 'on':
      pyaae.debug_on()
   elif onoff.lower() == 'off':
      pyaae.debug_off()
   else:
      return ['unknown argument. should be on/off', 'nack']
   return ['ack']

class AaeConsole(LineReceiver):
   def __init__(self, t, fl, conf):
     self._CMD_DICT = {
        "get_lv_hash" : (lambda x : cmd_get_lv_hash(t, x)),
        "get_seg" : (lambda x : cmd_get_seg(t, x)),
        "update" : (lambda x : cmd_update(t, x)),
        "rebuild" : (lambda x : cmd_rebuild_tree(t, conf, x)),
		  "list_filter" : (lambda x : cmd_get_filter_list(fl)),
        "debug" : cmd_debug
     }

   def lineReceived(self, data):
      rsp = []
      t = data.split(" ")
      il = []
      for i in t:
         i = i.strip()
         if i != "":
            il.append(i)
      if len(il) > 0 and il[0] in self._CMD_DICT:
         rsp = self._CMD_DICT[il[0]](il)
      else:
         rsp = (["unknown command.", "nack"])
      for line in rsp:
         self.sendLine(line)

class AaeConsoleFactory(Factory):
   def __init__(self, tree, fl, conf):
      self._tree = tree
      self._conf = conf 
      self._filterlist = fl

   def buildProtocol(self, addr):
      p = AaeConsole(self._tree, self._filterlist, self._conf)
      p.setLineMode()
      p.delimiter = '\n'
      return p
