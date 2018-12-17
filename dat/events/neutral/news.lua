

--[[
-- Event for creating generic news
--
--]]

header_table={}

header_table["Generic"] =     {_("We bring you the latest news in the galaxy.")
                              }
header_table["Independent"] = {_("Welcome to Universal News Feed. All the headlines all the time.")
                              }
header_table["Empire"] =      {_("Welcome to the Empire News Centre.")
                              }
header_table["Dvaered"] =     {_("Welcome to the Dvaered News Centre. All that happens. In simple words. So you can understand.")
                              }
header_table["Goddard"] =     {_("Welcome to Goddard News Centre. We bring you the news from around the Empire.")
                              }
header_table["Pirate"] =      {_("Pirate News. News that matters.")
                              }
header_table["Sirius"] =      {_("Sirius News Reel. Words of the Sirichana for all.")
                              }
header_table["FLF"] =         {_("The word of the Free Resistance.")
                              }
header_table["Frontier"] =    {_("News from the Frontier Alliance.")
                              }
header_table["Proteron"] =    {_("Word from the Proteron state.")
                              }
header_table["Za'lek"] =      {_("Scientific, Socioeconomic, and Sundry Events")
                              }
header_table["Soromid"] =     {_("The voice of the future.")
                              }
header_table["Thurion"] =     {_("Data Relay Network")
                              }

greet_table={}

greet_table["Generic"] =      {""
                              }
greet_table["Independent"] =  {""
                              }
greet_table["Empire"] =       {_("Fresh news from around the Empire."),
                              _("Remembering the Incident."),
                              _("Keeping you informed.")
                              }
greet_table["Dvaered"] =      {""
                              }
greet_table["Goddard"] =      {""}
greet_table["Pirate"] =       {_("News that matters."),
                              _("Adopt a cat today!"),
                              _("Laughing at the Emperor."),
                              _("On top of the world."),
                              _("Piracy has never been better.")
                              }
greet_table["FLF"] =          {""
                              }
greet_table["Frontier"] =     {_("News you can trust.")
                              }
greet_table["Sirius"] =       {_("Stay faithful."),
                              _("Sirichana watches and guides you.")
                              }
greet_table["Proteron"] =     {""
                              }
greet_table["Za'lek"] =       {""
                              }
greet_table["Soromid"] =      {_("Genetically tailoured for you.")
                              }
greet_table["Thurion"] =      {""
                              }

articles={}

