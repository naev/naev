--[[
-- Event for creating news
--
--]]

require "numstring.lua"
require "jumpdist.lua"


articles = {}

articles["Generic"] = {
   --[[
      Science and technology
   --]]
   {
      tag = "Techs Probe for Sol",
      title = _("Techs Probe for Sol"),
      desc = _("Technicians recently returned from a new effort to understand the Sol Incident. They expect the first data to arrive in 50 decaperiods' time.")
   },
   {
      tag = "Sand Monster Caught On Tape",
      title = _("Sand Monster Caught On Tape"),
      desc = _("Local residents claim footage from a security camera shows elusive beast walking through a desert storm. Exobiologists suspect a hoax.")
   },
   {
      tag = "New Handheld AI Debuts",
      title = _("New Handheld AI Debuts"),
      desc = _("Braeburn, the home division of Wellington, yesterday demonstrated their new PDAI. Developer Isaac Asimov assured us the new model is guaranteed not to develop a personality of its own.")
   },
   {
      tag = "Experiment Produces Cold Fusion",
      title = _("Experiment Produces Cold Fusion"),
      desc = _("In an interview with Bleeding Edge anchor McKenzie Kruft, a researcher at Eureka labs says he has produced a tabletop atomic reaction. He hopes to publish his results in a science journal later this cycle.")
   },
   {
      tag = "The Case for Sex",
      title = _("The Case for Sex"),
      desc = _("Though cloning rates continue to rise, Vlad Taneev believes we can still benefit from genetic recombination. \"Lady chance is more creative than we,\" says the famous gene splicer.")
   },
   {
      tag = "Hyperspace is Hot",
      title = _("Hyperspace is Hot"),
      desc = _("Researchers have found traces of matter moving very quickly in hyperspace. They are not yet sure whether the minute particles originated there, or are human contaminants.")
   },
   {
      tag = "A Family Freezer",
      title = _("A Family Freezer"),
      desc = _("Looking for a cryotube for your parents?  Our expert panel finds Glass Box Mark II will keep your family alive for nearly as long as the previous model.")
   },
   --[[
      Business
   --]]
   {
      tag = "Fullerton Reports Quarterly Loss",
      title = _("Fullerton Reports Quarterly Loss"),
      desc = _("Engine maker Fullerton Industries has posted a c47B loss over the past 25 decaperiods. A company spokeswoman attributed the loss to the high cost of deuterium fuel and falling sales.")
   },
   {
      tag = "Genetric Board Meets",
      title = _("Genetric Board Meets"),
      desc = _("In the wake of the Yavi Bartolo's departure as CSO of Genetric Technologies, the board of directors will meet today. The group is expected to appoint a new chief science officer.")
   },
   --[[
      Politics
   --]]
   --[[
      Human interest.
   --]]
   {
      tag = "Eyeteeth Back in Fashion",
      title = _("Eyeteeth Back in Fashion"),
      desc = _("Despite the advice of dentists, eyetooth caps have come into vogue again. Young people throughout the inhabited worlds use home kits to strip a layer of enamel from their teeth in favour of a binding of quartz, granite, or even flint.")
   },
   {
      tag = "Everyone Loves a SuperChimp",
      title = _("Everyone Loves a SuperChimp"),
      desc = _("For dozens of cycles used only as menial labourers, now SuperChimps are being widely adopted as domestic companions. Enhanced primates make an affectionate, intelligent pet, or a low-cost servant.")
   },
   {
      tag = "Admiral's Ball a Triumph",
      title = _("Admiral's Ball a Triumph"),
      desc = _("The glamorous season drew to a close with an underwater themed ball held in air bubbles deep under the oceans of Anecu. All the season's debutantes attended.")
   },
   {
      tag = "Amazing Survival Story",
      title = _("Amazing Survival Story"),
      desc = _("An Xing Long was rescued after two decaperiods floating in space. \"I used meditation to slow my breathing,\" Xing Long told us. \"It was hard because I was scared.\"")
   },
   {
      tag = "The Best Spaceport Bars",
      title = _("The Best Spaceport Bars"),
      desc = _("Where can you get the best gargleblaster?  The famous exotic drinks list at the Doranthex Lava Room charmed our reviewer, but if you care for ambiance, don't miss the Goddard Bar.")
   },
   {
      tag = "RIP: The Floating Vagabond",
      title = _("RIP: The Floating Vagabond"),
      desc = _("Only two cycles after the mysterious disappearance of its owner, the galaxy's only deep space bar shut down the generators and cycled the airlock one last time.")
   },
   {
      tag = "Games for Young Pilots",
      title = _("Games for Young Pilots"),
      desc = _("Want your child to have a chance at a career as a space pilot?  Games like Super Julio Omniverse and SpaceFox help your child develop twitch muscles.")
   },
}

