local head = {
   _("Space Traders Society News Network"),
}
local greeting = {
   _("Traders of the galaxy, Unite!"),
}
local articles = {
	--[[
      Business
   --]]
   {
      head = N_([[Taxation is theft!]]),
      body = _([[New trade-related taxes suggested to the imperial court astounded local traders. Our sources state that the bill "came out of nowhere" and seems to be supported by all aisles of the imperial legislative branch.]]),
   },
   {
      head = N_([[Nature of exploitation]]),
      body = _([[New venture capital firm, Natural Green Nature Inc. received full mining rights for Elysian natural reserve, ignoring the tree-hugging leaf-eating environmentalist protests plaguing the operation for the last few months. Scientists say the mineral-rich soil will provide a lucrative export for the surrounding towns, and for those quick enough to stake claim on the trade involved.]]),
   },
}

return function ()
   return "Traders Society", head, greeting, articles
end
