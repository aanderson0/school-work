create or replace function p2(inputname varchar) returns table (cname varchar, account varchar, province varchar) as $$
	declare 
		targetname varchar = inputname;
	begin
		return query 	select distinct c.cname, c.account, c.province 
				from c, t, v 
				where v.vname = targetname and c.account = t.account and t.vno = v.vno; 
	end;
$$ language plpgsql;
