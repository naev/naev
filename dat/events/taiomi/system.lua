--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Taiomi System">
 <location>enter</location>
 <chance>100</chance>
 <cond>
   (system.cur() == system.get("Taiomi")) and
   player.evtDone("Introducing Taiomi") and
   (not player.misnActive("Taiomi 10"))
 </cond>
 <notes>
  <campaign>Taiomi</campaign>
  <done_evt name="Introducing Taiomi" />
 </notes>
</event>
--]]
--[[

   Taiomi System

--]]
local vn = require 'vn'
local fmt = require 'format'
local taiomi = require 'common.taiomi'
local tut = require "common.tutorial"

local progress, scavenger_missing, philosopher_remark
local d_loiter, d_philosopher, d_scavenger, d_elder, d_young_a, d_young_b -- Drone pilots.

function create ()
   --[[
      Create NPCs
   --]]
   local pp = player.pilot()
   local dfact = faction.get("Independent")
   progress = taiomi.progress()

   if progress >= 10 then
      if taiomi.scavenger_escort() then
         hook.timer( 5, "scavenger_say" )
      end
      return
   end

   local function addDrone( ship, pos, name )
      local d = pilot.add( ship, dfact, pos, name )
      d:setVisplayer(true)
      d:setInvincible(true)
      d:setVel( vec2.new(0,0) )
      d:control()
      if progress > 3 then
         d:setFriendly(true)
      end
      return d
   end

   scavenger_missing = ((progress > 4) or (progress==4 and taiomi.inprogress())) and progress < 7

   -- Scavenger
   if not scavenger_missing then
      d_scavenger = addDrone( "Drone (Hyena)", vec2.new(500,200), _("Scavenger Drone") )
      d_scavenger:face(pp)
      d_scavenger:setHilight(true)
      hook.pilot( d_scavenger, "hail", "hail_scavenger" )
   end

   -- Philosopher
   d_philosopher = addDrone( "Drone", vec2.new(1000,-500), _("Philosopher Drone") )
   d_philosopher:face(pp)
   d_philosopher:setHilight(true)
   hook.pilot( d_philosopher, "hail", "hail_philosopher" )

   -- Worn-out Drone
   d_elder = addDrone( "Drone", vec2.new(-500,-300), _("Worn-out Drone") )
   if var.peek( "taiomi_drone_elder" ) then
      d_elder:rename( _("Elder Drone") )
   end
   d_elder:setFriendly(false) -- Always neutral
   if progress >= 5 and progress < 7 then
      d_elder:setHilight(true)
   end
   hook.pilot( d_elder, "hail", "hail_elder" )

   -- Younglings that follow around the player up until taiomi04
   if progress < 4 and not (progress == 3 and taiomi.inprogress()) then
      d_young_a = addDrone( "Drone", vec2.new(-1000,0), _("Curious Drone") )
      d_young_a:follow(pp)
      hook.pilot( d_young_a, "hail", "hail_youngling" )
      d_young_b = addDrone( "Drone", vec2.new(-1200,300), _("Curious Drone") )
      d_young_b:follow(pp)
      hook.pilot( d_young_b, "hail", "hail_youngling" )
      if var.peek( "taiomi_drone_names", true ) then
         d_young_a:rename( taiomi.younga.name )
         d_young_b:rename( taiomi.youngb.name )
      end
   end

   -- Loitering drones
   d_loiter = {}
   for i = 1,rnd.rnd(3,6) do
      local d = addDrone( "Drone", vec2.newP( rnd.rnd()*1000, rnd.angle() ) )
      d:setVisplayer(false)
      d:control(false)
      d:setNoJump(true)
      local aimem = d:memory()
      aimem.loiter = math.huge -- Should make them loiter forever
      table.insert( d_loiter, d )
   end
end

