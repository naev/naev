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
      tag = N_([[On the Usefullness of Tails]]),
      desc = _([[A paper published by the journal Unatural Observations on the usefulness of tails caused a great debate among the proponents and opponents of heavy planet colonization endeavors. Moderator quoted as saying "we can't seem to reach an agreement. I'm splitting my tails!"]]),
   },
   {
      tag = N_([[Unusual recessivness]]),
      desc = _([[Several dark planets report the natural re-emergence of daylight eyes, prompting leaders to declare planetary emergency.]]),
   },
   --[[
      Business
   --]]
   {
      tag = N_([[Fishy Business]]),
      desc = _([[Fishing mogul arrested on account of genetically modifying her corporation's fish and selling them to Empire museums as evidence of alien life. The scheme allegedly netted her a considerable sum, sources say.]]),
   },
   --[[
      Politics
   --]]
   {
      tag = N_([[Make Sorom Great Once More]]),
      desc = _([[A faction of Soromids has risen to prominence, requesting a "return to ancient ways" and to "bring back the glory days" of before the incident. Their followers even consist of a few fringe politicians.]]),
   },
   {
      tag = N_([[A Voice for Unionization]]),
      desc = _([[Several Soromid delivery services report a large movement for unionization among their workers. Some living ships are rumoured to have already signed up as well. "What's next," laments a manager who requested anonymity, "do they want to unionize my pet as well?"]]),
   },
   --[[
      Human interest.
   --]]
   {
      tag = N_([[The Circus of Ancient Visage]]),
      desc = _([[A roaming circus trails all over Soromid space, showing such magnificent and bizarre performers, like the 160cm man, the two legged woman, and the man who cannot fly. Visitors flock to this primitive display, basking in our ancestors' fragility and weird biology.]]),
   },
   {
      tag = N_([[Beauty in the Third Eye of the Beholder]]),
      desc = _([[The latest pan-Soromid beauty pageant seen controversy, as judges elected professional tentacle model, Virsia Darami, as the winner. Her amateur opponents requested a recount, stating unearned advantages and last minute genemodding as causes to reject her application.]]),
   },
}

return function ()
   return "Soromid", head, greeting, articles
end
