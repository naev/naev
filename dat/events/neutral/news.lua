--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Generic News">
 <trigger>land</trigger>
 <chance>100</chance>
</event>
--]]
--[[
-- Event for creating news
--
--]]

require "numstring.lua"
require "jumpdist.lua"


header_table = {}

header_table["Generic"] = _("We bring you the latest news in the galaxy.")
header_table["Independent"] = _("Welcome to Universal News Feed. All the headlines all the time.")
header_table["Empire"] = _("Welcome to the Empire News Centre.")
header_table["Dvaered"] = _("Welcome to the Dvaered News Centre. All that happens. In simple words. So you can understand.")
header_table["Goddard"] = _("Welcome to Goddard News Centre. We bring you the news from around the Empire.")
header_table["Pirate"] = _("Pirate News. News that matters.") 
header_table["Sirius"] = _("Sirius News Reel. Words of the Sirichana for all.")
header_table["FLF"] = _("The word of the Free Resistance.")
header_table["Frontier"] = _("News from the Frontier Alliance.")
header_table["Proteron"] = _("Word from the Proteron state.")
header_table["Za'lek"] = _("Scientific, Socioeconomic, and Sundry Events")
header_table["Soromid"] = _("The voice of the future.")
header_table["Thurion"] = _("Data Relay Network")

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


articles = {}

articles["Generic"] = {
   --[[
      Science and technology
   --]]
   {
      tag = N_("Techs Probe for Sol"),
      desc = _("Technicians recently returned from a new effort to understand the Sol Incident. They expect the first data to arrive in 50 decaperiods' time.")
   },
   {
      tag = N_("Sand Monster Caught On Tape"),
      desc = _("Local residents claim footage from a security camera shows elusive beast walking through a desert storm. Exobiologists suspect a hoax.")
   },
   {
      tag = N_("New Handheld AI Debuts"),
      desc = _("Braeburn, the home division of Wellington, yesterday demonstrated their new PDAI. Developer Isaac Asimov assured us the new model is guaranteed not to develop a personality of its own.")
   },
   {
      tag = N_("Experiment Produces Cold Fusion"),
      desc = _("In an interview with Bleeding Edge anchor McKenzie Kruft, a researcher at Eureka labs says he has produced a tabletop atomic reaction. He hopes to publish his results in a science journal later this cycle.")
   },
   {
      tag = N_("The Case for Sex"),
      desc = _("Though cloning rates continue to rise, Vlad Taneev believes we can still benefit from genetic recombination. \"Lady chance is more creative than we,\" says the famous gene splicer.")
   },
   {
      tag = N_("Hyperspace is Hot"),
      desc = _("Researchers have found traces of matter moving very quickly in hyperspace. They are not yet sure whether the minute particles originated there, or are human contaminants.")
   },
   {
      tag = N_("A Family Freezer"),
      desc = _("Looking for a cryotube for your parents?  Our expert panel finds Glass Box Mark II will keep your family alive for nearly as long as the previous model.")
   },
   --[[
      Business
   --]]
   {
      tag = N_("Fullerton Reports Quarterly Loss"),
      desc = _("Engine maker Fullerton Industries has posted a c47B loss over the past 25 decaperiods. A company spokeswoman attributed the loss to the high cost of deuterium fuel and falling sales.")
   },
   {
      tag = N_("Genetric Board Meets"),
      desc = _("In the wake of the Yavi Bartolo's departure as CSO of Genetric Technologies, the board of directors will meet today. The group is expected to appoint a new chief science officer.")
   },
   --[[
      Politics
   --]]
   --[[
      Human interest.
   --]]
   {
      tag = N_("Eyeteeth Back in Fashion"),
      desc = _("Despite the advice of dentists, eyetooth caps have come into vogue again. Young people throughout the inhabited worlds use home kits to strip a layer of enamel from their teeth in favour of a binding of quartz, granite, or even flint.")
   },
   {
      tag = N_("Everyone Loves a SuperChimp"),
      desc = _("For dozens of cycles used only as menial labourers, now SuperChimps are being widely adopted as domestic companions. Enhanced primates make an affectionate, intelligent pet, or a low-cost servant.")
   },
   {
      tag = N_("Admiral's Ball a Triumph"),
      desc = _("The glamorous season drew to a close with an underwater themed ball held in air bubbles deep under the oceans of Anecu. All the season's debutantes attended.")
   },
   {
      tag = N_("Amazing Survival Story"),
      desc = _("An Xing Long was rescued after two decaperiods floating in space. \"I used meditation to slow my breathing,\" Xing Long told us. \"It was hard because I was scared.\"")
   },
   {
      tag = N_("The Best Spaceport Bars"),
      desc = _("Where can you get the best gargle blaster?  The famous exotic drinks list at the Doranthex Lava Room charmed our reviewer, but if you care for ambiance, don't miss the Goddard Bar.")
   },
   {
      tag = N_("RIP: The Floating Vagabond"),
      desc = _("Only two cycles after the mysterious disappearance of its owner, the galaxy's only deep space bar shut down the generators and cycled the airlock one last time.")
   },
   {
      tag = N_("Games for Young Pilots"),
      desc = _("Want your child to have a chance at a career as a space pilot?  Games like Super Julio Omniverse and SpaceFox help your child develop twitch muscles.")
   },
   {
      tag = N_("Former Pirate Writes Target Management Self-Help Book"),
      desc = string.format(
         _("A former pirate shares her story on how she steered herself away from piracy, which she wrote about in an award-winning self-help book. \"I used to spend my whole life pressing %s to target enemies, but my life changed when I had a dream about a cat munching on some grass. 'Are you using the %s key?' it asked. 'I find that it is very useful.' I have been doing as the strange cat in my dream said ever since, and I no longer have to lose money or alienate friends. If the universe followed this simple advice, I suspect we would live in a much safer society.\""),
         naev.keyGet("target_nearest"), naev.keyGet("target_hostile") )
   },
}

