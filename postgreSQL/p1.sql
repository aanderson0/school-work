create or replace function p1(inputname varchar) returns table (Vname varchar, t_date date, amount float) as $$
	declare 
		targetname varchar = inputname;
	begin
		return query 	select v.vname, t.t_date, t.amount 
				from c, t, v 
				where c.cname = targetname and c.account = t.account and t.vno = v.vno; 
	end;
$$ language plpgsql;
