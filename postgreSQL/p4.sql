create or replace function p4() returns void as $$
	declare 
		acno varchar;
		cnam varchar;
		tamnt float;
		curtamnt float;
		vnam varchar;
		curvnam varchar;
		numtrans int;
		curdate date;
		maxdate date;
		c1 cursor for select c.cname, c.account from c order by account;
		c2 cursor for select count(t.account) from c LEFT JOIN t ON c.account = t.account group by c.account;
		t1 cursor for select t.t_date, t.amount, v.vname from t, v where t.account = acno and t.vno = v.vno;
	begin
		open c1;
		open c2;
		loop 
			fetch c1 into cnam, acno;
			exit when not found;
			fetch c2 into numtrans;
			exit when not found;
			if(numtrans > 0) then
				open t1;
				maxdate = '0001-01-01';
				loop
					fetch t1 into curdate, curtamnt, curvnam;
					exit when not found;
					if(curdate > maxdate) then
						maxdate = curdate;
						tamnt = curtamnt;
						vnam = curvnam;
					end if;
				end loop;
				close t1;
				raise notice '% % % %', acno, cnam, tamnt, vnam;
			else
				raise notice '% %: no transaction', acno, cnam;
			end if;
		end loop;
		close c1;
		close c2;
	end;
$$ language plpgsql;