articles["Dvaered"] = {
   --[[
      Science and technology
   --]]
   {
      tag = N_("New Mace Rockets"),
      desc = _("Dvaered Engineers are proud to present the new improved version of the Dvaered Mace rocket. \"We have proven the new rocket to be nearly twice as destructive as the previous versions,\" says Chief Dvaered Engineer Nordstrom.")
   },
   --[[
      Business
   --]]
   --[[
      Politics
   --]]
   {
      tag = N_("FLF Responsible for Piracy"),
      desc = _("Law enforcement expert Paet Dohmer's upcoming essay describes the group as \"more criminal gang than independence movement\", according to his publicist.")
   },
   {
      tag = N_("Front Responsible for Shipping Woes"),
      desc = _("A spokeswoman for the separatist group says they were behind the recent series of attacks on cargo ships operating between Dakron and Theras. Dvaered officials condemned the actions.")
   },
   {
      tag = N_("Jouvanin Tapped as Interim Chief"),
      desc = _("Following the arrest of Rex Helmer, former Anecu deputy governor Elene Jouvanin will be sworn in today. She will serve out the term as governor.")
   },
   {
      tag = N_("FLF Terrorist Trial Ends"),
      desc = _("FLF Terrorist Trial ended this cycle with an unsurprising death sentence for all five members of the Nor spaceport bombing. Execution is scheduled in 10 periods.")
   },
   {
      tag = N_("New Challenges for New Times"),
      desc = _("The Dvaered council after a unanimous ruling decided to increase patrols in Dvaered space due to the recent uprising in FLF terrorism. The new measure is expected to start within the next cycle.")
   },
   --[[
      Human interest.
   --]]
   {
      tag = N_("Sirius Weaker Than Ever"),
      desc = _("This cycle breaks the negative record for fewest pilgrims to Mutris since the formation of House Sirius. This weakness is yet another sign that House Dvaered must increase patrols on the border and into Sirius space.") 
   }
}

