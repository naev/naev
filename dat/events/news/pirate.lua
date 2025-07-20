local head = {
   _("Pirate News Network."),
}
local greeting = {
   _("News that matters."),
   _("Adopt a cat today!"),
   _("Laughing at the Emperor."),
   _("On top of the world."),
   _("Piracy has never been better."),
   _("Yo ho, a pirate's news for you!"),
}
local articles = {
   --[[
      Science and technology
   --]]
   {
      head = N_([[Skull and Bones Improving]]),
      body = _([[The technology behind Skull and Bones is advancing. Not only do they steal ships, but they improve on the original design. "This gives us pirates an edge against the injustice of the Empire," says Millicent Felecia Black, lead Skull and Bones engineer.]])
   },
   {
      test = function () return diff.isApplied("energy_harpoon") end,
      head = N_([[New Breakthrough in Weapon Technology]]),
      body = _([[Lord Maikki has announced the development of a new line of weapons dubbed "Energy Harpoon", available at New Haven. Weapons connoisseurs flock ]]),
   },
   --[[
      Business
   --]]
   {
      head = N_([[Draconis Favourite Plundering Space]]),
      body = _([[Draconis has recently passed Delta Pavonis in the pirate polls as the most favoured plundering space. The abundance of traders and high interference make it an excellent place to get some fast credits.]])
   },
   {
      head = N_([[New Ships for Skull and Bones]]),
      body = _([[The Skull and Bones was able to extract a few dozen high quality vessels from Caladan warehouses under the nose of the Empire. These ships will help keep production high and booming.]])
   },
   --[[
      Politics
   --]]
   {
      head = N_([[Emperor Weaker Than Ever]]),
      body = _([[Recent actions demonstrate the inefficiency and weakness of the Emperor. One of the last irrational decisions left Eridani without a defence fleet to protect the traders. It's a great time to be a pirate.]])
   },
   {
      head = N_([[Pretender or Contender?]]),
      body = _([[Our sources report there's a child some consider to be the heir to the Emperor's throne. Daughter of an alleged concubine, she was said to be captured by captain Addom Rojo, who learned of her special status and suggests using her as leverage against the Empire.]])
   },
   {
      head = N_([[Parley and Rejoice]]),
      body = _([[A long-standing enmity between two captains, Vasserdrach and Mallard, has come to an end, after a long negotiation arbitrated by retired captain Nusbaum. The two had reached an agreement following a three-year feud over the spoils of a relatively large imperial ship.]])
   },
   --[[
      Human interest.
   --]]
   {
      head = N_([[Cats in New Haven]]),
      body = _([[An explosion in the cat population of New Haven has created an adoption campaign with the slogan, "Pirate Cats, for those lonely space trips.". Is your space vessel full of vermin? Adopt a cat today!]])
   },
   {
      head = N_([[Rumours and Whispers]]),
      body = _([[A large Za'lek derelict ship is rumoured to have been found. Several maps have been circulated among the most prominent treasure seekers, but none has yet to be verified.]])
   },
   {
      head = N_([[Calmest Limerick Competition in Cycles]]),
      body = _([[Onlookers left disappointed as the Galactic Pirate Limerick Competition ended without a massive brawl for first time in history. "Does nobody respect tradition any more?" said one angry spectator. Organizers blame Pirate Lord Pizarros birthday party a few periods before for drying out all the reservoirs of Rum and giving most participants a deca-period long hangover.]]),
   },
   {
      head = N_([[Ghost in the Shell?]]),
      body = _([[Haunted peanuts may be an Imperial psyops operation to destabilize the Clans says Dreamer Clans spokespirate. Recommends people to stay away from unknown drugs coming in from Sirius space.]]),
   },
}

return function ()
   return "Pirate", head, greeting, articles
end
