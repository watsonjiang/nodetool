#!/usr/bin/python
#import pymysql
import MySQLdb


DB_DRV=MySQLdb

class Metadata:
   def __init__(self, dbinfo, busi_name, machine_room_no):
      self._dbinfo = dbinfo
      self._busi_name = busi_name
      self._machine_room_no = int(machine_room_no)

   def get_cols(self, tname):
      conn = DB_DRV.connect(**self._dbinfo)
      cur = conn.cursor()
      cur.execute("select "
                  "column_name "
                  "from myshard_table_columns "
                  "where table_name = '%s' "
                  "and busi_name = '%s' "
                  "and machine_room_no = %d " 
                  % (tname,self._busi_name, self._machine_room_no));
      cols = []
      for r in cur:
         cols.append(r[0])
      cur.close()
      conn.close()
      return cols 

   def get_keys(self, tname):
      conn = DB_DRV.connect(**self._dbinfo)
      cur = conn.cursor()
      cur.execute("select "
                  "column_name "
                  "from myshard_table_statistics "
                  "where table_name = '%s' "
                  "and busi_name = '%s' "
                  "and machine_room_no = %d "
                  "order by seq_in_index" 
                  % (tname, self._busi_name, self._machine_room_no));
      keys = []
      for r in cur:
         keys.append(r[0])
      cur.close()
      conn.close()
      return keys
      


