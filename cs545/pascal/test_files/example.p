program example(output);
var x, y, z :  integer;

function testadd(arg1, arg2 : integer ): integer;
var var1 : integer;
begin
   var1 := 5;
   testadd := arg1 + arg2
end;

begin
   x := (5 + testadd(7 - 3, 3 * 5) + 6) * 2;
   write(x)
end.