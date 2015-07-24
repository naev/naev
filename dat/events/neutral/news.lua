

--[[
-- Event for creating generic news
--
--]]

lang = naev.lang()
if lang == 'es' then --not translated atm
else --default english

   header_table={}

   header_table["Generic"] =     {"We bring you the latest news in the galaxy."
                                 }
   header_table["Independent"] = {"Welcome to Universal News Feed. All the headlines all the time."
                                 }
   header_table["Empire"] =      {"Welcome to the Empire News Centre."
                                 }
   header_table["Dvaered"] =     {"Welcome to the Dvaered News Centre. All that happens. In simple words. So you can understand."
                                 }
   header_table["Goddard"] =     {"Welcome to Goddard News Centre. We bring you the news from around the Empire."
                                 }
   header_table["Pirate"] =      {"Pirate News. News that matters."
                                 }
   header_table["Sirius"] =      {"Sirius News Reel. Words of the Sirichana for all."
                                 }
   header_table["FLF"] =         {"The word of the Free Resistance."
                                 }
   header_table["Frontier"] =    {"News from the Frontier Alliance."
                                 }
   header_table["Proteron"] =    {"Word from the Proteron state."
                                 }
   header_table["Za'lek"] =      {"Scientific, Socioeconomic, and Sundry Events"
                                 }
   header_table["Soromid"] =     {"The voice of the future."
                                 }
   header_table["Thurion"] =     {"Data Relay Network"
                                 }
   
   greet_table={}

   greet_table["Generic"] =      {""
                                 }
   greet_table["Independent"] =  {""
                                 }
   greet_table["Empire"] =       {"Fresh news from around the Empire.",
                                 "Remembering the Incident.",
                                 "Keeping you informed."
                                 }
   greet_table["Dvaered"] =      {""
                                 }
   greet_table["Goddard"] =      {""}
   greet_table["Pirate"] =       {"News that matters.",
                                 "Adopt a cat today!",
                                 "Laughing at the Emperor.",
                                 "On top of the world.",
                                 "Piracy has never been better."
                                 }
   greet_table["FLF"] =          {""
                                 }
   greet_table["Frontier"] =     {"News you can trust."
                                 }
   greet_table["Sirius"] =       {"Stay faithful.",
                                 "Sirichana watches and guides you."
                                 }
   greet_table["Proteron"] =     {""
                                 }
   greet_table["Za'lek"] =       {""
                                 }
   greet_table["Soromid"] =      {"Genetically tailoured for you."
                                 }
   greet_table["Thurion"] =      {""
                                 }
   
   articles={}

   articles["Generic"] = {
      --[[
         Science and technology
      --]]
      {
         title = "Techs Probe for Sol",
         desc = "Technicians at the Omega Station recently returned from a new effort to understand the Sol Incident. They expect the first data to arrive in two months' time."
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
         desc = "As they prepare the gardens of the future Imperial compound, entomologists have established the first colony of the Earth insects on the planet formerly known as G Scorpeii 5."
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
         desc = "In the wake of the Yavi Bartolo's departure as CSO of Genetric Technologies, the board of directors will meet today. The group is expected to appoint a new chief science officer."
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
         desc = "The Chief Executive Officer of ship maker Melendez Inc. thinks manufacturers should follow his company's lead in keeping costs down and producing for the mass market."
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
         desc = "Law enforcement expert Paet Dohmer's upcoming essay describes the group as \"more criminal gang than independence movement\", according to his publicist."
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
         desc = "On Saturday, Imperial secretary Karil Lorenze married long time fiancee Rachid Baouda in the future palace gardens on Emperor's Fist. His Eminence the Bishop of Bao performed the ceremony."
      },
      {
         title = "Eyeteeth Back in Fashion",
         desc = "Despite the advice of dentists, eyetooth caps have come into vogue again. Young people throughout the inhabited worlds use home kits to strip a layer of enamel from their teeth in favour of a binding of quartz, granite, or even flint."
      },
      {
         title = "Everyone Loves a SuperChimp",
         desc = "For decades used only as menial labourers, now SuperChimps are being widely adopted as domestic companions. Enhanced primates make an affectionate, intelligent pet, or low-cost servant."
      },
      {
         title = "Admiral's Ball a Triumph",
         desc = "The glamorous season drew to a close with an underwater themed ball held in air bubbles deep under the oceans of Anecu. All the season's debutantes attended."
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

   articles["Dvaered"]={
      --[[
         Science and technology
      --]]
      {
         title = "New Mace Rockets",
         desc = "Dvaered Engineers are proud to present the new improved version of the Dvaered Mace rocket. \"We have proven the new rocket to be nearly twice as destructive as the previous versions,\" says Chief Dvaered Engineer Nordstrom."
      },
      --[[
         Business
      --]]
      --[[
         Politics
      --]]
      --[[
         Human interest.
      --]]
      {
         title = "Sirius Weaker Than Ever",
         desc = "This SCU breaks the negative record for fewest pilgrims to Mutris since the formation of House Sirius. This weakness is yet another sign that House Dvaered must increase patrols on the border and into Sirius space." 
      }
   }

   articles["Goddard"]={
      --[[
         Science and technology
      --]]
      {
         title = "Goddard: Raising the Bar",
         desc = "Many new scientists are being contracted by House Goddard to investigate possible improvements. This new strategy will increase the gap with the competing ship fabricators."
      },
      --[[
         Business
      --]]
      {
         title = "Goddard Earnings on the Rise",
         desc = "House Goddard has once again increased its earnings. \"Our investment in technology and quality has paid off,\" said Kari Baker of House Goddard's marketing bureau."
      },
      {
         title = "Goddard Awarded Best Ship",
         desc = "Once again the Goddard Battlecruiser was awarded the Best Overall Ship prize by the Dvaered Armada's annual Ship Awards. \"Very few ships have reliability like the Goddard,\" said Lord Warthon upon receiving the award on behalf of House Goddard."
      }
      --[[
         Politics
      --]]
      --[[
         Human interest.
      --]]
   }

   articles["Sirius"]={
      --[[
         Science and technology
      --]]
      --[[
         Business
      --]]
      {
         title = "Trade Meeting at Lorelei",
         desc = "Lorelei, in the Churchill system, is the latest world to host major trade negotiations between the Fyrra and the Space Traders Guild. The Fyrra Arch-Canter has indicated that opening up trade routes is a major goal."
      },
      --[[
         Politics
      --]]
      {
         title = "Dvaered extorting pilgrims",
         desc = "Recent pilgrims headed to Mutris have been telling stories of extortion and violations by Dvaered operators. Dvaered Warlord Kra'tok claims that these are \"delusions of the touched\". Official complaints have been made to the Emperor."
      },
      --[[
         Human interest.
      --]]
      {
         title = "Words of Tranquility",
         desc = "We welcome many new Touched who have recently begun ministering to the Shaira echelon after their long pilgrimage on Mutris. House Sirius is still a refugee for the orphans lost in this Universe."
      }
   }

   articles["Pirate"]={
      --[[
         Science and technology
      --]]
      {
         title = "Skull and Bones Improving",
         desc = "The technology behind Skull and Bones is advancing. Not only do they steal ships, but they improve on the original design. \"This gives us pirates an edge against the injustice of the Empire,\" says Millicent Felecia Black, lead Skull and Bones engineer."
      },
      --[[
         Business
      --]]
      {
         title = "Draconis Favorite Plundering Space",
         desc = "Draconis has recently passed Delta Pavonis in the pirate polls as the most favored plundering space. The abundance of traders and high interference make it an excellent place to get some fast credits."
      },
      {
         title = "New Ships for Skull and Bones",
         desc = "The Skull and Bones was able to extract a few dozen high quality vessels from Caladan warehouses under the nose of the Empire. These ships will help keep production high and booming."
      },
      --[[
         Politics
      --]]
      {
         title = "Emperor Weaker Than Ever",
         desc = "Recent actions demonstrate the inefficiency and weakness of the Emperor. One of the last irrational decisions left Eridani without a defense fleet to protect the traders. It's a great time to be a pirate."
      },
      --[[
         Human interest.
      --]]
      {
         title = "Cats in New Haven",
         desc = "An explosion in the cat population of New Haven has created an adoption campaign with the slogan, \"Pirate Cats, for those lonely space trips.\". Is your space vessel full of vermin? Adopt a cat today!"
      }
   }

   articles["Empire"]={
      --[[
         Science and technology
      --]]
      {
         title = "Terraforming Emperor's Fist",
         desc = "New bleeding-edge terraforming techniques to be tried on Emperor's Fist. Studies show that these techniques could speed up the terraforming process by as much as 40%."
      },
      --[[
         Business
      --]]
      {
         title = "Empire Keeping Traders Safe",
         desc = "Recent studies show that reports of piracy on Trader vessels have gone down by up to 40% in some sectors. This is a demonstration of the Empire's commitment to eradicating piracy."
      },
      --[[
         Politics
      --]]
      {
         title = "New Empire Recruits",
         desc = "Emperor's recruiting strategy a success. Many new soldiers joining the Empire Armada. \"We haven't had such a successful campaign in ages!\" - Raid Steele, spokesman for recruiting campaign."
      },
      --[[
         Human interest.
      --]]
      {
         title = "New Cat in the Imperial Family",
         desc = "The Emperor's daughter was recently gifted a cat. Cat could be named \"Snuggles\" and seems to be all white."
      }
   }
   articles["Independent"]={}
   articles["FLF"]={}
   articles["Frontier"]={}
   articles["Proteron"]={}
   articles["Za'lek"]={}
   articles["Thurion"]={}



end



   --create generic news
function create()

   faction = planet.cur():faction():name()

   remove_header(faction)

   rm_genericarticles("Generic")
   rm_genericarticles(faction)

   add_header(faction)
   add_articles("Generic",math.random(3,4))
   add_articles(faction,math.random(1,2))

end

function add_header(faction)

   remove_header('Generic')

   if header_table[faction] == nil then
      warn( 'News: Faction \''..faction..'\' does not have entry in faction table!' )
      faction = 'Generic'
   end

   header=header_table[faction][1]
   desc=greet_table[faction][math.random(#greet_table[faction])]

   news.add(faction,header,desc,50000000000005,50000000000005)

end

function remove_header(faction)

   local news_table=news.get(faction)

   for _,v in ipairs(news.get(faction)) do

      for _,v0 in ipairs(header_table[faction]) do
         if v:title()==v0 then
            v:rm()
         end
      end

   end

   return 0

end


function add_articles(faction,num)

   if articles[faction]==nil then
      return 0
   end

   num=math.min(num,#articles[faction])
   news_table=articles[faction]

   local header
   local desc
   local rnum

   for i=1,num do

      rnum=math.random(#news_table)
      header=news_table[rnum]["title"]
      desc=news_table[rnum]["desc"]

      news.add(faction,header,desc,500000000000000,0)

      table.remove(news_table,rnum)

   end

end

function rm_genericarticles(faction)

   local news_table=news.get(faction)

   if articles[faction]==nil then return 0 end

   for _,v in ipairs(news_table) do

      for _,v0 in ipairs(articles[faction]) do
         if v:title()==v0["title"] then
            v:rm()
            break
         end
      end
   
   end


end
