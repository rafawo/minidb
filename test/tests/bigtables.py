
# This test creates one or more tables and inserts a big number of rows.
# Args:
#     number of tables to create (0 or nothing = random)
#     number of rows to create for each table (0 or nothing = random)
#     number of cols for each table (0 or nothing = random)
#     n if you want to run n queries after creating the tables
#         0 or nothing if you don't
# All of these args are optional, if not provided => random
# Argument <= 0 means that it will be random

import sys, random
from minidb_pylib.randomdb import RandomDatabase
from minidb_pylib.test import Test

MAX_TABLES = 100
MAX_ROWS = 10000

if __name__ == '__main__':
    index, testi = filter(lambda (i, x): x[0] == 'i', enumerate(sys.argv))[0]
    test = Test('bigtables', testi[1:])
    results_dir, sql_file = test.get_files()
    rdb = RandomDatabase(sql_file)

    check = lambda i: i == index or len(sys.argv) < i+1 or sys.argv[i] == '0'
    tables = random.randint(1, MAX_TABLES) if check(1) else int(sys.argv[1])
    rows = random.randint(1, MAX_ROWS) if check(2) else int(sys.argv[2])
    cols = 0 if check(3) else int(sys.argv[3])
    iters = 0 if check(4) else int(sys.argv[4])

    for t in xrange(tables):
        rdb.create_table(cols=cols)
        for _ in xrange(rows):
            rdb.db.insert('t' + str(t + 1), None)
    if iters:
       test.gen_random_queries(rdb, iters,
            queries=('select', 'select_join', 'select_where')) 

    for t in xrange(tables):
        tname = 't' + str(t + 1)
        rdb.db.select_count(tname, tname + 'c1')

    rdb.end()

    retcode = test.run_command(plot_mem=True)

    print "%d tables generated" % tables
    cols = 'random' if cols == 0 else str(cols)
    print "Each table has %d rows and %s cols will be generated" % (rows, cols)

    if retcode != 0:
        print retcode 
        print test.cmnd
        print "failed"
    else:
        #os.remove('%s/stress_DB.zip' % results_dir)
        #os.remove(sql_file)
        print "passed"

