
create table t1 (a int, b char, c double, d int, e char, f double);
create table t2 (a int, b char, c double, d int, e char, f double);
create table t3 (a int, b char, c double, d int, e char, f double);

insert into t1 values( 1, 'a',  1.0,  2, 'b',  2.0);
insert into t1 values( 2, 'b',  2.0,  2, 'b',  2.0);
insert into t1 values( 3, 'c',  3.0,  4, 'd',  4.0);
insert into t1 values( 4, 'd',  4.0,  4, 'd',  4.0);
insert into t2 values( 5, 'e',  5.0,  6, 'f',  6.0);
insert into t2 values( 6, 'f',  6.0,  6, 'f',  6.0);
insert into t2 values( 7, 'g',  7.0,  8, 'h',  8.0);
insert into t2 values( 8, 'h',  8.0,  8, 'h',  8.0);
insert into t3 values( 9, 'i',  9.0, 10, 'j', 10.0);
insert into t3 values(10, 'j', 10.0, 10, 'j', 10.0);
insert into t3 values(11, 'k', 11.0, 12, 'l', 12.0);
insert into t3 values(12, 'l', 12.0, 12, 'l', 12.0);

select t1.a, count(t1.a) from t1 group by t1.a;
select t1.b, count(t1.b) from t1 group by t1.b;
select t1.c, count(t1.c) from t1 group by t1.c;
select t1.d, count(t1.a) from t1 group by t1.d;
select t1.e, count(t1.a) from t1 group by t1.e;
select t1.f, count(t1.a) from t1 group by t1.f;

select t1.d, t1.e, t1.f, count(t1.d) from t1 group by t1.d, t1.e, t1.f;
select t1.d, t1.e, t1.f, count(t1.d) from t1 group by t1.f, t1.d, t1.e;
select t1.d, t1.e, t1.f, count(t1.d) from t1 group by t1.e, t1.f, t1.d;

select * from t1, t2, t3;

select t1.d, t2.e, t3.f, count(t1.a), count(t2.b), count(t3.c) from t1, t2, t3 group by t1.d, t2.e, t3.f;
select t1.d, t2.e, t3.f, count(t1.d) from t1, t2, t3 group by t1.d, t2.e, t3.f;

