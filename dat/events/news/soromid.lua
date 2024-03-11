local head = {
   _("The voice of the future."),
}
local greeting = {
   _("Genetically tailoured for you."),
   _("Naturally selected news."),
}
local articles = {

   --[[
      Science and technology
   --]]
   {
      head = N_([[On the Usefulness of Tails]]),
      body = _([[A paper published by the journal Unnatural Observations on the usefulness of tails caused a great debate among the proponents and opponents of heavy planet colonization endeavours. Moderator quoted as saying "we can't seem to reach an agreement. I'm splitting my tails!"]]),
   },
   {
      head = N_([[Unusual Recessiveness]]),
      body = _([[Several dark planets report the natural re-emergence of daylight eyes, prompting leaders to declare planetary emergency.]]),
   },
   {
      head = N_([[Colourful Outlook on Life]]),
      body = _([[New eye colour serum released for general consumption. The new colour changes, and can appear to be either blue, green, purple or yellow, depending on the direction it is viewed from. Renowned socialite Sharmante Givotake has already signed a 6-month deal to promote the new colour.]]),
   },
   --[[
      Business
   --]]
   {
      head = N_([[Fishy Business]]),
      body = _([[Fishing mogul arrested on account of genetically modifying her corporation's fish and selling them to Empire museums as evidence of alien life. The scheme allegedly netted her a considerable sum, sources say.]]),
   },
   --[[
      Politics
   --]]
   {
      head = N_([[Make Sorom Great Once More]]),
      body = _([[A faction of Soromids has risen to prominence, requesting a "return to ancient ways" and to "bring back the glory days" of before the incident. Their followers even consist of a few fringe politicians.]]),
   },
   {
      head = N_([[A Voice for Unionization]]),
      body = _([[Several Soromid delivery services report a large movement for unionization among their workers. Some intelligent bioengineered pets are rumoured to have already signed up as well. "What's next," laments a manager who requested anonymity, "do they want to unionize my bioship as well?"]]),
   },
   {
      head = N_([[Shameful display]]),
      body = _([[Sorom Remembrance Day ceremony marred by incident of public drunkenness and overall inconsiderate displays. Several delinquents arrested.]]),
   },
   --[[
      Human interest.
   --]]
   {
      head = N_([[The Circus of Ancient Visage]]),
      body = _([[A roaming circus trails all over Soromid space, showing such magnificent and bizarre performers, like the 160 cm man, the two legged woman, and the man who cannot fly. Visitors flock to this primitive display, basking in our ancestors' fragility and weird biology.]]),
   },
   {
      head = N_([[Beauty in the Third Eye of the Beholder]]),
      body = _([[The latest pan-Soromid beauty pageant seen controversy, as judges elected professional tentacle model, Virsia Darami, as the winner. Her amateur opponents requested a recount, stating unearned advantages and last minute genemodding as causes to reject her application.]]),
   },
}

return function ()
   return "Soromid", head, greeting, articles
end
