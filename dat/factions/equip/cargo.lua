local cargo = {}

function cargo.add( p )
   local avail_cargo = commodity.getStandard()
   if #avail_cargo <= 0 then
      return
   end

   -- Bias towards fewer types
   local ncargo
   local r = rnd.rnd()
   local cf = p:cargoFree()
   if r < 0.6 or cf<30 then
      ncargo = 1
   elseif r < 0.85 or cf < 60 then
      ncargo = 2
   else
      ncargo = 3
   end

   -- Add the cargo
   local q = 0
   for i=1,ncargo do
      local amount = rnd.rnd( 0, math.floor(p:cargoFree()) )
      q = q + p:cargoAdd( avail_cargo[ rnd.rnd( 1, #avail_cargo ) ]:nameRaw(), amount )
   end
   return q
end


return cargo
