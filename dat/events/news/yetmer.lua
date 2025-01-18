local head = {
   _("House Yetmer Reporting."),
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
      head = N_([[House Yetmer: True Successor]]),
      body = _([[Researchers have uncovered more evidence through historical blood analysis that House Yetmer is the true successor of House Yetmer-O'rez. New analysis suggest that Yetmer is the dominant bloodine by at least 0.3%, and is likely much higher.]]),
   },
}

return function ()
   return "Yetmer", head, greeting, articles
end
