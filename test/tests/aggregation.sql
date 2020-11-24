
create table t1(a int, b float, c double, d char, key int);
create table t2(a int, b float, c double, d char, key int);
create table t3(a int, b float, c double, d char, key int);

insert into t1 values(1, 1.0, 1.0, 'a', 1);  
insert into t1 values(2, 2.0, 2.0, 'b', 1);  
insert into t1 values(3, 3.0, 3.0, 'c', 1);  

insert into t2 values(4, 4.0, 4.0, 'd', 2);  
insert into t2 values(5, 5.0, 5.0, 'e', 2);  
insert into t2 values(6, 6.0, 6.0, 'f', 2);  
  
insert into t3 values(7, 7.0, 7.0, 'g', 3);  
insert into t3 values(8, 8.0, 8.0, 'h', 3);  
insert into t3 values(9, 9.0, 9.0, 'i', 3);

select sum(t1.a), sum(t1.b), sum(t1.c), sum(t1.d) from t1 group by t1.key;
select sum(t1.a), sum(t1.b), sum(t1.c), sum(t1.d)  from t1, t2, t3 group by t1.key, t2.key, t3.key;

select avg(t1.a), avg(t1.b), avg(t1.c), avg(t1.d) from t1 group by t1.key;
select avg(t1.a), avg(t1.b), avg(t1.c), avg(t1.d) from t1, t2, t3 group by t1.key, t2.key, t3.key;

select max(t1.a), max(t1.b), max(t1.c), max(t1.d) from t1 group by t1.key;
select max(t1.a), max(t1.b), max(t1.c), max(t1.d) from t1, t2, t3 group by t1.key, t2.key, t3.key;

select min(t1.a), min(t1.b), min(t1.c), min(t1.d) from t1 group by t1.key;
select min(t1.a), min(t1.b), min(t1.c), min(t1.d) from t1, t2, t3 group by t1.key, t2.key, t3.key;

select count(t1.a), count(t1.b), count(t1.c), count(t1.d) from t1 group by t1.key;
select count(t1.a), count(t1.b), count(t1.c), count(t1.d) from t1, t2, t3 group by t1.key, t2.key, t3.key;
