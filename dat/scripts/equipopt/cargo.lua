local cargo = {}

-- We assume standard cargo doesn't change over time
local avail_cargo = commodity.getStandard()

function cargo.add( p )
   if #avail_cargo <= 0 then
      warn(_("No standard commodities!"))
      return
   end

   -- Bias towards fewer types
   local ncargo
   local r = rnd.rnd()
   local cf = p:cargoFree()
   if r < 0.6 or cf<50 then
      ncargo = 1
   elseif r < 0.85 or cf < 200 then
      ncargo = 2
   else
      ncargo = 3
   end

   -- Add the cargo
   local q = 0
   for i=1,ncargo do
      cf = p:cargoFree()
      local amount = rnd.rnd( math.floor(0.5*cf/ncargo), math.floor(cf) )
      q = q + p:cargoAdd( avail_cargo[ rnd.rnd( 1, #avail_cargo ) ]:nameRaw(), amount )
   end
   return q
end

return cargo
