
create table t(a float);

insert into t values(0.0f);
insert into t values(0.000001f);
insert into t values(-0.000001f);
insert into t values(123456.125f);
insert into t values(-123456.125f);

insert into t values(340282346638528859811704183484516925440.000000f);
insert into t values(-340282346638528859811704183484516925440.000000f);

select * from t;
