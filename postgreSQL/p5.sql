create or replace function p5() returns table (vn varchar, vnam varchar, cit varchar, bal float) as $$
	declare 
		transcount int;
		curno varchar;
		curbal float;
		c1 cursor for select count(t.vno), v.vno, Vbalanace from v LEFT JOIN t ON v.vno = t.vno group by v.vno, v.Vbalanace;
	begin
		open c1;
		loop 
			fetch c1 into transcount, curno, curbal;
			exit when not found;
			update v set Vbalanace = transcount + curbal where vno = curno;
		end loop;
		close c1;
		return query SELECT * FROM V order by vno;
	end;
$$ language plpgsql;
