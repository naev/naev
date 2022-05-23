local fmt = require "format"

local header = {
   _("We bring you the latest news in the universe.")
}
local greeting = {
   "",
}
local articles = {
   --[[
      Science and technology
   --]]
   {
      tag = N_([[Techs Probe for Sol]]),
      desc = _([[Technicians recently returned from a new effort to understand the Sol Incident. They expect the first data to arrive in 50 decaperiods' time.]])
   },
   {
      tag = N_([[Sand Monster Caught On Tape]]),
      desc = _([[Local residents claim footage from a security camera shows elusive beast walking through a desert storm. Exobiologists suspect a hoax.]])
   },
   {
      tag = N_([[New Handheld AI Debuts]]),
      desc = _([[Braeburn, the home division of Wellington, yesterday demonstrated their new PDAI. Developer Isaac Asimov assured us the new model is guaranteed not to develop a personality of its own.]])
   },
   {
      tag = N_([[Experiment Produces Cold Fusion]]),
      desc = _([[In an interview with Bleeding Edge anchor McKenzie Kruft, a researcher at Eureka labs says he has produced a tabletop atomic reaction. He hopes to publish his results in a science journal later this cycle.]])
   },
   {
      tag = N_([[The Case for Sex]]),
      desc = _([[Though cloning rates continue to rise, Vlad Taneev believes we can still benefit from genetic recombination. "Lady chance is more creative than we," says the famous gene splicer.]])
   },
   {
      tag = N_([[Hyperspace is Hot]]),
      desc = _([[Researchers have found traces of matter moving very quickly in hyperspace. They are not yet sure whether the minute particles originated there, or are human contaminants.]])
   },
   {
      tag = N_([[A Family Freezer]]),
      desc = _([[Looking for a cryotube for your parents?  Our expert panel finds Glass Box Mark II will keep your family alive for nearly as long as the previous model.]])
   },
   --[[
      Business
   --]]
   {
      tag = N_([[Fullerton Reports Quarterly Loss]]),
      desc = _([[Engine maker Fullerton Industries has posted a c47B loss over the past 25 decaperiods. A company spokeswoman attributed the loss to the high cost of deuterium fuel and falling sales.]])
   },
   {
      tag = N_([[Genetric Board Meets]]),
      desc = _([[In the wake of the Yavi Bartolo's departure as CSO of Genetric Technologies, the board of directors will meet today. The group is expected to appoint a new chief science officer.]])
   },
   --[[
      Politics
   --]]
   --[[
      Human interest.
   --]]
   {
      tag = N_([[Eyeteeth Back in Fashion]]),
      desc = _([[Despite the advice of dentists, eyetooth caps have come into vogue again. Young people throughout the inhabited worlds use home kits to strip a layer of enamel from their teeth in favour of a binding of quartz, granite, or even flint.]])
   },
   {
      tag = N_([[Everyone Loves a SuperChimp]]),
      desc = _([[For dozens of cycles used only as menial labourers, now SuperChimps are being widely adopted as domestic companions. Enhanced primates make an affectionate, intelligent pet, or a low-cost servant.]])
   },
   {
      tag = N_([[Admiral's Ball a Triumph]]),
      desc = _([[The glamorous season drew to a close with an underwater themed ball held in air bubbles deep under the oceans of Anecu. All the season's debutantes attended.]])
   },
   {
      tag = N_([[Amazing Survival Story]]),
      desc = _([[An Xing Long was rescued after two decaperiods floating in space. "I used meditation to slow my breathing," Xing Long told us. "It was hard because I was scared."]])
   },
   {
      tag = N_([[The Best Spaceport Bars]]),
      desc = _([[Where can you get the best gargle blaster?  The famous exotic drinks list at the Doranthex Lava Room charmed our reviewer, but if you care for ambiance, don't miss the Goddard Bar.]])
   },
   {
      tag = N_([[RIP: The Floating Vagabond]]),
      desc = _([[Only two cycles after the mysterious disappearance of its owner, the galaxy's only deep space bar shut down the generators and cycled the airlock one last time.]])
   },
   {
      tag = N_([[Games for Young Pilots]]),
      desc = _([[Want your child to have a chance at a career as a space pilot?  Games like Super Julio Omniverse and SpaceFox help your child develop twitch muscles.]])
   },
   {
      tag = N_([[Former Pirate Writes Target Management Self-Help Book]]),
      desc = function () fmt.f(
         _([[A former pirate shares her story on how she steered herself away from piracy, which she wrote about in an award-winning self-help book. "I used to spend my whole life pressing {target_nearest} to target enemies, but my life changed when I had a dream about a cat munching on some grass. 'Are you using the {target_hostile} key?' it asked. 'I find that it is very useful.' I have been doing as the strange cat in my dream said ever since, and I no longer have to lose money or alienate friends. If the universe followed this simple advice, I suspect we would live in a much safer society."]]),
	 {target_nearest=naev.keyGet("target_nearest"), target_hostile=naev.keyGet("target_hostile")} ) end
   },
}

return function ()
   return "Generic", header, greeting, articles
end
