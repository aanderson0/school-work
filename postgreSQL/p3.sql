create or replace function p3(newaccount varchar, newname varchar, newprovince varchar, newcrlimit float) returns table (account varchar, cname varchar, province varchar, cbalance float, crlimit int) as $$
	begin
				INSERT INTO c
				VALUES (newaccount, newname, newprovince, 0, newcrlimit);
				return query SELECT * FROM C;
	end;
$$ language plpgsql;