articles["Dvaered"] = {
   --[[
      Science and technology
   --]]
   {
      tag = "New Mace Rockets",
      title = _("New Mace Rockets"),
      desc = _("Dvaered Engineers are proud to present the new improved version of the Dvaered Mace rocket. \"We have proven the new rocket to be nearly twice as destructive as the previous versions,\" says Chief Dvaered Engineer Nordstrom.")
   },
   --[[
      Business
   --]]
   --[[
      Politics
   --]]
   {
      tag = "FLF Responsible for Piracy",
      title = _("FLF Responsible for Piracy"),
      desc = _("Law enforcement expert Paet Dohmer's upcoming essay describes the group as \"more criminal gang than independence movement\", according to his publicist.")
   },
   {
      tag = "Front Responsible for Shipping Woes",
      title = _("Front Responsible for Shipping Woes"),
      desc = _("A spokeswoman for the separatist group says they were behind the recent series of attacks on cargo ships operating between Dakron and Theras. Dvaered officials condemned the actions.")
   },
   {
      tag = "Jouvanin Tapped as Interim Chief",
      title = _("Jouvanin Tapped as Interim Chief"),
      desc = _("Following the arrest of Rex Helmer, former Anecu deputy governor Elene Jouvanin will be sworn in today. She will serve out the term as governor.")
   },
   {
      tag = "FLF Terrorist Trial Ends",
      title = _("FLF Terrorist Trial Ends"),
      desc = _("FLF Terrorist Trial ended this cycle with an unsurprising death sentence for all five members of the Nor spaceport bombing. Execution is scheduled in 10 periods.")
   },
   {
      tag = "New Challenges for New Times",
      title = _("New Challenges for New Times"),
      desc = _("The Dvaered council after a unanimous ruling decided to increase patrols in Dvaered space due to the recent uprising in FLF terrorism. The new measure is expected to start within the next cycle.")
   },
   --[[
      Human interest.
   --]]
   {
      tag = "Sirius Weaker Than Ever",
      title = _("Sirius Weaker Than Ever"),
      desc = _("This cycle breaks the negative record for fewest pilgrims to Mutris since the formation of House Sirius. This weakness is yet another sign that House Dvaered must increase patrols on the border and into Sirius space.") 
   }
}

articles["Goddard"] = {
   --[[
      Science and technology
   --]]
   {
      tag = "Goddard: Raising the Bar",
      title = _("Goddard: Raising the Bar"),
      desc = _("Many new scientists are being contracted by House Goddard to investigate possible improvements. This new strategy will increase the gap with the competing ship fabricators.")
   },
   --[[
      Business
   --]]
   {
      tag = "Goddard Earnings on the Rise",
      title = _("Goddard Earnings on the Rise"),
      desc = _("House Goddard has once again increased its earnings. \"Our investment in technology and quality has paid off,\" said Kari Baker of House Goddard's marketing bureau.")
   },
   {
      tag = "Goddard Awarded Best Ship",
      title = _("Goddard Awarded Best Ship"),
      desc = _("Once again the Goddard Battlecruiser was awarded the Best Overall Ship prize by the Dvaered Armada's annual Ship Awards. \"Very few ships have reliability like the Goddard,\" said Lord Warthon upon receiving the award on behalf of House Goddard.")
   },
   {
      tag = "Aerosys Earnings Drop",
      title = _("Aerosys Earnings Drop"),
      desc = _("The spaceways may swarm with Hyena-model craft, but today Aerosys recorded another quarterly loss. The company is investigating the discrepancy between the popularity of the craft and its sales figures.")
   },
   {
      tag = "Aerosys Victim of Pirate Manufacturing",
      title = _("Aerosys Victim of Pirate Manufacturing"),
      desc = _("The ship manufacturer has released a study indicating its signature Hyena model is being produced by a hidden system of unlicensed manufacturers.")
   },
   {
      tag = "Melendez CEO on Strategy",
      title = _("Melendez CEO on Strategy"),
      desc = _("The Chief Executive Officer of ship maker Melendez Inc. thinks manufacturers should follow his company's lead in keeping costs down and producing for the mass market.")
   },
   {
      tag = "The Goddard Exception",
      title = _("The Goddard Exception"),
      desc = _("Why has a community with more expertise sailing than flying produced the Empire's elite civilian spacecraft?  Lord Warthon says the secret lies in his family's hands-on tradition of leadership.")
   },
   {
      tag = "Sneak Peek: the Kestrel",
      title = _("Sneak Peek: the Kestrel"),
      desc = _("Our reporter took a tour through Krain's mysterious space craft. He says it poses a challenge to the Goddard.")
   },
   --[[
      Politics
   --]]
   --[[
      Human interest.
   --]]
}

