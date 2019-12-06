create or replace function p7() returns void as $$
	declare 
		charge float;
		curnam varchar;
		curlim varchar;
		curbal float;
		curac varchar;
		c1 cursor for select c.account, c.cname, c.Cbalance, c.Crlimit from c;
	begin
		open c1;
		loop 
			fetch c1 into curac, curnam, curbal, curlim;
			exit when not found;
			charge = curlim::float - curbal;
			if(charge > 0) then
				charge = charge * 0.1;
				update c set cbalance = curbal + charge where account = curac;
				raise notice 'name: % new balance: %', curnam, curbal + charge;
			end if;
		end loop;
		close c1;
	end;
$$ language plpgsql;
