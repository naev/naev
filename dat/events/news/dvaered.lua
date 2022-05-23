local head = {
   _("Welcome to the Dvaered News Centre."),
}
local greeting = {
   _("Short and to the point news."),
   _("All that happens. In simple words. So you can understand."),
   _("Simple news for busy people."),
}
local articles = {
   --[[
      Science and technology
   --]]
   {
      head = N_([[New Mace Rockets]]),
      body = _([[Dvaered Engineers are proud to present the new improved version of the Dvaered Mace rocket. "We have proven the new rocket to be nearly twice as destructive as the previous versions," says Chief Dvaered Engineer Nordstrom.]])
   },
   --[[
      Business
   --]]
   --[[
      Politics
   --]]
   {
      head = N_([[FLF Responsible for Piracy]]),
      body = _([[Law enforcement expert Paet Dohmer's upcoming essay describes the group as "more criminal gang than independence movement", according to his publicist.]])
   },
   {
      head = N_([[Front Responsible for Shipping Woes]]),
      body = _([[A spokeswoman for the separatist group says they were behind the recent series of attacks on cargo ships operating between Dakron and Theras. Dvaered officials condemned the actions.]])
   },
   {
      head = N_([[Jouvanin Tapped as Interim Chief]]),
      body = _([[Following the arrest of Rex Helmer, former Anecu deputy governor Elene Jouvanin will be sworn in today. She will serve out the term as governor.]])
   },
   {
      head = N_([[FLF Terrorist Trial Ends]]),
      body = _([[FLF Terrorist Trial ended this cycle with an unsurprising death sentence for all five members of the Nor spaceport bombing. Execution is scheduled in 10 periods.]])
   },
   {
      head = N_([[New Challenges for New Times]]),
      body = _([[The Dvaered council after a unanimous ruling decided to increase patrols in Dvaered space due to the recent uprising in FLF terrorism. The new measure is expected to start within the next cycle.]])
   },
   --[[
      Human interest.
   --]]
   {
      head = N_([[Sirius Weaker Than Ever]]),
      body = _([[This cycle breaks the negative record for fewest pilgrims to Mutris since the formation of House Sirius. This weakness is yet another sign that House Dvaered must increase patrols on the border and into Sirius space.]])
   }
}

return function ()
   return "Dvaered", head, greeting, articles
end
