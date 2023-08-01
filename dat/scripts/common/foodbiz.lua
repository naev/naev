local foodbiz = {}

function foodbiz.michalspob ()
   local spb = var.peek("foodbiz_spob")
   if not spb then
      spb = "Zeo"
   end
   return spob.get(spb)
end

return foodbiz
