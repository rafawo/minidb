
create table t1 (a int, b char, c double, d int, e char, f double);
create table t2 (a int, b char, c double, d int, e char, f double);

insert into t1 values( 1, 'a',  1.0,  2, 'b',  2.0);
insert into t1 values( 2, 'b',  2.0,  2, 'b',  2.0);
insert into t1 values( 3, 'c',  3.0,  3, 'c',  4.0);
insert into t1 values( 4, 'd',  4.0,  3, 'c',  4.0);
insert into t2 values( 5, 'e',  5.0,  4, 'd',  6.0);
insert into t2 values( 6, 'f',  6.0,  4, 'd',  6.0);
insert into t2 values( 7, 'g',  7.0,  5, 'e',  8.0);
insert into t2 values( 8, 'h',  8.0,  5, 'e',  8.0);

select * from t1 where t1.a < 3 && t1.b < 'b';
select * from t1 where ((t1.a > 0) && (t1.d > 2)) || t1.c == 1.0;
select * from t1 where ((t1.a < 3) || (t1.d > 2)) && ((t1.e == 'b') || (t1.f != 4.0));
select * from t1 where t1.a == t1.d;

select * from t1, t2;

select * from t1, t2 where t1.a == t2.d;
select * from t1, t2 where ((t1.a < t2.d) && (t1.d == t1.a)) || t1.b == t2.e;
select * from t1, t2 where ((t1.f > t1.c) || (t1.b == t2.e)) && ((t1.a == 1) || (t2.a == 4));
select * from t1, t2 where t1.b < t2.e && t2.e > 'd';

