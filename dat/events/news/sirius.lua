local head = {
   _("Sirius News Reel. Words of the Sirichana for all."),
}
local greeting = {
   _("Stay faithful."),
   _("Sirichana watches and guides you."),
}
local articles = {
   --[[
      Science and technology
   --]]
   --[[
      Business
   --]]
   {
      head = N_([[Trade Meeting at Lorelei]]),
      body = _([[Lorelei, in the Churchill system, is the latest world to host major trade negotiations between the Fyrra and the Space Traders Society. The Fyrra Arch-Canter has indicated that opening up trade routes is a major goal.]])
   },
   --[[
      Politics
   --]]
   {
      head = N_([[Dvaered Extorting Pilgrims]]),
      body = _([[Recent pilgrims headed to Mutris have been telling stories of extortion and violations by Dvaered operators. Dvaered Warlord Kra'tok claims that these are "figments of the touched's imagination". Official complaints have been made to the Emperor.]])
   },
   --[[
      Human interest.
   --]]
   {
      head = N_([[Words of Tranquility]]),
      body = _([[We welcome many new Touched who have recently begun ministering to the Shaira echelon after their long pilgrimage on Mutris. House Sirius is still a refugee for the orphans lost in this Universe.]])
   },
}

return function ()
   return "Sirius", head, greeting, articles
end
