
import os, sys, random, signal, time, re
import subprocess as sp
import matplotlib.pyplot as plt

class MemoryPlotter:
    def __init__(self):
        self.used_mem_regex = re.compile(r'Mem:\s+(\d+)\s+(\d+)')
        self.initial_time = time.time()
        self.initial_mem = self.mem_use()
        self.times = [0]
        self.mem  = [0]

    def add_mem_mark(self):
        self.times.append(time.time() - self.initial_time)
        m = self.mem_use() - self.initial_mem
        self.mem.append(max(m, 0))

    def mem_use(self):
        s = sp.Popen(['free', '-m'], stdout=sp.PIPE).communicate()[0]
        return int(self.used_mem_regex.search(s).group(2))
   
    def plot_mem_use(self, directory, name):
        plt.plot(self.times, self.mem)
        plt.ylabel('Memory (MB)')
        plt.xlabel('Time (seconds)')
        plt.title('Memory use: ' + name)
        plt.savefig('%s/%s.png' % (directory, name))


class Test:

    QUERIES = ('select', 'insert', 'drop_table', 'create_table', 'select_join',
               'select_where')

    def __init__(self, name, index):
        self.name = name
        file_name = self.name
        file_name += ".sql" if len(sys.argv) < 3 else "_%s.sql" % index 
        self.results_dir = os.environ['MINIDB_TEST'] + '/results' 
        self.sql_file = self.results_dir + '/' + file_name
        self.cmnd = os.environ['MINIDB_ROOT'] + '/MiniDBShell '
        self.cmnd += '-q %s -e saveDB %s/%s_DB exit' % \
            (self.sql_file,self.results_dir,self.name)
        self.memory = MemoryPlotter()

    def get_files(self):
        return self.results_dir, self.sql_file

    def run_command(self, prnt=True, plot_mem=False):
        #os.environ['LIBC_FATAL_STDERR_'] = '1'
        self.cmnd += ' > tmpfile' if prnt else ' >/dev/null 2>&1'
        print self.cmnd
        p = sp.Popen(self.cmnd, shell=True)
        while p.poll() is None:
            time.sleep(0.5)
            self.memory.add_mem_mark()
        if prnt:
            with open('tmpfile', 'r') as f:
                print f.read()
            os.remove('tmpfile')
        if plot_mem:
            self.memory.plot_mem_use(self.results_dir, self.name)
        return p.returncode

    def gen_random_queries(self, db, iters, queries=QUERIES):
        for _ in xrange(iters):
            query = 'create_table' if db.empty() else random.choice(queries)
            getattr(db, query)()

