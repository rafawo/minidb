
import os, random, copy
import subprocess as sp


MAX_COLS = 50
# Maps a datatype to (str_name, size)
SIZEOF = os.path.join(os.path.split(os.path.abspath(__file__))[0], 'sizeof')
DATATYPES = eval(sp.Popen(SIZEOF, stdout=sp.PIPE).communicate()[0])
PNEGATE = 0.3
LOGOPS = ('||', '&&')

def random_dtval(datatype):
    if datatype == str:
        x = chr(random.randrange(32, 127))
    else:
        x = random.random() * 2147483648 * random.choice([1, -1])
    return datatype(x)


def irepr(s):
    if s == "'":
        return "'''"
    elif s == '\\':
        return "'\\'"
    return repr(s)
        

class Interface:
    # All methods assume that the table passed is
    # the table after the execution of the command that will
    # be printed

    def __init__(self, sql_file): 
        self.sql_file = open(sql_file, 'w')
    
    def print_create(self, table):
        schema = ', '.join(map(str, table.columns))
        cmnd = "create table %s (%s);\n" % (table.name, schema)
        self.write_to_sql(cmnd)

    def print_select(self, tables, columns, conditions):
        cols_str = columns if columns == '*' else ''
        if cols_str != '*':
            cols_str = list()
            for i, t in enumerate(tables):
                cols_str.append(', '.join([t.name + '.' + c.name
                                for c in columns[i]]))
            cols_str = ', '.join(cols_str)
        tables_str = ', '.join([t.name for t in tables])
        cmnd = "select %s from %s" % (cols_str, tables_str)
        if conditions:
            conditions = map(lambda c: '%s.%s %s %s.%s' % c if len(c) > 4
                             else '%s.%s %s %s' % c, conditions)
            if conditions > 1:
                cond_str = reduce(lambda c1,c2: '(%s) %s (%s)' % (c1, random.choice(LOGOPS), c2),
                                  conditions[1:], conditions[0])
            else:
                cond_str = conditions[0]
            cmnd += ' where ' + cond_str;
        self.write_to_sql(cmnd + ';\n')

    def print_insert(self, table, values):
        values_str = ', '.join([irepr(v) for v in values])
        cmnd = "insert into %s values(%s);\n" % (table.name, values_str)
        self.write_to_sql(cmnd)

    def print_drop(self, table):
        cmnd = "drop table %s;\n" % table.name        
        self.write_to_sql(cmnd)

    def close(self):
        self.sql_file.close()

    def write_to_sql(self, s):
        self.sql_file.write(s + '\n')

    def print_select_count(self, table, column):
        self.sql_file.write('select count(%s.%s) from %s;' % 
                (table.name, column, table.name))


class Column:
    def __init__(self, name, datatype=None):
        self.name = name
        if datatype:
            self.datatype = datatype
        else:
            self.datatype = random.choice(DATATYPES.keys())

    def insert(self, x=None):
        if type(x) == self.datatype:
            return
        return random_dtval(self.datatype)

    def datatype_size(self):
        return DATATYPES[self.datatype][1]
    
    def __str__(self):
        return self.name + ' ' + DATATYPES[self.datatype][0]


class Table:
    def __init__(self, name, cols_dts=None, cols=0, rows=0):
        self.name = name
        self.rows = rows
        if not cols_dts:
            cols = cols if cols > 0 else random.randint(1, MAX_COLS)
            self.columns = [Column(name + 'c' + str(i))
                            for i in xrange(cols)]
        else:
            self.columns = [Column(name + 'c' + str(i), dt)
                            for i, dt in enumerate(cols_dts)]

    def cols(self):
        return len(self.columns)

    def insert(self, values=None):
        if not values:
            values = map(lambda c: c.insert(), self.columns)
        elif not all([type(v) is self.columns[i].datatype
                      for i, v in enumerate(values)]):
            raise TypeError("Inserted values doesn't match with defined datatypes")
        self.rows += 1
        return values
        
    def select(self, cols_indexes=None):
        if cols_indexes:
            cols = [self.columns[i] for i in cols_indexes]
        else:
            cols = random.sample(self.columns, random.randint(1, len(self.columns)))
        return Table(self.name, cols, self.rows)

    def join(self, table):
        getdt = lambda cs: [c.datatype for c in cs]
        result = Table(self.name + "j" + table.name,
                       cols_dts = getdt(self.columns + table.columns),
                       rows = self.rows * table.rows)
        return result

    def column_names(self):
        return map(lambda c: c.name, self.columns)

    def has_common_dt_col(self, table):
        for col1 in self.columns:
            for col2 in table.columns:
                if col1.datatype == col2.datatype:
                    return True
        return False


class Database:

    LESS       = '<'
    GREATER    = '>'
    LESS_EQ    = '<='
    GREATER_EQ = '>='
    EQUAL      = '=='
    NEQUAL     = '!='

    RELOPS = (LESS, GREATER, LESS_EQ,
              GREATER_EQ, EQUAL, NEQUAL) 
    INTEGER    = int
    DOUBLE     = float
    CHAR       = str

    def __init__(self, sql_file):
        self.tables = dict() # maps a table to name to Table object
        self.nexti = 1
        self.interface = Interface(sql_file)

    def select(self, tables_columns=None, conditions=[]): 
        """ tables_columns is a dict() that maps table_name -> [columns]
            conditions => decide format """
        tables = [self.tables[t] for t in tables_columns]
        columns = [[self.tables[t].columns[i] for i in cs]
                    for t, cs in tables_columns.items()]
        self.interface.print_select(tables, columns, conditions)

    def insert(self, table, values):
        values = self.tables[table].insert(values)
        self.interface.print_insert(self.tables[table], values)

    def drop_table(self, table):
        self.interface.print_drop(self.tables[table])
        del self.tables[table]

    def create_table(self, cols_dts):
        "Columns is a list of column datatypes"
        tname = 't' + str(self.nexti)
        self.tables[tname] = Table(tname, cols_dts=cols_dts)
        self.nexti += 1
        self.interface.print_create(self.tables[tname]) 

    def empty(self):
        return len(self.tables) == 0

    def end(self):
        self.interface.close()

    def select_all(self, table_names):
        tables = list()
        if type(table_names) == str:
            tables.append(self.tables[table_names])
        else:
            tables = [self.tables[table] for table in table_names]
        self.interface.print_select(tables, '*', [])

    def __str__(self):
        return '\n'.join(map(str, self.tables))

    def select_count(self, table, column):
        self.interface.print_select_count(self.tables[table], column)


if __name__ == '__main__':
    d = Database('none')
    d.create_table((Database.INTEGER, Database.CHAR))
    d.create_table((Database.DOUBLE, Database.INTEGER))
    assert not d.empty()

    d.insert('t1', (1, 'a'))
    d.insert('t2', (2.0, 2))

    d.select({'t1': [0]})
    d.select({'t1': [1, 0]})
    d.select({'t2': [1]})
    d.select({'t2': [0, 1]})
    d.select({'t1': [1, 0], 't2': [1]})
    d.select_all('t1')
    d.select_all(['t1', 't2'])

    d.drop_table('t1')
    d.drop_table('t2')
    assert d.empty()

    d.end()
    with open('none', 'r') as f:
        print f.read()

