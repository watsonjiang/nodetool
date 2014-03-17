#!/usr/bin/python

# a module maintain filter list.
# we need to unify the data format before input it to hashtree.
# filter is a map which maps the column to it's data convert func.

from pyaae import _Filterlist
import pymysql

class FilterList:
   def __init__(self, dbinfo):
      self._filterlist = _Filterlist()
      self._data = {}
      self._dbinfo = dbinfo

   def update(self):
      conn = pymysql.connect(**self._dbinfo)
      cur = conn.cursor()
      cur.execute("select "
                  "table_name, column_name, data_type "
                  "from myshard_table_columns")
      all_row_keys = []
      for r in cur:
         tname = r[0]
         colname = r[1]
         all_row_keys.append([tname, colname])
         datatype = r[2]
         func_id = 0
         if datatype.lower() == "datatime":
            func_id = 1
         elif datatype.lower() == "bigint":
            func_id = 1
         elif datatype.lower() == "integer":
            func_id = 1
         if func_id != 0:
            tmp_key = "|".join([tname, colname])
            if tmp_key in self._data:
               if _data[tmp_key] != func_id:
                  self._filterlist.add(tname, colname, func_id)
                  _data[tmp_key] = func_id
      cur.close()
      conn.close()
      a = set(["|".join([i,j]) for i,j in all_row_keys])
      b = set([i for i in self._data])
      c = b.difference(a) 
      for i in c:
          tname, colname = i.split("|") 
          self._filterlist.remove(tname, colname)
          del self._data[tmp_key]

   def get_list(self):
      return self._data