articles["Goddard"] = {
   --[[
      Science and technology
   --]]
   {
      tag = N_("Goddard: Raising the Bar"),
      desc = _("Many new scientists are being contracted by House Goddard to investigate possible improvements. This new strategy will increase the gap with the competing ship fabricators.")
   },
   --[[
      Business
   --]]
   {
      tag = N_("Goddard Earnings on the Rise"),
      desc = _("House Goddard has once again increased its earnings. \"Our investment in technology and quality has paid off,\" said Kari Baker of House Goddard's marketing bureau.")
   },
   {
      tag = N_("Goddard Awarded Best Ship"),
      desc = _("Once again the Goddard Battlecruiser was awarded the Best Overall Ship prize by the Dvaered Armada's annual Ship Awards. \"Very few ships have reliability like the Goddard,\" said Lord Warthon upon receiving the award on behalf of House Goddard.")
   },
   {
      tag = N_("Aerosys Earnings Drop"),
      desc = _("The spaceways may swarm with Hyena-model craft, but today Aerosys recorded another quarterly loss. The company is investigating the discrepancy between the popularity of the craft and its sales figures.")
   },
   {
      tag = N_("Aerosys Victim of Pirate Manufacturing"),
      desc = _("The ship manufacturer has released a study indicating its signature Hyena model is being produced by a hidden system of unlicensed manufacturers.")
   },
   {
      tag = N_("Melendez CEO on Strategy"),
      desc = _("The Chief Executive Officer of ship maker Melendez Corp. thinks manufacturers should follow his company's lead in keeping costs down and producing for the mass market.")
   },
   {
      tag = N_("The Goddard Exception"),
      desc = _("Why has a community with more expertise sailing than flying produced the Empire's elite civilian spacecraft?  Lord Warthon says the secret lies in his family's hands-on tradition of leadership.")
   },
   {
      tag = N_("Sneak Peek: the Kestrel"),
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
      tag = N_("Trade Meeting at Lorelei"),
      desc = _("Lorelei, in the Churchill system, is the latest world to host major trade negotiations between the Fyrra and the Space Traders Guild. The Fyrra Arch-Canter has indicated that opening up trade routes is a major goal.")
   },
   --[[
      Politics
   --]]
   {
      tag = N_("Dvaered extorting pilgrims"),
      desc = _("Recent pilgrims headed to Mutris have been telling stories of extortion and violations by Dvaered operators. Dvaered Warlord Kra'tok claims that these are \"figments of the touched's imagination\". Official complaints have been made to the Emperor.")
   },
   --[[
      Human interest.
   --]]
   {
      tag = N_("Words of Tranquility"),
      desc = _("We welcome many new Touched who have recently begun ministering to the Shaira echelon after their long pilgrimage on Mutris. House Sirius is still a refugee for the orphans lost in this Universe.")
   },
}

articles["Pirate"] = {
   --[[
      Science and technology
   --]]
   {
      tag = N_("Skull and Bones Improving"),
      desc = _("The technology behind Skull and Bones is advancing. Not only do they steal ships, but they improve on the original design. \"This gives us pirates an edge against the injustice of the Empire,\" says Millicent Felecia Black, lead Skull and Bones engineer.")
   },
   --[[
      Business
   --]]
   {
      tag = N_("Draconis Favorite Plundering Space"),
      desc = _("Draconis has recently passed Delta Pavonis in the pirate polls as the most favored plundering space. The abundance of traders and high interference make it an excellent place to get some fast credits.")
   },
   {
      tag = N_("New Ships for Skull and Bones"),
      desc = _("The Skull and Bones was able to extract a few dozen high quality vessels from Caladan warehouses under the nose of the Empire. These ships will help keep production high and booming.")
   },
   --[[
      Politics
   --]]
   {
      tag = N_("Emperor Weaker Than Ever"),
      desc = _("Recent actions demonstrate the inefficiency and weakness of the Emperor. One of the last irrational decisions left Eridani without a defense fleet to protect the traders. It's a great time to be a pirate.")
   },
   --[[
      Human interest.
   --]]
   {
      tag = N_("Cats in New Haven"),
      desc = _("An explosion in the cat population of New Haven has created an adoption campaign with the slogan, \"Pirate Cats, for those lonely space trips.\". Is your space vessel full of vermin? Adopt a cat today!")
   },
}