articles["Generic"] = {
   --[[
      Science and technology
   --]]
   {
      title = _("Techs Probe for Sol"),
      desc = _("Technicians at the Omega Station recently returned from a new effort to understand the Sol Incident. They expect the first data to arrive in two months' time.")
   },
   {
      title = _("Sand Monster Caught On Tape"),
      desc = _("Local residents claim footage from an Arrakis security camera shows elusive beast walking through a desert storm. Exobiologists suspect a hoax.")
   },
   {
      title = _("New Handheld AI Debuts"),
      desc = _("Braeburn, the home division of Wellington, yesterday demonstrated their new PDAI. Developer Isaac Asimov assured us the new model is guaranteed not to develop a personality of its own.")
   },
   {
      title = _("Experiment Produces Cold Fusion"),
      desc = _("In an interview with Bleeding Edge anchor McKenzie Kruft, a researcher at Eureka labs says he has produced a tabletop atomic reaction. He hopes to publish his results in a science journal later this year.")
   },
   {
      title = _("Bees Introduced to Emperor's Fist"),
      desc = _("As they prepare the gardens of the future Imperial compound, entomologists have established the first colony of the Earth insects on the planet formerly known as G Scorpeii 5.")
   },
   {
      title = _("The Case for Sex"),
      desc = _("Though cloning rates continue to rise, Vlad Taneev believes we can still benefit from genetic recombination. \"Lady chance is more creative than we,\" says the famous gene splicer.")
   },
   {
      title = _("Hyperspace is Hot"),
      desc = _("Researchers have found traces of matter moving very quickly in hyperspace. They are not yet sure whether the minute particles originated there, or are human contaminants.")
   },
   {
      title = _("A Family Freezer"),
      desc = _("Looking for a cryotube for your parents?  Our expert panel finds Glass Box Mark II will keep your family alive for nearly as long as the previous model.")
   },
   --[[
      Business
   --]]
   {
      title = _("Fullerton Reports Quarterly Loss"),
      desc = _("Engine maker Fullerton Industries has posted a c47B loss over the past 25 decaperiods. A company spokeswoman attributed the loss to the high cost of deuterium fuel and falling sales.")
   },
   {
      title = _("Genetric Board Meets"),
      desc = _("In the wake of the Yavi Bartolo's departure as CSO of Genetric Technologies, the board of directors will meet today. The group is expected to appoint a new chief science officer.")
   },
   {
      title = _("Aerosys Earnings Drop"),
      desc = _("The spaceways may swarm with Hyena-model craft, but today Aerosys recorded another quarterly loss. The company is investigating the discrepancy between the popularity of the craft and its sales figures.")
   },
   {
      title = _("Aerosys Victim of Pirate Manufacturing"),
      desc = _("The ship manufacturer has released a study indicating its signature Hyena model is being produced by a hidden system of unlicensed manufacturers.")
   },
   {
      title = _("Melendez CEO on Strategy"),
      desc = _("The Chief Executive Officer of ship maker Melendez Inc. thinks manufacturers should follow his company's lead in keeping costs down and producing for the mass market.")
   },
   {
      title = _("The Goddard Exception"),
      desc = _("Why has a community with more expertise sailing than flying produced the Empire's elite civilian spacecraft?  Lord Warthon says the secret lies in his family's hands-on tradition of leadership.")
   },
   {
      title = _("Nexus Contract Finalised"),
      desc = _("The Empire agreed terms with shipbuilder Nexus for a new generation of military craft. The deal extends the partnership with the government for another decade.")
   },
   {
      title = _("Sneak Peek: the Kestrel"),
      desc = _("Our reporter took a tour through Krain's mysterious space craft. He says it poses a challenge to the Goddard.")
   },
   --[[
      Politics
   --]]
   {
      title = _("FLF Responsible for Piracy"),
      desc = _("Law enforcement expert Paet Dohmer's upcoming essay describes the group as \"more criminal gang than independence movement\", according to his publicist.")
   },
   {
      title = _("Front Responsible for Shipping Woes"),
      desc = _("A spokeswoman for the separatist group says they were behind the recent series of attacks on cargo ships operating between Dakron and Theras. Dvaered officials condemned the actions.")
   },
   {
      title = _("Election on Caladan Marred by Fraud"),
      desc = _("As many as two of every hundred votes counted after the recent polling day may be falsified, an ombudsman reports. The opposition party demanded the election be annulled.")
   },
   {
      title = _("Empire Relies on Prison Labour"),
      desc = _("A recent report by the Soromid House Ways and Means Committee suggests infrastructure may be too dependent the on the incarcerated population.")
   },
   {
      title = _("Imperial Oversight Scandal"),
      desc = _("Sources close to the Imperial Chancellor say they see the failure at the Department of Oversight, whose inspectors overlooked serious failings in other supervisory bodies, as a serious oversight.")
   },
   {
      title = _("Governor Helmer Jailed"),
      desc = _("Imperial Auditors arrested governor Rex Helmer of Anecu on charges of corruption. He has been removed from office and transported to a holding facility awaiting trial.")
   },
   {
      title = _("Jouvanin Tapped as Interim Chief"),
      desc = _("Following the arrest of Rex Helmer, former Anecu deputy governor Elene Jouvanin will be sworn in today. She will serve out the term as governor.")
   },
   {
      title = _("Imperial Council Opens Doors"),
      desc = _("The supreme advisory body invited undergraduates from six top schools to sit in on a day's deliberations. Topics included biodiversity strategy.")
   },
   {
      title = _("FLF Terrorist Trial Ends"),
      desc = _("FLF Terrorist Trial ended this cycle with an unsurprising death sentence for all five members of the Nor spaceport bombing. Execution is scheduled in 10 periods.")
   },
   {
      title = _("New Challenges for New Times"),
      desc = _("The Dvaered council after a unanimous ruling decided to increase patrols in Dvaered space due to the recent uprising in FLF terrorism. The new measure is expected to start within the next cycle.")
   },
   --[[
      Human interest.
   --]]
   {
      title = _("Emperor's Aid Gets Hitched"),
      desc = _("On Saturday, Imperial secretary Karil Lorenze married long time fiancee Rachid Baouda in the future palace gardens on Emperor's Fist. His Eminence the Bishop of Bao performed the ceremony.")
   },
   {
      title = _("Eyeteeth Back in Fashion"),
      desc = _("Despite the advice of dentists, eyetooth caps have come into vogue again. Young people throughout the inhabited worlds use home kits to strip a layer of enamel from their teeth in favour of a binding of quartz, granite, or even flint.")
   },
   {
      title = _("Everyone Loves a SuperChimp"),
      desc = _("For decades used only as menial labourers, now SuperChimps are being widely adopted as domestic companions. Enhanced primates make an affectionate, intelligent pet, or low-cost servant.")
   },
   {
      title = _("Admiral's Ball a Triumph"),
      desc = _("The glamorous season drew to a close with an underwater themed ball held in air bubbles deep under the oceans of Anecu. All the season's debutantes attended.")
   },
   {
      title = _("Amazing Survival Story"),
      desc = _("An Xing Long was rescued after two days floating in space. \"I used meditation to slow my breathing,\" Xing Long told us. \"It was hard because I was scared.\"")
   },
   {
      title = _("The Best Spaceport Bars"),
      desc = _("Where can you get the best gargleblaster?  The famous exotic drinks list at the Doranthex Lava Room charmed our reviewer, but if you care for ambiance, don't miss the Goddard Bar.")
   },
   {
      title = _("RIP: The Floating Vagabond"),
      desc = _("Only two years after the mysterious disappearance of its owner, the galaxy's only deep space bar shut down the generators and cycled the airlock one last time.")
   },
   {
      title = _("Games for Young Pilots"),
      desc = _("Want your child to have a chance at a career as a space pilot?  Games like Super Julio Omniverse and SpaceFox help your child develop twitch muscles.")
   },
   {
      title = _("Remembering the Past"),
      desc = _("The Emperor has scheduled a new monument to be constructed on Emperor's Fist in honour of all those dead in the Incident.")
   }
}

