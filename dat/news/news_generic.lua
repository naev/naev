--[[

   Random Generic News

--]]


function news_greetGeneric ()
   local greet

   -- Set the greeting
   greet = "Welcome to Universal News Feed. All the headlines all the time."

   return greet
end


function news_addGeneric( rawtable )
   gtable = {
      --[[
         Science and technology
      --]]
      {
         title = "Techs Probe for Sol",
         desc = "Technicians at the Omega Station recently returned from a new effort to understand the Sol Incident. They expect first data to arrive in two months' time."
      },
      {
         title = "Sand Monster Caught On Tape",
         desc = "Local residents claim footage from an Arrakis security camera shows elusive beast walking through a desert storm. Exobiologists suspect a hoax."
      },
      {
         title = "New Handheld AI Debuts",
         desc = "Braeburn, the home division of Wellington, yesterday demonstrated their new PDAI. Developer Isaac Asimov assured us the new model is guaranteed not to develop a personality of its own."
      },
      {
         title = "Experiment Produces Cold Fusion",
         desc = "In an interview with Bleeding Edge anchor McKenzie Kruft, a researcher at Eureka labs says he has produced a tabletop atomic reaction. He hopes to publish his results in a science journal later this year."
      },
      {
         title = "Bees Introduced to Emperor's Fist",
         desc = "As they prepare the gardens of the future imperial compound, entomologists have established the first colony of the Earth insects on the planet formerly known as G Scorpeii 5."
      },
      {
         title = "The Case for Sex",
         desc = "Though cloning rates continue to rise, Vlad Taneev believes we can still benefit from genetic recombination. \"Lady chance is more creative than we,\" says the famous gene splicer."
      },
      {
         title = "Hyperspace is Hot",
         desc = "Researchers have found traces of matter moving very quickly in hyperspace. They are not yet sure whether the minute particles originated there, or are human contaminants."
      },
      {
         title = "A Family Freezer",
         desc = "Looking for a cryotube for your parents?  Our expert panel finds Glass Box Mark II will keep your family alive for nearly as long as the previous model."
      },
      --[[
         Business
      --]]
      {
         title = "Fullerton Reports Quarterly Loss",
         desc = "Engine maker Fullerton Industries has posted a c47B loss over the past 250 STPs. A company spokeswoman attributed the loss to the high cost of deuterium fuel and falling sales."
      },
      {
         title = "Genetric Board Meets",
         desc = "In the wake of the Yavi Bartolo's departure as CSO of Genetric Technologies, the board of directors will meet today. The group is expected to appoint a new science officer."
      },
      {
         title = "Aerosys Earnings Drop",
         desc = "The spaceways may swarm with Hyena-model craft, but today Aerosys recorded another quarterly loss. The company is investigating the discrepancy between the popularity of the craft and its sales figures."
      },
      {
         title = "Aerosys Victim of Pirate Manufacturing",
         desc = "The ship manufacturer has released a study indicating its signature Hyena model is being produced by a hidden system of unlicensed manufacturers."
      },
      {
         title = "Melendez CEO on Strategy",
         desc = "The Chief Executive Officer of ship maker Melendez Inc. thinks manufacturers should follow his companies lead in keeping costs down and producing for the mass market."
      },
      {
         title = "The Goddard Exception",
         desc = "Why has a community with more expertise sailing than flying produced the Empire's elite civilian spacecraft?  Lord Warthon says the secret lies in his family's hands-on tradition of leadership."
      },
      {
         title = "Nexus Contract Finalised",
         desc = "The Empire agreed terms with shipbuilder Nexus for a new generation of military craft. The deal extends the partnership with the government for another decade."
      },
      {
         title = "Sneak Peek: the Kestrel",
         desc = "Our reporter took a tour through Krain's mysterious space craft. He says it poses a challenge to the Goddard."
      },
      --[[
         Politics
      --]]
      {
         title = "FLF Responsible for Piracy",
         desc = "Law enforcement Expert Paet Dohmer's upcoming essay describes the group as \"more criminal gang than independence movement,\" according to his publicist."
      },
      {
         title = "Front Responsible for Shipping Woes",
         desc = "A spokeswoman for the separatist group says they were behind the recent series of attacks on cargo ships operating between Dakron and Theras. Dvaered officials condemned the actions."
      },
      {
         title = "Election on Caladan Marred by Fraud",
         desc = "As many as two of every hundred votes counted after the recent polling day may be falsified, an ombudsman reports. The opposition party demanded the election be annulled."
      },
      {
         title = "Empire Relies on Prison Labour",
         desc = "A recent report by the Soromid House Ways and Means Committee suggests infrastructure may be too dependent the on the incarcerated population."
      },
      {
         title = "Imperial Oversight Scandal",
         desc = "Sources close to the Imperial Chancellor say they see the failure at the Department of Oversight, whose inspectors overlooked serious failings in other supervisory bodies, as a serious oversight."
      },
      {
         title = "Governor Helmer Jailed",
         desc = "Imperial Auditors arrested governor Rex Helmer of Anecu on charges of corruption. He has been removed from office and transported to a holding facility awaiting trial."
      },
      {
         title = "Jouvanin Tapped as Interim Chief",
         desc = "Following the arrest of Rex Helmer, former Anecu deputy governor Elene Jouvanin will be sworn in today. She will serve out the term as governor."
      },
      {
         title = "Imperial Council Opens Doors",
         desc = "The supreme advisory body invited undergraduates from six top schools to sit in on a day's deliberations. Topics included biodiversity strategy."
      },
      {
         title = "FLF Terrorist Trial Ends",
         desc = "FLF Terrorist Trial ended this SCU with an unsurprising death sentence for all five members of the Nor spaceport bombing. Execution is scheduled in 10 STP."
      },
      {
         title = "New Challenges for New Times",
         desc = "The Dvaered council after a unanimous ruling decided to increase patrols in Dvaered space due to the recent uprising in FLF terrorism. The new measure is expected to start within the next SCU."
      },
      --[[
         Human interest.
      --]]
      {
         title = "Emperor's Aid Gets Hitched",
         desc = "On Saturday, imperial secretary Karil Lorenze married long time fiancee Rachid Baouda in the future palace gardens on the Emperor's Fist. His Eminence the bishop of Bao performed the ceremony."
      },
      {
         title = "Eyeteeth back in Fashion",
         desc = "Despite the advice of dentists, eyetooth caps have come into vogue again. Young people throughout the inhabited worlds use home kits to strip a layer of enamel from their teeth in favour of a binding of quartz, granite, or even flint."
      },
      {
         title = "Everyone Loves a SuperChimp",
         desc = "For decades used only as menial labourers, now SuperChimps are being widely adopted as domestic companions. Enhanced primates make an affectionate, intelligent pet, or low cost servant."
      },
      {
         title = "Admiral's Ball a Triumph",
         desc = "The glamorous season drew to a close with an underwater themed ball held in air bubbles deep under the oceans of Anecu. All the seasons debutantes attended."
      },
      {
         title = "Amazing Survival Story",
         desc = "An Xing Long was rescued after two days floating in space. \"I used meditation to slow my breathing,\" Xing Long told us. \"It was hard because I was scared.\""
      },
      {
         title = "The Best Spaceport Bars",
         desc = "Where can you get the best gargleblaster?  The famous exotic drinks list at the Doranthex Lava Room charmed our reviewer, but if you care for ambiance, don't miss the Goddard Bar."
      },
      {
         title = "RIP: The Floating Vagabond",
         desc = "Only two years after the mysterious disappearance of its owner, the galaxy's only deep space bar shut down the generators and cycled the airlock one last time."
      },
      {
         title = "Games for Young Pilots",
         desc = "Want your child to have a chance at a career as a space pilot?  Games like Super Julio Omniverse and SpaceFox help your child develop twitch muscles."
      },
      {
         title = "Remembering the Past",
         desc = "The Emperor has scheduled a new monument to be constructed on Emperor's Fist in honour of all those dead in the Incident."
      }
   }

   -- Insert into table
   for i,v in ipairs(gtable) do
      table.insert( rawtable, v )
   end
end
