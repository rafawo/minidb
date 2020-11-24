
import subprocess as sp

cmnd = "/home/miguel/Desktop/MiniDB/MiniDBShell -q /home/miguel/Desktop/MiniDB/test/results/stress.sql -e saveDB /home/miguel/Desktop/MiniDB/test/results/stress_DB exit > myout"

p = sp.Popen(cmnd, shell=True)
while p.poll() is None:
    pass
print p.communicate()[1], p.poll()
