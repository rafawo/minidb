
create table t1(a int);
create table t2(b int);
create table t3(c int);

select * from t4;

select t1.b from t1;
select t2.c from t2;
select t3.d from t3;

select t1.a, t2.c from t1,t2;
select t3.a from t1, t2;

