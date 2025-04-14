local head = {
   _("The word of the Free Resistance."),
}
local greeting = {
   _("Independent news from the independent frontier."),
}
local articles = {
   --[[
      Science and technology
   --]]
   {
      head = N_([[Potential Upper Hand]]),
      body = _([[The oppressors should count their days! FLF scientist released a new paper stating more than a few advancements in jamming technology, which could potentially help our brave freedom fighters on their righteous operations against the Red Menace.]])
   },
   --[[
      Business
   --]]
   {
      head = N_([[Cookie Cutter]]),
      body = _([[With the goal of raising funds for the cause, young recruits began selling cookies in the Frontier colonies. Some have even been roughly modeled on Resistance heroes and leaders.]])
   },
   --[[
      Politics
   --]]
   {
      head = N_([[Boycott, Divestment, Sanctions]]),
      body = _([[In an attempt to further our righteous cause, Frontiersmen under the Dvaered's boot have started a movement to boycott and sanction Dvaered political and social leaders wherever they're invited to speak and attend around the Empire.]])
   },
   --[[
      Human interest.
   --]]
}

return function ()
   return "FLF", head, greeting, articles
end
