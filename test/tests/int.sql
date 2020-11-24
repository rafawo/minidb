create table table1(a int, b int);

insert into table1 values(0, 0);
insert into table1 values(-1024, 1024);
insert into table1 values(-1048576, 1048576);
insert into table1 values(-2147483647, 2147483647);


insert into table1 values(-2147483648, 2147483648);
insert into table1 values(-4294967295, 4294967295);

select * from table1;