articles["Empire"] = {
   --[[
      Science and technology
   --]]
   {
      tag = N_("Terraforming Emperor's Fist"),
      desc = _("New bleeding-edge terraforming techniques to be tried on Emperor's Fist. Studies show that these techniques could speed up the terraforming process by as much as 40%.")
   },
   {
      tag = N_("Bees Introduced to Emperor's Fist"),
      desc = _("As they prepare the gardens of the future Imperial compound, entomologists have established the first colony of the Earth insects on the planet formerly known as G Scorpeii 5.")
   },
   --[[
      Business
   --]]
   {
      tag = N_("Empire Keeping Traders Safe"),
      desc = _("Recent studies show that reports of piracy on Trader vessels have gone down by up to 40% in some sectors. This is a demonstration of the Empire's commitment to eradicating piracy.")
   },
   {
      tag = N_("Nexus Contract Finalised"),
      desc = _("The Empire agreed to terms with shipbuilder Nexus for a new generation of military craft. The deal extends the partnership with the government for another 10 cycles.")
   },
   --[[
      Politics
   --]]
   {
      tag = N_("New Empire Recruits"),
      desc = _("Emperor's recruiting strategy a success. Many new soldiers joining the Empire Armada. \"We haven't had such a successful campaign in ages!\" - Raid Steele, spokesman for recruiting campaign.")
   },
   {
      tag = N_("Governor Helmer Jailed"),
      desc = _("Imperial Auditors arrested governor Rex Helmer of Anecu on charges of corruption. He has been removed from office and transported to a holding facility awaiting trial.")
   },
   {
      tag = N_("Imperial Council Opens Doors"),
      desc = _("The supreme advisory body invited undergraduates from six top schools to sit in on a day's deliberations. Topics required biodiversity strategy.")
   },
   --[[
      Human interest.
   --]]
   {
      tag = N_("New Cat in the Imperial Family"),
      desc = _("The Emperor's daughter was recently gifted a cat. Cat could be named \"Snuggles\" and seems to be all white.")
   },
   {
      tag = N_("Emperor's Aid Gets Hitched"),
      desc = _("Imperial secretary Karil Lorenze married long time fiancee Rachid Baouda in the future palace gardens on Emperor's Fist. His Eminence the Bishop of Bao performed the ceremony.")
   },
   {
      tag = N_("Remembering the Past"),
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
      tag = N_("Election on Caladan Marred by Fraud"),
      desc = _("As many as two of every hundred votes counted after the recent polling decaperiod may be falsified, an ombudsman reports. The opposition party demanded the election be annulled.")
   },
   {
      tag = N_("Empire Relies on Prison Labour"),
      desc = _("A recent report by the Soromid House Ways and Means Committee suggests infrastructure may be too dependent the on the incarcerated population.")
   },
   {
      tag = N_("Imperial Oversight Scandal"),
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

econ_title = _("Current Market Prices")
econ_header = _("\n\n%s in %s")
econ_desc_part = _("\n%s: %s")


-- Return an economy article based on the given commodity name, planet
-- name, and number of credits. Wrapper for gettext.ngettext.
function get_econ_article( commod_name, plnt_name, credits )
   local econ_articles = {
      {
         title = _("Unfortunate Merchant Goes Bankrupt"),
         desc = gettext.ngettext(
            "A merchant was forced into bankruptcy due to a badly timed trade of %s on %s. \"I thought %s credit per tonne was a good deal, but it turns out I should have waited,\" the merchant said.",
            "A merchant was forced into bankruptcy due to a badly timed trade of %s on %s. \"I thought %s credits per tonne was a good deal, but it turns out I should have waited,\" the merchant said.",
            credits )
      },
      {
         title = _("Shipping Company Goes Out of Business"),
         desc = gettext.ngettext(
            "A small shipping business failed just this decaperiod. While it was already failing, what finally put the company under was a poorly-timed trade of %s on %s for %s credit per tonne. \"It was poor executive decision,\" one analyst asserts. \"Patience is key when trading, and it's clear that the owner of this company didn't have enough of that.\"",
            "A small shipping business failed just this decaperiod. While it was already failing, what finally put the company under was a poorly-timed trade of %s on %s for %s credits per tonne. \"It was poor executive decision,\" one analyst asserts. \"Patience is key when trading, and it's clear that the owner of this company didn't have enough of that.\"",
            credits )
      },
      {
         title = _("Interview with an Economist"),
         desc = gettext.ngettext(
            "One of the galaxy's foremost experts on economics gives an interview explaining our modern economy. \"We actually have a pretty good understanding of how the economy works. For example, we were able to predict what the price of %s on %s would reach very accurately; the actual price reached was %s credit per tonne, which we were only off by about 15%%. Over time, we hope to lower that margin of error to as little as 2%%.\"",
            "One of the galaxy's foremost experts on economics gives an interview explaining our modern economy. \"We actually have a pretty good understanding of how the economy works. For example, we were able to predict what the price of %s on %s would reach very accurately; the actual price reached was %s credits per tonne, which we were only off by about 15%%. Over time, we hope to lower that margin of error to as little as 2%%.\"",
            credits )
      },
      {
         title = _("Economist Describes Sinusoidal Economic Theory"),
         desc = gettext.ngettext(
            "A little-known economist discusses a controversial economic theory. \"When you look at the trends, it resembles a sine wave. For instance, the price of %s on %s is now %s credit per tonne, and it seems to return to that price with some regularity. We are working on developing a model to predict these curves more accurately.\" Other economists disagree, however, attributing these economists' results to chance.",
            "A little-known economist discusses a controversial economic theory. \"When you look at the trends, it resembles a sine wave. For instance, the price of %s on %s is now %s credits per tonne, and it seems to return to that price with some regularity. We are working on developing a model to predict these curves more accurately.\" Other economists disagree, however, attributing these economists' results to chance.",
            credits )
      },
      {
         title = _("Young Pilot Buys Their First Commodity"),
         desc = gettext.ngettext(
            "A young pilot has bought some %s as a way of breaking into the freelance piloting business. Born and raised on %s, where they bought their first commodity, they spoke with enthusiasm for the new career. \"You know, it's real exciting! Even on my home planet the price of %s credit per tonne isn't static, but when you look all around, there's so much price variation, so much potential for profit! I'm excited to finally get started.\"",
            "A young pilot has bought some %s as a way of breaking into the freelance piloting business. Born and raised on %s, where they bought their first commodity, they spoke with enthusiasm for the new career. \"You know, it's real exciting! Even on my home planet the price of %s credits per tonne isn't static, but when you look all around, there's so much price variation, so much potential for profit! I'm excited to finally get started.\"",
            credits )
      },
      {
         title = _("Corporate Scandal Rips Through the Galaxy"),
         desc = gettext.ngettext(
            "Economists are attributing the price of %s on %s to a scandal involving WarpTron Industries. Debates have ensued regarding whether or not the price, seen to be %s credit per tonne, will go up, down, or remain the same this time.",
            "Economists are attributing the price of %s on %s to a scandal involving WarpTron Industries. Debates have ensued regarding whether or not the price, seen to be %s credits per tonne, will go up, down, or remain the same this time.",
            credits )
      },
      {
         title = _("Commodity Trading Likened to Gambling"),
         desc = gettext.ngettext(
            "In a controversial statement, one activist has likened commodity trading to gambling. \"It's legalized gambling, plain and simple! Right now the price of %s on %s is %s credit per tonne, for example, but everyone knows the price fluctuates. Tomorrow it could be lower, or it could be higher. Who knows? Frankly, it is my firm opinion that this 'commodity trading' is self-destructive and needs to stop.\"",
            "In a controversial statement, one activist has likened commodity trading to gambling. \"It's legalized gambling, plain and simple! Right now the price of %s on %s is %s credits per tonne, for example, but everyone knows the price fluctuates. Tomorrow it could be lower, or it could be higher. Who knows? Frankly, it is my firm opinion that this 'commodity trading' is self-destructive and needs to stop.\"",
            credits )
      },
      {
         title = _("Leadership Decision Disrupts Prices"),
         desc = gettext.ngettext(
            "The price of %s was jeopardized on %s today when the local government passed a controversial law, bringing it to %s credit per tonne. Protests have erupted demanding a repeal of the law so that the economy can stabilize.",
            "The price of %s was jeopardized on %s today when the local government passed a controversial law, bringing it to %s credits per tonne. Protests have erupted demanding a repeal of the law so that the economy can stabilize.",
            credits )
      },
      {
         title = _("Five Cycle Old Child Starts Commodity Trading"),
         desc = gettext.ngettext(
            "A child no more than five cycles old has started commodity trading early, buying 1 tonne of %s. A native of %s, she explained that she has a keen interest in the economy and wishes to be a space trader some day. \"I bought it for %s credit, but it goes up and down, so if you time it right, you can make more money! My mom is a trader too and I want to be just like her.\"",
            "A child no more than five cycles old has started commodity trading early, buying 1 tonne of %s. A native of %s, she explained that she has a keen interest in the economy and wishes to be a space trader some day. \"I bought it for %s credits, but it goes up and down, so if you time it right, you can make more money! My mom is a trader too and I want to be just like her.\"",
            credits )
      },
   }

   local i = rnd.rnd( 1, #econ_articles )
   local title = econ_articles[i]["title"]
   local desc = econ_articles[i]["desc"]:format(
      _(commod_name), _(plnt_name), numstring(credits) )

   return title, desc
end


-- create news
function create()
   local f = planet.cur():faction()
   if f == nil then evt.finish(false) end
   local my_faction = f:nameRaw()

   add_header( my_faction )
   add_article( my_faction )
   add_article( "Generic" )
   add_econ_article()
end


function add_header( my_faction )
   -- Remove old headers (we only want one at a time)
   for i, article in ipairs( news.get( "header" ) ) do
      article:rm()
   end

   if header_table[my_faction] == nil then
      warn( string.format(
            _('News: Faction \'%s\' does not have entry in faction table!'),
            my_faction) )
      my_faction = 'Generic'
   end

   local cur_t = time.get()
   local header = header_table[my_faction]
   local desc = greet_table[my_faction][ rnd.rnd( 1, #greet_table[my_faction] ) ]
   local a = news.add(
         my_faction, header, desc, cur_t + time.create( 0, 0, 1 ),
            cur_t + time.create( 5000, 0, 0 ) )
   a:bind( "header" )
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
   local title = alst[i]["title"] or _(tag)
   local desc = alst[i]["desc"]

   if #news.get( tag ) > 0 then
      return
   end

   local exp = time.get() + time.create( 0, 50, 5000 * rnd.sigma() )
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
         local exp = time.get() + time.create( 0, 20, 5000 * rnd.sigma() )
         local commchoices = p:commoditiesSold()
         local commod = commchoices[ rnd.rnd( 1, #commchoices ) ]
         local price = commod:priceAtTime( p, pd )
         local title, desc = get_econ_article( commod:name(), p:name(), price )
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
