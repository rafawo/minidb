
import shutil, sys, random
from minidb_pylib.randomdb import RandomDatabase
from minidb_pylib.test import Test

if __name__ == '__main__':
    i = 1 if len(sys.argv) == 2 else 2
    index = int(sys.argv[i][1:])
    test = Test('stress', index)
    results_dir, sql_file = test.get_files()


    iters = int(sys.argv[1]) if len(sys.argv) > 2 else random.randint(100, 2000)
    db = RandomDatabase(sql_file)
    test.gen_random_queries(db, iters)
    db.end()

    retcode = test.run_command()

    if retcode:
        print retcode
        print test.cmnd
        print "failed"
    else:
        #os.remove('%s/stress_DB.zip' % results_dir)
        #os.remove(sql_file)
        print "passed"

