local minerva = {}

-- Roughly 1 token is 1000 credits
function minerva.tokens_get()
   return var.peek( "minerva_tokens" ) or 0
end
function minerva.tokens_get_gained()
   return var.peek( "minerva_tokens_gained" ) or 0
end
function minerva.tokens_pay( amount )
   local v = minerva.tokens_get()
   var.push( "minerva_tokens", v+amount )
   -- Store lifetime earnings
   if amount > 0 then
      v = var.peek( "minerva_tokens_gained" ) or 0
      var.push( "minerva_tokens_gained", v+amount )
   end
end

return minerva
