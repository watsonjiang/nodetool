import shutil
import MySQLdb

DB_DRV = MySQLdb

class Datadumper:
   def __init__(self, dbinfo):
      self._dbinfo = dbinfo

   def dump_table_to_file(self, tname, cols, dst_folder):
      conn = DB_DRV.connect(**self._dbinfo)
      cur = conn.cursor()
      sql = ("select " + ",".join(cols) +
            " into outfile '" + dst_folder +
            "' from " + tname)
      cur.execute(sql)
