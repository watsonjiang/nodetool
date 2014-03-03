import sys
import random
def data_gen(file, lines):
   f = open(file, 'w')
   for i in xrange(0, lines):
      l = "%s\t%s\n" % (random.uniform(0, 1000000000), random.uniform(0,100000000000000))
      f.write(l)
   f.close()


if __name__ == "__main__":
   data_gen(sys.argv[1], int(sys.argv[2]))
