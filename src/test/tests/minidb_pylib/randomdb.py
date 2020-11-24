
import random
from db import Database, irepr, random_dtval

MAXCONDITIONS = 20


class RandomDatabase:
    def __init__(self, sql_file):
        self.db = Database(sql_file)
 
    def select(self, n=1, has_where=False):
        conditions = list()
        if n != 1:
            n = random.randint(2, n)
        tables = random.sample(self.db.tables.items(), n)
        table_objs = [t for _, t in tables]
        columns = [t.select().columns for _, t in tables]
        if has_where:
            table_pairs = [(t1, t2) for t1 in table_objs for t2 in table_objs
                           if t1.has_common_dt_col(t2)]
            for _ in xrange(random.randint(1, MAXCONDITIONS)):
                op = random.choice(Database.RELOPS)
                if random.random() < 0.5: # Form: (col rel col)
                    t1, t2 = random.choice(table_pairs)
                    c1, c2 = random.choice([(c1, c2)
                                 for c1 in t1.columns for c2 in t2.columns
                                 if c1.datatype == c2.datatype])
                    conditions.append((t1.name, c1.name, op, t2.name, c2.name))
                else: # Form: (col rel val)
                    t = random.choice(table_objs)
                    c = random.choice([c for c in t.columns])
                    conditions.append((t.name, c.name, op, irepr(random_dtval(c.datatype))))
        self.db.interface.print_select(table_objs, columns, conditions) 

    def select_join(self):
        self.select(len(self.db.tables))

    def select_where(self):
        self.select(has_where=True)

    def select_join_where(self):
        self.select(len(self.db.tables), True)

    def insert(self):
        table = random.choice(self.db.tables.keys())
        self.db.insert(table, None)

    def drop_table(self):
        table = random.choice(self.db.tables.keys())
        self.db.drop_table(table)

    def create_table(self, cols=0):
        self.db.create_table(None)

    def empty(self):
        return self.db.empty()

    def end(self):
        self.db.end()

    def __str__(self):
        return str(self.db)

    def select_all(self, table):
        self.db.select_all(table)

