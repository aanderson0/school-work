create or replace function p8(newtno varchar, newvno varchar, newac varchar, newamt float) returns table(outno varchar, outvno varchar, outac varchar, outdate date, outamt float, outcname varchar, outprovince varchar, outcbalance float, outcrlim int, outvname varchar, outcity varchar, outvbalance float) as $$
	declare
		newdate date;
	begin
		SELECT into newdate CURRENT_DATE;
		INSERT INTO t
		VALUES (newtno, newvno, newac, newdate, newamt);
		update c set cbalance = cbalance - newamt where account = newac;
		update v set vbalanace = vbalanace + newamt where vno = newvno;
		return query select * from t natural join c natural join v where t.tno = newtno;
	end;
$$ language plpgsql;
