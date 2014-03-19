#!/usr/bin/python

import xml.etree.ElementTree as ET

class Config:
   def __init__(self, xmlfile):
      self._xmlfile = xmlfile

   def get_console_info(self):
      info = []
      tree = ET.parse(self._xmlfile);
      root = tree.getroot()
      info.append(root.find('./console/ip').text)
      info.append(int(root.find('./console/port').text))
      return info 
   
   def get_metadb_info(self):
      info = {}
      tree = ET.parse(self._xmlfile)
      root = tree.getroot()
      info['host'] = root.find('./metadata-db/ip').text
      info['port'] = int(root.find('./metadata-db/port').text)
      info['user'] = root.find('./metadata-db/user').text
      info['db'] = root.find('./metadata-db/db').text
      info['passwd'] = root.find('./metadata-db/pass').text
      return info

   def get_machine_room_no(self):
      tree = ET.parse(self._xmlfile)
      root = tree.getroot()
      return root.find('./group').text
   
   def get_busi_name(self):
      tree = ET.parse(self._xmlfile)
      root = tree.getroot()
      return root.find('./business').attrib['alias']

   def get_mysharddb_info(self):
      info = {}
      tree = ET.parse(self._xmlfile)
      root = tree.getroot()
      info['host'] = root.find('./myshard-db/ip').text
      info['port'] = int(root.find('./myshard-db/port').text)
      info['user'] = root.find('./myshard-db/user').text
      info['db'] = root.find('./myshard-db/db').text
      info['passwd'] = root.find('./myshard-db/pass').text
      return info



