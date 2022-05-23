local header = {
   _("Welcome to Goddard News Centre."),
}
local greeting = {
   _("We bring you the news from around the Empire."),
}
local articles = {
   --[[
      Science and technology
   --]]
   {
      tag = N_([[Goddard: Raising the Bar]]),
      desc = _([[Many new scientists are being contracted by House Goddard to investigate possible improvements. This new strategy will increase the gap with the competing ship fabricators.]])
   },
   --[[
      Business
   --]]
   {
      tag = N_([[Goddard Earnings on the Rise]]),
      desc = _([[House Goddard has once again increased its earnings. "Our investment in technology and quality has paid off," said Kari Baker of House Goddard's marketing bureau.]])
   },
   {
      tag = N_([[Goddard Awarded Best Ship]]),
      desc = _([[Once again the Goddard Battlecruiser was awarded the Best Overall Ship prize by the Dvaered Armada's annual Ship Awards. "Very few ships have reliability like the Goddard," said Lord Warthon upon receiving the award on behalf of House Goddard.]])
   },
   {
      tag = N_([[Aerosys Earnings Drop]]),
      desc = _([[The spaceways may swarm with Hyena-model craft, but today Aerosys recorded another quarterly loss. The company is investigating the discrepancy between the popularity of the craft and its sales figures.]])
   },
   {
      tag = N_([[Aerosys Victim of Pirate Manufacturing]]),
      desc = _([[The ship manufacturer has released a study indicating its signature Hyena model is being produced by a hidden system of unlicensed manufacturers.]])
   },
   {
      tag = N_([[Melendez CEO on Strategy]]),
      desc = _([[The Chief Executive Officer of ship maker Melendez Corp. thinks manufacturers should follow his company's lead in keeping costs down and producing for the mass market.]])
   },
   {
      tag = N_([[The Goddard Exception]]),
      desc = _([[Why has a community with more expertise sailing than flying produced the Empire's elite civilian spacecraft?  Lord Warthon says the secret lies in his family's hands-on tradition of leadership.]])
   },
   {
      tag = N_([[Sneak Peek: the Kestrel]]),
      desc = _([[Our reporter took a tour through Krain's mysterious space craft. He says it poses a challenge to the Goddard.]])
   },
   --[[
      Politics
   --]]
   --[[
      Human interest.
   --]]
}

return function ()
   return "Goddard", header, greeting, articles
end
