program ifthen(output);
var x, y, z :  integer;
begin
   x := 5;
   y := 3;
   z := 10;
      if (1 < x) then
         if (3 = y) then z := 1 else z := 2;
      write(z)
end.