articles["Dvaered"]={
   --[[
      Science and technology
   --]]
   {
      title = _("New Mace Rockets"),
      desc = _("Dvaered Engineers are proud to present the new improved version of the Dvaered Mace rocket. \"We have proven the new rocket to be nearly twice as destructive as the previous versions,\" says Chief Dvaered Engineer Nordstrom.")
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
      title = _("Sirius Weaker Than Ever"),
      desc = _("This cycle breaks the negative record for fewest pilgrims to Mutris since the formation of House Sirius. This weakness is yet another sign that House Dvaered must increase patrols on the border and into Sirius space.") 
   }
}

articles["Goddard"]={
   --[[
      Science and technology
   --]]
   {
      title = _("Goddard: Raising the Bar"),
      desc = _("Many new scientists are being contracted by House Goddard to investigate possible improvements. This new strategy will increase the gap with the competing ship fabricators.")
   },
   --[[
      Business
   --]]
   {
      title = _("Goddard Earnings on the Rise"),
      desc = _("House Goddard has once again increased its earnings. \"Our investment in technology and quality has paid off,\" said Kari Baker of House Goddard's marketing bureau.")
   },
   {
      title = _("Goddard Awarded Best Ship"),
      desc = _("Once again the Goddard Battlecruiser was awarded the Best Overall Ship prize by the Dvaered Armada's annual Ship Awards. \"Very few ships have reliability like the Goddard,\" said Lord Warthon upon receiving the award on behalf of House Goddard.")
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
      title = _("Trade Meeting at Lorelei"),
      desc = _("Lorelei, in the Churchill system, is the latest world to host major trade negotiations between the Fyrra and the Space Traders Guild. The Fyrra Arch-Canter has indicated that opening up trade routes is a major goal.")
   },
   --[[
      Politics
   --]]
   {
      title = _("Dvaered extorting pilgrims"),
      desc = _("Recent pilgrims headed to Mutris have been telling stories of extortion and violations by Dvaered operators. Dvaered Warlord Kra'tok claims that these are \"delusions of the touched\". Official complaints have been made to the Emperor.")
   },
   --[[
      Human interest.
   --]]
   {
      title = _("Words of Tranquility"),
      desc = _("We welcome many new Touched who have recently begun ministering to the Shaira echelon after their long pilgrimage on Mutris. House Sirius is still a refugee for the orphans lost in this Universe.")
   }
}

