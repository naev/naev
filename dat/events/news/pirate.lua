local head = {
   _("Pirate News Network."),
}
local greeting = {
   _("News that matters."),
   _("Adopt a cat today!"),
   _("Laughing at the Emperor."),
   _("On top of the world."),
   _("Piracy has never been better."),
}
local articles = {
   --[[
      Science and technology
   --]]
   {
      head = N_([[Skull and Bones Improving]]),
      body = _([[The technology behind Skull and Bones is advancing. Not only do they steal ships, but they improve on the original design. "This gives us pirates an edge against the injustice of the Empire," says Millicent Felecia Black, lead Skull and Bones engineer.]])
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
   --[[
      Human interest.
   --]]
   {
      head = N_([[Cats in New Haven]]),
      body = _([[An explosion in the cat population of New Haven has created an adoption campaign with the slogan, "Pirate Cats, for those lonely space trips.". Is your space vessel full of vermin? Adopt a cat today!]])
   },
}

return function ()
   return "Pirate", head, greeting, articles
end
