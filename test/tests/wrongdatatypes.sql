create table table1(a char, b int, c double, d float);

insert into table1 values('a', 3.0, 1, 22.2423f);
insert into table1 values('b', 3.1, 3.1, 3.1f);
insert into table1 values('c', 4, 5, 6);
insert into table1 values('d', 7, 8, 9f);

insert into table1 values(1, 2, 3.0, 4f);
insert into table1 values('f', 'a', 1, 2f);
insert into table1 values('g', 1, 'b', 3f);
insert into table1 values('h', 1, 2.0, 'c');

select * from table1;
