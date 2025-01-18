local head = {
   _("Pride of House O'rez"),
}
local greeting = {
   _("Latest news on the war front."),
}
local articles = {
   {
      head = N_([[More War Crimes by House O'rez]]),
      body = _([[New war crimes have been captured on footage by brave informants. Evidence is to be presented to the Imperial court to strengthen the case for the swift elimination of House O'rez.]]),
   },
   {
      head = N_([[More Fabrications and Lies by House Yetmer]]),
      body = _([[New sham campaign by House Yetmer to cover the fact they are losing the war. The traitors have made up claims regarding bloodlines, which will be soon squashed by the House O'rez elite lawyer team working hard at Jade Court.]]),
   },
}

return function ()
   return "O'rez", head, greeting, articles
end