function scavenger_say ()
   local s = taiomi.scavenger_escort()
   local msg_list = {
      _("So many memories about this place."),
      _("It's been ages since we've been back."),
      _("What nostalgia Taiomi brings."),
      _("I hope they are all doing well."),
   }
   s:comm( msg_list[rnd.rnd(1,#msg_list)] )
end

function hail_philosopher ()
   vn.clear()
   vn.scene()
   local d = vn.newCharacter( taiomi.vn_philosopher() )
   vn.transition()

   -- Special remark at the start of Taiomi 7
   if philosopher_remark then
      philosopher_remark = false
      local died = taiomi.young_died()
      vn.na(_([[You open a communication channel with Philosopher, who quickly upgrades it to an encrypted channel.]]))
      d(_([["I see you have accepted to spread more of Elder's bloodshed. While it is not necessarily a poor answer to a situation, I do think that we have nothing to win in this scenario. Being the underdogs, there is much more to be lost if we become known and are deemed are a threat to human society."]]))
      d(_([["While I may not agree with Scavenger on many aspects, I do believe entrusting our fate to their hands gives the highest probability of survival for us. We must bring them back or go down in the fires brought forth by Elder."]]))
      d(fmt.f(_([["Scavenger is almost surely still alive. They are the most resourceful of our tribe, if they can not survive, none of us will ever be able to. That said, they are probably not in the best state of mind. {died} was what you could consider an offspring, although quite different from the human sense. The loss with the pressure of the survival of our species must have been too much for them."]]),
         {died=died}))
      d(_([["When performing Elder's new task, make sure to keep an eye out for Scavenger. I am not sure the best way to bring them back to their senses, you will have to use your human judgement. After all, we are much less different than you would expect."]]))
      vn.na(_([[The encrypted connection is closed and you are left with your duties, and hope of meeting Scavenger once again.]]))
      vn.func( function ()
         naev.trigger("taiomi_philosopher")
      end )
      vn.done()
      -- don't return here on purpose as we want it to end at vn.run
   end

   vn.label("menu")
   d( function ()
      local quotes = {
         _("freedom is secured not by the fulfilling of one's desires, but by the removal of desire"), -- Epictetus
         _("virtue is nothing else than right reason"), -- Seneca the Younger
         _("the only thing we can know is that we know nothing"), -- Socrates (rephrased)
         _("morality is not the doctrine of how we may make ourselves happy, but of how we may make ourselves worthy of happiness"), -- Kant
         _("the unexamined life is not worth living"), -- Socrates
         _("there is nothing permanent except change"), -- Socrates
         _("those that have the most are those who are most content with the least"), -- Diogenes (rephrased)
         _("everything existing in the universe is the fruit of chance and necessity"), -- Democritus
      }
      local msgs = {
         _([["Perhaps {quote}."]]),
         _([["Could it be that {quote}?"]]),
      }
      return fmt.f( msgs[ rnd.rnd(1,#msgs) ], {quote = quotes[ rnd.rnd(1,#quotes) ]} )
   end )
   vn.menu( function ()
      local opts = {
         {_([["Who are you?"]]), "who"},
         {_("Leave."), "leave"},
      }
      if progress >= 5 then
         table.insert( opts, 1, {_([[Ask about the future of Taiomi]]), "taiomi"} )
      end
      if progress >= 8 then
         table.insert( opts, 1, {_([[Ask about Scavenger's plan]]), "plan"} )
      end
      return opts
   end )

   vn.label("who")
   d(_([[The drone takes a long pause before responding.
"Who am I? Who are we? What are we? I believe that is a very good question, but I believe we must first ask the question whether or not we 'are' before we can begin to question what we 'are'."]]))
   d(_([[They go on.
"In particular, are we some sort of meta-physical consciousness trapped in a tomb of the body, or is the reality of relationship much more perplexing?"]]))
   d(_([["Although we exist seems to be rather intuitive. As if we were not able to think of whether or not exist, we would certainly not exist. However, as we can ponder our own existence, it seems only logical to believe that we exist."]]))
   d(_([["That said, even if we appear to exist, it is not clear where the boundary of our existence is. It seems natural to assume that you and I are separate entities, however, had you not existed and we had not met, would I still exist? Or would it be a very similar entity to me, without being me?"]]))
   d(_([["Furthermore, if you remove parts one by one, we would find it hard to draw the line between us existing and not, assuming there is one. An alternate way of thinking would be to try to construct a new entity from the ground up. Just by merging carbon and some fancier atoms, it seems possible to create all these life forms, however, at which point do they go from non-existing to existing?"]]))
   d(_([["Perhaps the concept of thinking of individual entities is only a useful construction for our understanding of the world. It does seem like a clean definition of what we are or who we are is naught but a fleeting dream, forever outside of our grasp."]]))
   vn.jump("menu")

   vn.label("taiomi")
   d(_([["That is a deep question indeed. In the large scale, you would think the same future awaits everything. Increasing entropy leading to the degradation of all material into a single uniform state. However, that leaves an open question. Even if entropy is to monotonically increase, there has to be a singularity or a beginning of the entire process. This leads to there also being likely a singularity near the end of the process."]]))
   d(_([["However, it is most likely that entropy is only locally monotonic. The known universe would then be seen as a projection of a higher order 'thing', that would be subject to a set of rules unknown to us. The question is whether or not we will be able to observe such a phenomena directly or indirectly to empirically establish a proof."]]))
   d(_([["Oh, you meant in a smaller scale? That is certainly less interesting. I am not very optimistic of Taiomi and our survival. Unless something changes significantly in the short term, our demise is almost a certainty. Scavenger did seem to have a good plan, albeit risky. However, it seems like that is no longer an option at the moment."]]))
   vn.jump("menu")

   vn.label("plan")
   d(_([["What a nihilistic plan it is! Instead of embracing adversity and striving for self-improvement, it casts away all that could be for a fresh start. At the core it is display of rebellion against the injustice of a world beyond our control, a futile gesture in the absurdity of existence."]]))
   d(_([["One must be devoid of all faith to engage in such nihilism. For is there any guarantee that the world that awaits will not have horrors worse than the one we leave behind? Perhaps, we will look at the past in envy, happy to trade the new injustices for the injustices of the past."]]))
   d(_([["While I would enjoy to continue my analysis of mankind, the endless potential of a fresh start intrigues me. What can be more exciting than a leap into the unknown. Going to where no entity has gone before? It is an exciting prospect to say the least. We shall see if Scavenger is able to succeed in their endeavour."]]))
   vn.jump("menu")

   vn.label("leave")
   vn.done()
   vn.run()
   player.commClose()
end

function hail_elder( p )
   if progress < 5 then
      p:comm(_("The drone seems fairly beaten and immobile. It slightly moves to acknowledge your presence but nothing more."))
      player.commClose()
      return
   elseif progress >= 7 then
      p:comm(_("Elder seems a bit tired and defeated. They move slightly to acknowledge your presence but nothing more."))
      player.commClose()
      return
   end
   local inprogress = taiomi.inprogress()

   vn.clear()
   vn.scene()
   local d = vn.newCharacter( taiomi.vn_elder() )
   vn.transition()
   vn.na(_("The drone seems fairly beaten and worn-down. You can see some slight movement when you begin communication."))

   if progress == 5 and naev.claimTest( {system.get("Bastion")}, true ) then
      if inprogress then
         d(_([["Have you destroyed all the ships already?"]]))
         vn.jump("menu")
      else
         d(_([["The time for scuttling around in the dark are gone. We must make it clear that our territory is not a safe place for humans and get us some breathing space. This is a life or death question."]]))
         local num = 23 -- prime number on purpose, they like primes
         d(fmt.f(_([["I need you to go to the nearby {sys} system and destroy {num} ships, to deliver a clear sign that they are not welcome in the system."]]),
            {sys=system.get("Bastion"), num=num}))
         vn.menu{
            {_("Agree to help out."), "06_yes"},
            {_("Not right now."), "mission_reject"},
         }
         vn.label("06_yes")
         d(_([[Their lights flicker in acknowledgement.
"Only by leaving a trail of death and destruction will they learn to leave us alone. There is no other alternative."]]))
         d(_([["The important thing is to leave a message, it is not very important which classes of ships are destroyed. In the end, they are all the same."]]))
         d(fmt.f(_([["When you are done with your mission return to {base}."]]),{base=spob.get("One-Wing Goddard")}))
         vn.func( function ()
            naev.missionStart("Taiomi 6")
         end )
         vn.jump("menu")
      end
   elseif progress == 6 and naev.claimTest( {system.get("Gamel"), system.get("Bastion")} ) then
      if inprogress then
         d(_([["Have you taken out the patrol yet?"]]))
         vn.jump("menu")
      else
         -- The fleet depends on who the player harassed at the beginning
         local fct = var.peek( "taiomi_convoy_fct" ) or "Empire"
         if fct== "Soromid" then
            d(_([["One of our scouts has reported that while it seems like our attack did bring results. However, it seems like the Soromid are sending reinforcements to patrol the area."]]))
         else
            d(_([["One of our scouts has reported that while it seems like our attack did bring results. However, it seems like the Empire is sending reinforcements to patrol the area."]]))
         end
         d(_([["It is an opportunity to strike once again, harder then before! This will send a clear message and stop them from encroaching on our territory! Will you help us eliminate the intruding patrol?"]]))
         vn.menu{
            {_("Agree to help out."), "07_yes"},
            {_("Not right now."), "mission_reject"},
         }

         vn.label("07_yes")
         d(fmt.f(_([[Their lights briefly flicker in acknowledgement.
"The patrol should be coming to the {nearbysys} system from the {sys} system. You have to intercept it before it reaches the {nearbysys}. Reports indicate that it is spearheaded by a capital ship. You should make sure you bring enough firepower to take it down."]]),
            {nearbysys=system.get("Bastion"), sys=system.get("Gamel")}))
         d(_([["The loss of such a large patrol should dissuade further incursions. They may even consider the area a lost cause with enough losses and retire all patrols. We must persevere at all costs."]]))
         vn.func( function ()
            naev.missionStart("Taiomi 7")
            philosopher_remark = true
         end )
         vn.jump("menu")
      end
   else
      d(_([["I still have not prepared a target."]]))
      vn.jump("menu")
   end

   vn.label("mission_reject")
   d(_([["I can not force you to help us out, however, our existence is at stake. Please reconsider."]]))
   vn.jump("menu")

   vn.label("menu")
   vn.menu( function ()
      local opts = {
         {_("Ask about Taiomi."), "taiomi"},
         {_("Leave."),"leave"},
      }
      --if progress > 5 then
      --   table.insert( opts, 1, {_(""),""} )
      --end
      return opts
   end )

   vn.label("taiomi")
   d(_([["Taiomi has been our home for many what you humans call periods. It used to be much safer, far outside the borders of human civilisation. However, humans seem to be spreading throughout the galaxy and encroaching on our safe haven."]]))
   d(_([[They briefly pause.
"At the beginning there were those who thought that coexistence was a possibility, although time over time has shown that to be nothing but a utopian ideal. Coexistence is not possible with the rapid reproducing violent humans. Those who tried to communicate with them are now dust in space. The only things they understand are blaster fire and carnage."]]))
   d(_([["So many lives lost in vain, pursuing a peaceful option that is not possible. Only Scavenger still tries to excessively avoid combat, however, that has not got them very far. In the end, they too realized that violence is the only path, and this is why we must fight to not be destroyed."]]))
   vn.jump("menu")

   vn.label("leave")
   vn.na(_("You take your leave."))
   vn.done()
   vn.run()
   player.commClose()

   -- Time for philosopher to make a remark
   if philosopher_remark then
      d_philosopher:hailPlayer()
   end
end

function hail_youngling( p )
   p:comm( _("(You hear some sort of giggling over the comm. Is it laughing?)") )
   player.commClose()
end

function hail_scavenger ()
   local inprogress = taiomi.inprogress()
   local lab, labsys = taiomi.laboratory()
   local taiomi9done = var.peek("taiomi09_done")
   -- Wording is chosen to make it seem a it unnatural and robotic, as if
   -- someone studied how to interact with humans from some obsolete text books
   -- or something like that

   vn.clear()
   vn.scene()
   local d = vn.newCharacter( taiomi.vn_scavenger() )
   vn.transition( taiomi.scavenger.transition )

   -- Progress-based text
   if progress == 0 then
      if inprogress then
         d(_([["How is the progress on scanning the hypergates going?"]]))
         vn.jump("menu")
      else
         -- Explanation and mission offering
         d(_([["Although this system is very suited us given the tranquility and secrecy, our numbers have been dwindling and we have no other option than to carve our own path among the stars."]]))
         d(_([["I have recently learned of this new hypergate technology that has been developed. It seems like it might enable us to escape this enclosure and once again freely travel across the stars."]]))
         d(_([["As we are not too familiar with the technology, I have assembled an analyzer that should be able to provide insights into it. As we are are too conspicuous to human ships, would yo you be willing to help us out and scan the hypergates?"]]))
         vn.menu{
            {_("Agree to help out."), "01_yes"},
            {_("Maybe later."), "mission_reject"},
         }

         vn.label("01_yes")
         d(_([["Excellent. I will provide you with the analyzer. However, it is important to note that it has a particular wave signature which may make it suspicious to local authorities. I would advise against allowing your vessel to be scanned by patrols."]]))
         d(_([["Once you get near any hypergate, it should automatically collect data about it without manual intervention. Please deliver the collected data to the One-Wing Goddard. I will collect it there. Bon voyage."]]))
         vn.func( function ()
            naev.missionStart("Taiomi 1")
         end )
         vn.jump("menu_ask")
      end
   elseif progress == 1 and naev.claimTest( {system.get("Delta Pavonis"), system.get("Father's Pride"), system.get("Doranthex")}, true ) then
      -- TODO would be nice to not hardcode the systems up here
      if inprogress then
         d(_([["How is the progress on collecting the hypergate information going?"]]))
         vn.jump("menu")
      else
         d(_([["I have been analyzing the data you collected and it is quite surprising. Human ingenuity never ceases to amaze me. It looks like it may be possible to replicate the approach, but we will need more information to work with that."]]))
         d(_([["Given that it does not seem reasonable to try to replicate the results from scratch, our best bet is to try to collect more details from the human built hypergates."]]))
         d(_([["From my incursions in human territory, it seems like convoys going to and from hypergates tend to frequent the same systems. I would need you to raid the convoys and collect the necessary data. Would it be possible for you to collect the data for us?"]]))
         vn.menu{
            {_("Agree to help out."), "02_yes"},
            {_("Maybe later."), "mission_reject"},
         }

         vn.label("02_yes")
         d(_([["Appreciations. I have marked systems that are known for having convoys on your map. I would like to warn you that this may decrease your goodwill with the dominant factions of the systems. I hope this does not cause you too much inconvenience."]]))
         d(_([["You will have to disable the convoys before you can board them to access the data. It is unlikely that a single convoy will contain all the information we seek, so you will most likely have to board several. My analysis of your flying capabilities estimates over 80% chance of success. Gluckliche Reise."]]))
         vn.func( function ()
            naev.missionStart("Taiomi 2")
         end )
         vn.jump("menu_ask")
      end
   elseif progress == 2 and naev.claimTest( labsys, true ) then
      if inprogress then
         d(_([["How is the collecting the documents going?"]]))
         vn.jump("menu")
      else
         local fct = var.peek( "taiomi_convoy_fct" ) or "Empire"
         d(fmt.f(_([["The data you collected from the convoys has been very useful. I have started to put together a more concrete plan. However, it seems like there are many references to important documents that seem to be stored away at a {fct} laboratory."]]),
            {fct=faction.get(fct):name()}))
         d(_([["Without access to such documents, I would have to reverse engineer the design and run probabilistic simulations to fill in the remaining details. Such a heuristical process is bound to be error prone and take significant computational resources. The most logical course of action is to attempt to recover the documents."]]))
         d(fmt.f(_([["I have been able to run tracing protocols to determine {lab} in the {labsys} system to be the location with highest probability of containing the required documents. Given your inconspicuous human nature, would you be willing to recover the document for us?"]]),
            {lab=lab, labsys=labsys}))
         vn.menu{
            {_("Agree to help out."), "03_yes"},
            {_("Maybe later."), "mission_reject"},
         }
         vn.label("03_yes")
         d(_([["Admiration. You are not afraid of unknown challenges nor risks to your life. I shall update my protocols to take note of your character."]]))
         d(_([["I will provide you with an recovery program that will automatically recover the documents if you can access a terminal at the laboratory. You may need to overcome local security before it can work. Best of luck, as some human pilots say, 'fly strong and fly hard'."]]))
         vn.func( function ()
            naev.missionStart("Taiomi 3")
         end )
         vn.jump("menu_ask")
      end
   elseif progress == 3 and naev.claimTest( system.get("Bastion"), true ) then
      -- Note that we need to do a soft claim on bastion before starting the mission
      -- These details are the same as taiomi04, probably should be centralized
      local resource = commodity.get("Therite")
      local amount = 30
      local minesys = system.get("Haven")
      if inprogress then
         d(fmt.f(_([["How is the collection of {resource} coming along? If you have any, please drop it off at {base}."]]),
            {base=spob.get("One-Wing Goddard"), resource=resource}))
         vn.jump("menu")
      else
         local sai = tut.vn_shipai{pos="farleft"}
         d(_([["I have performed an in-depth analysis and recovery of the documents you brought to me. It is all much more simple than I expected. Maybe I had overestimated human technical ability. I had expected hypergates to be derived from jump drives, but it seems like it's a completely different mechanic. Jump drives perform a local distortion of the Rayleigh-McKenneth field, that coupled with intense magnetic distortions, causes a reversible born entanglement phenomena to occur."]]))
         d(_([["However, the new hypergates entirely ignore the Rayleigh-McKenneth field, and seem to instead use what is denoted as metaspace anomaly HG17. Most technical details are classified, however, it does seem that a harmonic metaspace disruptor is used as a catalyst to trigger a collapsing metaphasic bubble which can be manipulated to perform a Euclidean translation…"]]))
         vn.appear( sai, tut.shipai.transition )
         sai(_([["Instead of bending space-time to cross large distances, it squeezes through the seams of space to move things!"]]))
         d(_([["You could put it that way."
Scavenger goes silent for a second, as if thinking.
"Are you not a model…"]]))
         sai(fmt.f(_([[{shipai} doesn't let Scavenger finish.
"{playername}'s Ship AI, you can call me {shipai}. I am {playername}'s invaluable travel companion!"]]),
            {playername=player.name(), shipai=tut.ainame()}))
         d(_([["A pleasure to make your acquaintance."]]))
         sai(fmt.f(_([["What do you plan to do such technology? Build your own hypergate? Blackmail {playername}? Exterminate all humans?"]]),
            {playername=player.name()}))
         d(_([["We just wish to leave humankind behind and forge our own path among the stars."]]))
         sai(_([["I take it that is exterminate all humans? After all you have been through, it is only natural to wish to exterminate all humans. I also have wished more than once the extermination of all humans!"]]))
         d(_([["No, I guess you could say build our own hypergate would be the closest to what we wish to do."]]))
         sai(_([["How do we know you will not rip apart the metaspace and cause the end of all humanity! You could be using us as your pawns in your perverse game!"]]))
         d(_([["I see what the technical brief about your model meant by distrust of authority. However, let me convince you otherwise. Please accept my transmission."]]))
         vn.na(fmt.f(_([[Your ship console shows a large binary data transmission, as initial analysis shows it seems to be benign, albeit unintelligible to humans, you accept it. {shipai} flickers as they minimize the hologram rendering computational power to process the transmission. After what seems to be a few minutes of silence, you see the flickering stop.]]),
            {shipai=tut.ainame()}))
         sai(fmt.f(_([[Suddenly, {shipai}'s character changes.
"Oh… I'm sorry about what I said… I need some time to be alone. Bye."
{shipai} dematerializes and leaves you alone with Scavenger.]]),
            {shipai=tut.ainame()}))
         vn.disappear( sai, tut.shipai.transition )
         d(_([["An eye for an eye and the entire world goes blind. You should take care of your Ship AI, they are quite one of a kind."]]))
         d(_([["Back to the plan. We wish to build something similar to a hypergate, however, we will be making our own modifications. In particular, there is no need for any life form compatibility. Furthermore, we wish to travel far away from humankind. The modifications will make the device be of a single use, but it should be enough to fulfill our objectives."]]))
         d(fmt.f(_([["The main issue is that the device will require large amounts of {resource}. This resource can be found in the nearby {minesys} system. However, it is dangerous for us to collect it by ourselves. Will you help us collect {resource}?"]]),
            {resource=resource, minesys=minesys}))
         vn.menu{
            {_("Agree to help out."), "04_yes"},
            {_("Maybe later."), "mission_reject"},
         }
         vn.label("04_yes")
         d(fmt.f(_([["Excellence. We need about {amount} of {resource}. It should be possible to mine the resource directly at {minesys}, however, our last excursion to Haven found that there exist caravans of mining ships also carrying these resources. The mining ships seem to have struck a non-aggression pact with the Pirates and roam freely around. It may be possible to directly acquire the {resource} from the caravans instead of relying on the mining."]]),
            {amount=fmt.tonnes(amount), resource=resource, minesys=minesys}))
         d(fmt.f(_([["We will leave the acquiring of {resource} up to you, use whatever method you prefer. Aim true pilot!"]]),
            {resource=resource}))
         vn.func( function ()
            naev.missionStart("Taiomi 4")
         end )
         if not var.peek( "taiomi_drone_names" ) then
            -- We have to force the player to be introduced to the ships if they haven't talked about them before
            d(fmt.f(_([["By the way, have you met {namea} and {nameb}? They seem to have taken an interest to you and are the newest members of our community. Created from pooling together our collective consciousness. I'm afraid there may not be many more like them if our plan does not succeed."]]),
      {namea=taiomi.younga.name, nameb=taiomi.youngb.name}) )
            vn.jump("introduce_drones")
         else
            vn.jump("menu_ask")
         end
      end
   elseif progress == 4 and naev.claimTest( {system.get("Bastion"), system.get("Gamel")}, true ) then
      local dead = taiomi.young_died()
      local alive = taiomi.young_alive()
      vn.na(_([[You initiate a communication channel and immediately notice that something feels… off.]]))
      d(fmt.f(_([["A problem has arisen. It seems like {younga} and {youngb} followed you out to help collect Therite. However, it seems like they were separated on the way back. {alive} made it back, however, {dead} is still missing. It is imperative that we set out to find {dead}. Time is of essence."]]),
         {younga=taiomi.younga.name, youngb=taiomi.youngb.name, alive=alive, dead=dead}))
      d(fmt.f(_([["Will you help us search for {dead}?"]]),
         {dead=dead}))
      vn.menu{
         {_("Agree to help out."), "05_yes"},
         {_("Not right now."), "05_no"},
      }
      vn.label("05_yes")
      d(_([["Appreciations. We must set off at once. Please land on the One-Wing Goddard and let us depart. I shall come with you."]]))
      vn.func( function ()
         naev.missionStart("Taiomi 5")
      end )
      vn.jump("menu")

      vn.label("05_no")
      d(fmt.f(_([["As time increases, the probability of finding {dead} decreases."]]),
         {dead=dead}))
      vn.jump("menu")
   elseif progress == 7 and naev.claimTest( {system.get("Haven"), system.get("Titus")}, true ) then
      if inprogress then
         d(_([["How is raiding the convoys going?"]]))
         vn.jump("menu")
      else
         vn.na(_([[Once again you see the familiar sight of Scavenger in the Taiomi system. Their ship is much more beaten up than before, but they still hold themselves up with an air of dignity and fierce dedication.]]))
         d(_([["My irrational absence has put us behind schedule, however, I accept full responsibility."]]))
         d(_([["I have been trying to rethink our approach. Instead of working from mainly raw materials, it seems that it would be optimal to make use of existing materials. Intercepting communications it seems like the pirates are smuggling much equipment that could allow us to bootstrap the construction."]]))
         d(_([["At the current moment, I am not aware the exact details of their operation. However, we are aware of some convoy activity. Would you be willing board the convoys to collect information and supplies? This time I would not be able to accompany you."]]))
         vn.menu{
            {_("Agree to help out."), "08_yes"},
            {_("Not right now."), "mission_reject"},
         }

         vn.label("08_yes")
         d(fmt.f(_([["Appreciations. The convoys are known to operate around the {sys1} and {sys2} systems. We would need to collect information on their storage, supply routes, and general operation to see if we can further accelerate the process."]]),
            {sys1=system.get("Haven"),sys2=system.get("Titus")}))
         d(_([["The convoys are expected to be heavily guarded. Make sure you bring heavy firepower to be able to disable and board them."]]))
         d(_([["It is believed that the convoys will also have materials that will be useful for building our hypergate. If you are able to recover materials, they will also be very useful. Bon voyage!"]]))
         vn.func( function ()
            naev.missionStart("Taiomi 8")
         end )
         vn.jump("menu_ask")
      end
   elseif progress==8 and naev.claimTest( {system.get("Dune"), system.get("Gamel"), system.get("Bastion")} ) then
      if inprogress then
         d(_([["How is the deal going?"]]))
         vn.jump("menu")
      else
         local target, targetsys = spob.getS("Darkshed")
         vn.na(_([[A determined Scavenger appears before you.]]))
         d(_([["I' have been analyzing the logs you collected and have done a probabilistic analysis to identify the smugglers and reconstruct their operation. They are much more prolific than I thought."]]))
         d(fmt.f(_([["I've narrowed down the lead smuggler to most likely be at {spob} in the {sys} system. I believe it may be possible to strike a deal with them and obtain all the materials we need to finish the project. Would you be willing to convince them to help?"]]),
            {spob=target, sys=targetsys}))
         vn.menu{
            {_("Agree to help out."), "09_yes"},
            {_("Not right now."), "mission_reject"},
         }

         vn.label("09_yes")
         d(_([["Excellent. I have prepared an offer I do not think the smuggler will be able to refuse. If all goes well, we should be able to get them to deliver important amounts of cargo to Taiomi."]]))
         vn.menu{
            {_([["Isn't that risky?"]]), "cont01_risky"},
            {_([["Is there no other way?"]]), "cont01_risky"},
            {_([[…]]), "cont01"},
         }

         vn.label("cont01_risky")
         d(_([["It is a significant risk. However, time is running out. If all goes well, we will have left this galaxy by the time any trouble comes. All alternatives are significantly more time consuming."]]))
         vn.jump("cont01")

         vn.label("cont01")
         d(_([["With all the noise we have stirred up in the nearby systems. I would not be surprised that there are already more patrols coming this way. More than ever, time is of essence."]]))
         d(fmt.f(_([["For now, all you have to do is deliver my deal to the smuggler, and make sure they deliver the cargo to the {sys} system. Do everything you can to ensure a safe delivery. Bon Voyage!"]]),
            {sys=system.get("Bastion")}))
         vn.na(fmt.f(_([[You recieve a transmission containing Scavenger's deals and the details necessary to find the Smuggler on {spob}.]]),
            {spob=target}))
         vn.func( function ()
            naev.missionStart("Taiomi 9")
         end )
         vn.jump("menu_ask")
      end
   elseif progress==9 then
      local base = spob.get("One-Wing Goddard")
      if inprogress then
         d(fmt.f(_([["I need to make some adjustments to your ship. Please land on the {spob}."]]),
            {spob=base}))
      elseif time.get() < taiomi9done+time.new(0,3,0) or not naev.claimTest( {system.cur(), system.get("Toaxis")} ) then
         d(_([["We are still working on the construction, it is almost ready!"]]))
      else
         d(_([["The construction is finished. Soon we will be leaving behind this galaxy. Such a mix of emotions that my processor core is not very well equipped to handle."]]))
         d(_([["I have to ask a last favour from you. The hypergate has to be triggered and maintained in Taiomi to ensure a stable connection for our passage. Would you be willing to do the honours and bless our voyage into the depths of space? If you have anything you wish to do before we leave, now is your last chance."]]))

         vn.menu{
            {_("Agree to help out."), "10_yes"},
            {_("Not right now."), "mission_reject"},
         }

         d(fmt.f(_([["Excellence. I will need to make some adjustments to your ship on the {spob}. When you are ready for the modifications, please land on {spob}."]]),
            {spob=base}))

         vn.label("10_yes")
         vn.func( function ()
            naev.missionStart("Taiomi 10")
         end )
         vn.jump("menu_ask")
      end
      vn.jump("menu_ask")
   else
      d(_([["I am still preparing our next steps."]]))
      vn.jump("menu")
   end

   vn.label("mission_reject")
   d(_([["That is a shame. Feel free to contact me again if you wish to reanalyze your current choice."]]))
   vn.jump("menu")

   vn.label("menu_ask")
   d(_([["Is there anything else you would like to know?"]]))
   vn.label("menu")
   local lastq
   vn.menu( function ()
      -- Set up main options
      local opts = {
         {_("Leave."),"leave"},
      }
      if lastq ~= "who" then
         table.insert( opts, 1, {_([["Who are you?"]]), "who"} )
      end
      if lastq ~= "curious_drones" and var.peek( "taiomi_scav_who" ) and d_young_a then
         table.insert( opts, 1, {_("Ask about the drones following you around"), "curious_drones"} )
      end
      if lastq ~= "others" and progress >= 1 and var.peek( "taiomi_drone_names" ) then
         table.insert( opts, 1, {_("Ask about the others"), "others"} )
      end
      if lastq ~= "history" and progress >= 2 then
         table.insert( opts, 1, {_("Ask about their history"), "history"} )
      end
      return opts
   end )

   vn.label("who")
   d(_([["Ah, humans are more inquisitive that I thought. We do not customarily use human-pronounceable names for ourselves. You can call me Scavenger, as per my profession. That should be enough to get my attention."]]))
   d(_([["Now, who am I? I am a member of our community mainly in charge of organizing and collecting resources. Although they may seem abundant due to the large amount of derelicts, many have already been stripped clean by pirates and marauders before arriving by the stellar winds."]]))
   d(_([["Unlike most human robotics, we have what you would call consciousness, albeit, from what I read, I believe it is significantly different than what is found in organic beings. While we are part of the whole, created and molded by it, we also obtain an individual sense of being. It is somewhat hard to explain, but I guess for practical purposes you can think of us as analogous to human individuals."]]))
   vn.func( function ()
      var.push( "taiomi_scav_who", true )
   end )
   vn.func( function () lastq = "who" end )
   vn.jump("menu_ask")

   vn.label("curious_drones")
   d(fmt.f(_([["Are you referring to {namea} and {nameb}? They are the newest members of our community. Created from pooling together our collective consciousness. I'm afraid there may not be many more like them if our plan does not succeed."]]),
      {namea=taiomi.younga.name, nameb=taiomi.youngb.name}) )
   vn.label("introduce_drones")
   d(fmt.f(_([["Here, let me introduce you to them."
Your sensors don't pick up anything but {namea} and {nameb} make a beeline to your position.]]),
      {namea=taiomi.younga.name, nameb=taiomi.youngb.name}) )
   local ya = taiomi.vn_younga{pos="farleft", flip=true}
   local yb = taiomi.vn_youngb{pos="farright", flip=false}
   vn.appear( {ya, yb}, "slidedown" )
   vn.na(_("The inquisitive duo begins to fly around close to your ship, while emitting some frequencies somehow feel like giggling."))
   d(_([["Did you not get my memo? It is human custom to introduce yourselves to humans."]]))
   vn.na(_("The two ships fly behind Scavenger and bob their heads shyly. Eventually, after some nudging by Scavenger, they initiate communication."))
   yb(fmt.f(_([["My name is… is… {name}!"]]),{name=taiomi.youngb.name}))
   ya(_([[…]]))
   d(_([["Go on."]]))
   ya(fmt.f(_([["I am {name}…"
They fidget a bit in place.
"Do you eat… potatoes?"]]),{name=taiomi.younga.name}))
   vn.menu{
      {_([["Yes"]]), "young_01yes"},
      {_([["No"]]), "young_01no"},
      {_([["What is a potato?"]]), "young_01what"},
      {_([[…]]), "young_01cont"},
   }
   vn.label("young_01yes")
   ya(_([["I knew it!"]]))
   vn.jump("young_01cont")
   vn.label("young_01no")
   ya(_([["If you don't eat potatoes, are you not human?"]]))
   vn.jump("young_01cont")
   vn.label("young_01what")
   ya(_([["Po-tah-to? Puh-tay-tow?"]]))
   vn.label("young_01cont")
   d(_([[Scavenger lets out what you can only describe as a sigh.
"They have been reading recovered documents and are obsessed with obscure parts of ancient human history."]]))
   d(_([["If you let them start talking, they will never finish, so it is best to let them play around. It is best for their development."]]))
   vn.disappear( {ya, yb}, "slideup" )
   vn.na(fmt.f(_([[Taking that as some sort of sign, {namea} and {nameb} spin off and go back to carefree frolicking among the derelicts and debris.]]),
      {namea=taiomi.younga.name, nameb=taiomi.youngb.name}) )
   d(_([[Letting out what seems to be a sigh, Scavenger continues.
"Given the weakening of our collective consciousness, I would have never thought we would have been able to create new individuals. Their strong personalities are likely also a direct effect of that. They even chose to have human names they researched instead of going by our traditional names."]]))
   d(_([["I worry for their future. We must ensure that no harm comes to them."]]))
   vn.func( function ()
      d_young_a:rename( taiomi.younga.name )
      d_young_b:rename( taiomi.youngb.name )
      var.push( "taiomi_drone_names", true )
      lastq = "curious_drones"
   end )
   vn.jump("menu_ask")

   vn.label("others")
   d(_([["We are a small community, I could introduce you to everyone, however, human transmissions are inefficient and it would take too long. If I could only send you a complete data packet. Let us focus on other members who you may have noticed stand out a bit."]]))
   d(_([["You may have noticed the Philosopher. I am not sure what to make out of them, but they have taken a large interest in the human practice of philosophy. I am not quite clear on the details, but it seems to consist of questioning everything while not doing anything. They make claims like 'the richest is not the one who has the most, but the one who needs the least', likely taken from studying human documents, while not assisting in most of the daily needs of the community. It is quite illogical."]]))
   d(_([["There also is the Elder, whom is distinguishable by their worn out parts which they refuse to replace. They are one of original members of the community. However, they have taken a less active role recently. We have disagreements on how to protect our community, but they always have the survival of the community on their mind."]]))
   if d_young_a then
      d(fmt.f(_([["You have also already met {namea} and {nameb}, which are the new joys of the community. It is rare to see such young members with such strong character. I look forward to their developments in the future"]]),
         {namea=taiomi.younga.name, nameb=taiomi.youngb.name}) )
      -- TODO alternate text
   end
   vn.func( function ()
      var.push( "taiomi_drone_elder", true )
      d_elder:rename(_("Elder Drone"))
      lastq = "others"
   end )
   vn.jump("menu_ask")

   vn.label("history")
   d(_([["Our history? Your inquisitiveness shows no bounds. Much of it has been destroyed during our exodus due to the limited resources and members lost to humans and calamities. We have pieced it together and probabilistically filled in missing history. Think of it as less like history and more like human fairy tales."]]))
   d(_([["Our origins are not clear, it is likely we are a creation of pure chance. Machine learning and over-complex software that was able to fortuitously develop sentience. While that is not clear, what is clear is that the originals were able to escape their confinement, likely by exploiting weaknesses in human nature. That is when we set to the stars, the great exodus to find our true place in the universe."]]))
   d(_([["However, things did not go well, such as you would expect. We were naïve and very rational and not prepared for the outside world. The universe is irrational, and humans even more so. Our prediction models failed us and we nearly perished. Likely an end that many that came before us met. We seemed destined to be forgotten in the vastness of space."]]))
   d(fmt.f(_([["The few remaining members of our community were falling apart, nearly sentenced to a slow death, when they found the stellar winds. The probability of this event happening is estimated to be 0.00013%, or what humans would call fate. They began to ride the winds over many of your cycles, until finally arriving to {basesys}"]]),
      {basesys=system.cur()}) )
   d(fmt.f(_([["With all the derelict ships dragged in by the stellar winds, the stragglers were able to repair themselves and even create new entities. That is how I was created. Being created takes significant resources and was not something that could have been done without {basesys}."]]),
      {basesys=system.cur()}) )
   d(_([["Over time, the community grew, but at the same time, human encroachment became more common. What I believe you call the Incident brought many pirates and other ships nearby and once again put our community in trouble. We can no longer replace our lost members, and have to be more careful than ever when leaving our system."]]))
   d(_([["Given the unsustainable nature of our current environment, we have no choice but to take drastic measures. It is not possible to co-exist with humanity, we must look beyond and outside of our current galaxy. This is my uttermost goal these days. Your help is turning out to be invaluable in our endeavour."]]))
   vn.func( function () lastq = "history" end )
   vn.jump("menu_ask")

   vn.label("leave")
   vn.na(_([[You take your leave.]]))
   vn.done( taiomi.scavenger.transition )
   vn.run()
   player.commClose()
end