articles["Pirate"]={
   --[[
      Science and technology
   --]]
   {
      title = _("Skull and Bones Improving"),
      desc = _("The technology behind Skull and Bones is advancing. Not only do they steal ships, but they improve on the original design. \"This gives us pirates an edge against the injustice of the Empire,\" says Millicent Felecia Black, lead Skull and Bones engineer.")
   },
   --[[
      Business
   --]]
   {
      title = _("Draconis Favorite Plundering Space"),
      desc = _("Draconis has recently passed Delta Pavonis in the pirate polls as the most favored plundering space. The abundance of traders and high interference make it an excellent place to get some fast credits.")
   },
   {
      title = _("New Ships for Skull and Bones"),
      desc = _("The Skull and Bones was able to extract a few dozen high quality vessels from Caladan warehouses under the nose of the Empire. These ships will help keep production high and booming.")
   },
   --[[
      Politics
   --]]
   {
      title = _("Emperor Weaker Than Ever"),
      desc = _("Recent actions demonstrate the inefficiency and weakness of the Emperor. One of the last irrational decisions left Eridani without a defense fleet to protect the traders. It's a great time to be a pirate.")
   },
   --[[
      Human interest.
   --]]
   {
      title = _("Cats in New Haven"),
      desc = _("An explosion in the cat population of New Haven has created an adoption campaign with the slogan, \"Pirate Cats, for those lonely space trips.\". Is your space vessel full of vermin? Adopt a cat today!")
   }
}

articles["Empire"]={
   --[[
      Science and technology
   --]]
   {
      title = _("Terraforming Emperor's Fist"),
      desc = _("New bleeding-edge terraforming techniques to be tried on Emperor's Fist. Studies show that these techniques could speed up the terraforming process by as much as 40%.")
   },
   --[[
      Business
   --]]
   {
      title = _("Empire Keeping Traders Safe"),
      desc = _("Recent studies show that reports of piracy on Trader vessels have gone down by up to 40% in some sectors. This is a demonstration of the Empire's commitment to eradicating piracy.")
   },
   --[[
      Politics
   --]]
   {
      title = _("New Empire Recruits"),
      desc = _("Emperor's recruiting strategy a success. Many new soldiers joining the Empire Armada. \"We haven't had such a successful campaign in ages!\" - Raid Steele, spokesman for recruiting campaign.")
   },
   --[[
      Human interest.
   --]]
   {
      title = _("New Cat in the Imperial Family"),
      desc = _("The Emperor's daughter was recently gifted a cat. Cat could be named \"Snuggles\" and seems to be all white.")
   }
}
articles["Independent"]={}
articles["FLF"]={}
articles["Frontier"]={}
articles["Proteron"]={}
articles["Za'lek"]={}
articles["Thurion"]={}


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
      warn( string.format( _('News: Faction \'%s\' does not have entry in faction table!'), faction) )
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