articles["Sirius"] = {
   --[[
      Science and technology
   --]]
   --[[
      Business
   --]]
   {
      tag = "Trade Meeting at Lorelei",
      title = _("Trade Meeting at Lorelei"),
      desc = _("Lorelei, in the Churchill system, is the latest world to host major trade negotiations between the Fyrra and the Space Traders Guild. The Fyrra Arch-Canter has indicated that opening up trade routes is a major goal.")
   },
   --[[
      Politics
   --]]
   {
      tag = "Dvaered extorting pilgrims",
      title = _("Dvaered extorting pilgrims"),
      desc = _("Recent pilgrims headed to Mutris have been telling stories of extortion and violations by Dvaered operators. Dvaered Warlord Kra'tok claims that these are \"delusions of the touched\". Official complaints have been made to the Emperor.")
   },
   --[[
      Human interest.
   --]]
   {
      tag = "Words of Tranquility",
      title = _("Words of Tranquility"),
      desc = _("We welcome many new Touched who have recently begun ministering to the Shaira echelon after their long pilgrimage on Mutris. House Sirius is still a refugee for the orphans lost in this Universe.")
   },
}

articles["Pirate"] = {
   --[[
      Science and technology
   --]]
   {
      tag = "Skull and Bones Improving",
      title = _("Skull and Bones Improving"),
      desc = _("The technology behind Skull and Bones is advancing. Not only do they steal ships, but they improve on the original design. \"This gives us pirates an edge against the injustice of the Empire,\" says Millicent Felecia Black, lead Skull and Bones engineer.")
   },
   --[[
      Business
   --]]
   {
      tag = "Draconis Favorite Plundering Space",
      title = _("Draconis Favorite Plundering Space"),
      desc = _("Draconis has recently passed Delta Pavonis in the pirate polls as the most favored plundering space. The abundance of traders and high interference make it an excellent place to get some fast credits.")
   },
   {
      tag = "New Ships for Skull and Bones",
      title = _("New Ships for Skull and Bones"),
      desc = _("The Skull and Bones was able to extract a few dozen high quality vessels from Caladan warehouses under the nose of the Empire. These ships will help keep production high and booming.")
   },
   --[[
      Politics
   --]]
   {
      tag = "Emperor Weaker Than Ever",
      title = _("Emperor Weaker Than Ever"),
      desc = _("Recent actions demonstrate the inefficiency and weakness of the Emperor. One of the last irrational decisions left Eridani without a defense fleet to protect the traders. It's a great time to be a pirate.")
   },
   --[[
      Human interest.
   --]]
   {
      tag = "Cats in New Haven",
      title = _("Cats in New Haven"),
      desc = _("An explosion in the cat population of New Haven has created an adoption campaign with the slogan, \"Pirate Cats, for those lonely space trips.\". Is your space vessel full of vermin? Adopt a cat today!")
   },
}

