create or replace function p6() returns void as $$
	declare 
		fee float;
		curnam varchar;
		curno varchar;
		curbal float;
		c1 cursor for select v.vno, v.vname, v.Vbalanace from v;
	begin
		open c1;
		loop 
			fetch c1 into curno, curnam, curbal;
			exit when not found;
			update v set Vbalanace = Vbalanace * 0.96 where vno = curno;
			raise notice 'name: % fee: % new balance: %', curnam, curbal * 0.04, curbal * 0.96;
		end loop;
		close c1;
	end;
$$ language plpgsql;
