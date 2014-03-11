#!/usr/bin/python

# a module implement myshard aae console 
# protocol
from twisted.internet.protocol import Factory
from twisted.protocols.basic import LineReceiver
from pyaae import Hashtree

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
   kv_list = tree.get_seg(tname, idx)
   rsp = []
   for i,j in kv_list:
     rsp.append(" ".join(i, j.encode("hex").upper()))
   rsp.append("ack")
   return rsp
     

def cmd_update(tree, args):
   tname = args[1]
   tree.update(tname)

CMD_DICT = {
   "get_lv_hash" : cmd_get_lv_hash,
   "get_seg" : cmd_get_seg,
   "update" : cmd_update
}

class AaeConsole(LineReceiver):
   def __init__(self, t):
      self._tree = t

   def lineReceived(self, data):
      rsp = []
      t = data.split(" ")
      il = []
      for i in t:
         i = i.strip()
         if i != "":
            il.append(i)
      if len(il) > 0 and il[0] in CMD_DICT:
         rsp = CMD_DICT[il[0]](self._tree, il)
      else:
         rsp = (["unknown command.", "nack"])
      for line in rsp:
         self.sendLine(line)

class AaeConsoleFactory(Factory):
   def __init__(self, tree):
      self._tree = tree

   def buildProtocol(self, addr):
      p = AaeConsole(self._tree)
      p.setLineMode()
      p.delimiter = '\n'
      return p