articles["Empire"] = {
   --[[
      Science and technology
   --]]
   {
      tag = "Terraforming Emperor's Fist",
      title = _("Terraforming Emperor's Fist"),
      desc = _("New bleeding-edge terraforming techniques to be tried on Emperor's Fist. Studies show that these techniques could speed up the terraforming process by as much as 40%.")
   },
   {
      tag = "Bees Introduced to Emperor's Fist",
      title = _("Bees Introduced to Emperor's Fist"),
      desc = _("As they prepare the gardens of the future Imperial compound, entomologists have established the first colony of the Earth insects on the planet formerly known as G Scorpeii 5.")
   },
   --[[
      Business
   --]]
   {
      tag = "Empire Keeping Traders Safe",
      title = _("Empire Keeping Traders Safe"),
      desc = _("Recent studies show that reports of piracy on Trader vessels have gone down by up to 40% in some sectors. This is a demonstration of the Empire's commitment to eradicating piracy.")
   },
   {
      tag = "Nexus Contract Finalised",
      title = _("Nexus Contract Finalised"),
      desc = _("The Empire agreed to terms with shipbuilder Nexus for a new generation of military craft. The deal extends the partnership with the government for another 10 cycles.")
   },
   --[[
      Politics
   --]]
   {
      tag = "New Empire Recruits",
      title = _("New Empire Recruits"),
      desc = _("Emperor's recruiting strategy a success. Many new soldiers joining the Empire Armada. \"We haven't had such a successful campaign in ages!\" - Raid Steele, spokesman for recruiting campaign.")
   },
   {
      tag = "Governor Helmer Jailed",
      title = _("Governor Helmer Jailed"),
      desc = _("Imperial Auditors arrested governor Rex Helmer of Anecu on charges of corruption. He has been removed from office and transported to a holding facility awaiting trial.")
   },
   {
      tag = "Imperial Council Opens Doors",
      title = _("Imperial Council Opens Doors"),
      desc = _("The supreme advisory body invited undergraduates from six top schools to sit in on a day's deliberations. Topics required biodiversity strategy.")
   },
   --[[
      Human interest.
   --]]
   {
      tag = "New Cat in the Imperial Family",
      title = _("New Cat in the Imperial Family"),
      desc = _("The Emperor's daughter was recently gifted a cat. Cat could be named \"Snuggles\" and seems to be all white.")
   },
   {
      tag = "Emperor's Aid Gets Hitched",
      title = _("Emperor's Aid Gets Hitched"),
      desc = _("Imperial secretary Karil Lorenze married long time fiancee Rachid Baouda in the future palace gardens on Emperor's Fist. His Eminence the Bishop of Bao performed the ceremony.")
   },
   {
      tag = "Remembering the Past",
      title = _("Remembering the Past"),
      desc = _("The Emperor has scheduled a new monument to be constructed on Emperor's Fist in honour of all those dead in the Incident.")
   },
}

articles["Frontier"] = {
   --[[
      Science and technology
   --]]
   --[[
      Business
   --]]
   --[[
      Politics
   --]]
   {
      tag = "Election on Caladan Marred by Fraud",
      title = _("Election on Caladan Marred by Fraud"),
      desc = _("As many as two of every hundred votes counted after the recent polling decaperiod may be falsified, an ombudsman reports. The opposition party demanded the election be annulled.")
   },
   {
      tag = "Empire Relies on Prison Labour",
      title = _("Empire Relies on Prison Labour"),
      desc = _("A recent report by the Soromid House Ways and Means Committee suggests infrastructure may be too dependent the on the incarcerated population.")
   },
   {
      tag = "Imperial Oversight Scandal",
      title = _("Imperial Oversight Scandal"),
      desc = _("Sources close to the Imperial Chancellor say they see the failure at the Department of Oversight, whose inspectors overlooked serious failings in other supervisory bodies, as a serious oversight.")
   },
   --[[
      Human interest.
   --]]
}

articles["Independent"] = {}
articles["FLF"] = {}
articles["Proteron"] = {}
articles["Za'lek"] = {}
articles["Thurion"] = {}

econ_articles = {
   {
      title = _("Unfortunate Merchant Goes Bankrupt"),
      desc = _("A merchant was forced into bankruptcy due to a badly timed trade of %s on %s. \"I thought %s credits per tonne was a good deal, but it turns out I should have waited,\" the merchant said.")
   },
   {
      title = _("Shipping Company Goes Out of Business"),
      desc = _("A small shipping business failed just this decaperiod. While it was already failing, what finally put the company under was a poorly-timed trade of %s on %s for %s credits per tonne. \"It was poor executive decision,\" one analyst asserts. \"Patience is key when trading, and it's clear that the owner of this company didn't have enough of that.\"")
   },
   {
      title = _("Interview with an Economist"),
      desc = _("One of the galaxy's foremost experts on economics gives an interview explaining our modern economy. \"We actually have a pretty good understanding of how the economy works. For example, we were able to predict what the price of %s on %s would reach very accurately; the actual price reached was %s credits per tonne, which we were only off by about 15%%. Over time, we hope to lower that margin of error to as little as 2%%.\"")
   },
   {
      title = _("Economist Describes Sinusoidal Economic Theory"),
      desc = _("A little-known economist discusses a controversial economic theory. \"When you look at the trends, it resembles a sine wave. For instance, the price of %s on %s is now %s credits per tonne, and it seems to return to that price with some regularity. We are working on developing a model to predict these curves more accurately.\" Other economists disagree, however, attributing these economists' results to chance.")
   },
   {
      title = _("Young Pilot Buys Their First Commodity"),
      desc = _("A young pilot has bought some %s as a way of breaking into the freelance piloting business. Born and raised on %s, where they bought their first commodity, they spoke with enthusiasm for the new career. \"You know, it's real exciting! Even on my home planet the price of %s isn't static, but when you look all around, there's so much price variation, so much potential for profit! I'm excited to finally get started.\"")
   },
}

