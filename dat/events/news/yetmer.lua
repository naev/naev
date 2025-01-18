local head = {
   _("House Yetmer Reporting."),
}
local greeting = {
   _("Latest news on the war front."),
}
local articles = {
   {
      head = N_([[More War Crimes by House O'rez]]),
      body = _([[Yet more O'rez war crimes have been recorded by our brave informants. The evidence is to be presented to the Imperial court to strengthen the case for the swift elimination of House O'rez.]]),
   },
   {
      head = N_([[House Yetmer: True Successor]]),
      body = _([[Through historical blood analysis, researchers have uncovered more evidence House Yetmer is the true successor of House Yetmer-O'rez. New analyses suggest Yetmer is the dominant bloodline by at least 0.3%, and likely much more.]]),
   },
}

return function ()
   return "Yetmer", head, greeting, articles
end