econ_title = _("Current Market Prices")
econ_header = _("\n\n%s in %s")
econ_desc_part = _("\n%s: %s Cr./Tonne")


   --create generic news
function create()
   local f = planet.cur():faction()
   local my_faction = f ~= nil and f:name() or "Generic"
   if my_faction == nil then evt.finish(false) end

   add_article( my_faction )
   add_article( "Generic" )
   add_econ_article()
end


function add_article( my_faction )
   local last_article = var.peek( "news_last_article" )
   if last_article ~= nil then
      local t = time.fromnumber( last_article )
      if time.get() - t < time.create( 0, 1, 5000 ) then
         return
      end
   end

   if rnd.rnd() > 0.25 then
      return
   end

   local alst = articles[my_faction]
   if alst == nil or #alst <= 0 then
      return
   end

   local i = rnd.rnd( 1, #alst )
   local tag = alst[i]["tag"]
   local title = alst[i]["title"]
   local desc = alst[i]["desc"]

   if #news.get( tag ) > 0 then
      return
   end

   local exp = time.get() + time.create( 0, 10, 5000 * rnd.sigma() )
   local a = news.add( my_faction, title, desc, exp )
   a:bind( tag )
   var.push( "news_last_article", time.get():tonumber() )
end


function add_econ_article ()
   local last_article = var.peek( "news_last_econ_article" )
   local t = nil
   if last_article ~= nil then t = time.fromnumber( last_article ) end
   if (t == nil or time.get() - t > time.create( 0, 1, 0 ))
         and rnd.rnd() < 0.75 then
      local planets = {}
      for i, s in ipairs( getsysatdistance( system.cur(), 2, 4 ) ) do
         for j, p in ipairs( s:planets() ) do
            if p:faction() ~= faction.get("Pirate")
                  and p:faction() ~= faction.get("FLF")
                  and #(p:commoditiesSold()) > 0 then
               planets[ #planets + 1 ] = p
            end
         end
      end
      if #planets > 0 then
         local p = planets[ rnd.rnd( 1, #planets ) ]
         local pd = time.get() - time.create(
               0, p:system():jumpDist() + rnd.rnd( 0, 1 ), 9000 * rnd.sigma() )
         local exp = time.get() + time.create( 0, 5, 5000 * rnd.sigma() )
         local commchoices = p:commoditiesSold()
         local commod = commchoices[ rnd.rnd( 1, #commchoices ) ]
         local price = commod:priceAtTime( p, pd )
         local i = rnd.rnd( 1, #econ_articles )
         local title = econ_articles[i]["title"]
         local desc = econ_articles[i]["desc"]:format(
               commod:name(), p:name(), numstring( price ) )
         news.add( "Generic", title, desc, exp, pd )
         p:recordCommodityPriceAtTime( pd )
         var.push( "news_last_econ_article", time.get():tonumber() )
      end
   end

   -- Remove old econ articles (we only want one at a time)
   for i, article in ipairs( news.get( "econ" ) ) do
      article:rm()
   end

   local cur_t = time.get()
   local body = ""
   for i, sys in ipairs( getsysatdistance( system.cur(), 0, 1 ) ) do
      for j, plnt in ipairs( sys:planets() ) do
         local commodities = plnt:commoditiesSold()
         if #commodities > 0 then
            body = body .. econ_header:format( plnt:name(), sys:name() )
            for k, comm in ipairs( commodities ) do
               body = body .. econ_desc_part:format( comm:name(),
                     numstring( comm:priceAtTime( plnt, cur_t ) ) )
            end
            plnt:recordCommodityPriceAtTime( cur_t )
         end
      end
   end
   if body ~= "" then
      -- Create news, expires immediately when time advances (i.e.
      -- when you take off from the planet).
      local a = news.add( "Generic", econ_title, body,
            cur_t + time.create( 0, 0, 1 ) )
      a:bind( "econ" )
   end
end
