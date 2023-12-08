--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Negotiation 2">
 <unique/>
 <priority>2</priority>
 <chance>30</chance>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <done>Dvaered Negotiation 1</done>
 <cond>
   if faction.playerStanding("Dvaered") &lt; 20 then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
 <notes>
  <campaign>Dvaered Recruitment</campaign>
 </notes>
</mission>
--]]
--[[
-- Dvaered Negotiation 2
-- This is the second mission of the Dvaered Recruitment arc when the player has to help a Warlord getting a second Goddard Battleship.
-- The player needs to please Baron Sauterfeldt, who is a shareholder of Goddard. For that, player has to steal a nozzle hubcap to a Hyena gang leader.
-- First, the player has to defy the gang's deputee leader in a deadly race named the "Hallway to Hell"
-- Afterwards, the leader accepts a death fight with the player, who has to disable her and steal the nozzle hubcap.

   Stages :
   0) Way to Ulios to encounter the Baron
   1) Way to Vault in Mason for the biker's convention
   2) Taking off for the "Hallway to Hell" with Big Bunny Benny
   3) Land back on Vault
   4) Discussions on Vault
   5) Taking off for the duel with Blue Belly Billy
   6) Way back to Ulios for getting paid
--]]

local tut      = require 'common.tutorial'
local fmt      = require "format"
local portrait = require "portrait"
local vn       = require 'vn'
local vntk     = require 'vntk'
local dv       = require "common.dvaered"
local pir      = require "common.pirate"



local agentPort = "dvaered/dv_military_m2.webp"
local BBB1Port = "pirate/pirate4.webp"
local BBB2Port = "pirate/pirate6.webp"
local npc1 = portrait.get()
local npc2 = portrait.get()
local npc3 = portrait.get()
local npc4 = portrait.get()

local gangs = { _("Big Bang Band"), -- gang of Blue Belly Billy
                _("Cringe Crew"),
                _("Dilettante Daemons"),
                _("The Fuel Drinkers"),
                _("Flamethrower Gang"),
                _("The Stabbers"),
                _("The Warriors") }

-- Forward declarations of functions
local spawnGang, land_state1, land_state3

function create()
   mem.baronpnt, mem.baronsys  = spob.getS("Ulios") -- Where you get paid
   mem.convpnt,   mem.convsys  = spob.getS("Vault") -- Where you do the fights

   if not misn.claim(mem.convsys) then misn.finish(false) end -- Claim

   misn.setNPC( _("Colonel Okran"), agentPort, _("Colonel Okran, the second of Lord Fatgun might have work for you") )
end

function accept()
   misn.accept()

   -- Discussion
   vn.clear()
   vn.scene()
   local sol = vn.newCharacter( _("Colonel Okran"), { image=portrait.getFullPath(agentPort) } )
   local doaccept = false

   vn.transition()
   sol(_([["Good day, citizen. I have a new task for you, in relation to our efforts to get a second battlecruiser for Lord Fatgun. Are you interested?"]]))
   vn.menu{
      {_([["Of course! What should I do?"]]), "accept"},
      {_([["I'm afraid I have other things to do."]]), "refuse"},
   }

   vn.label("refuse")
   sol(_([["I guess working for a true major Warlord might be too challenging for someone like you. Too bad."]]))
   vn.func( function () doaccept = false end )
   vn.done()

   vn.label("accept")
   sol(_([["I knew you would say that.
So, as the negotiations with Goddard shareholders is making progress, we identified another private person that needs to be… worked on. It is the very famous Baron Sauterfeldt, President of the recently-created Hereditary Democratic Republic of Ulios."]]))
   vn.func( function () doaccept = true end )
   vn.menu{
      {_([["Shall I exert some negotiation on him with my cannons?"]]), "negotiate"},
      {_([["When you say he needs to be worked on… What does that imply?"]]), "work"},
   }

   vn.label("negotiate")
   sol(_([["Oh no! We do not need you to provide to Mr. Sauterfeldt the same kind of service you did to Mrs. Grosjean. It would even rather be the exact opposite."]]))
   vn.jump("work")

   vn.label("work")
   sol(fmt.f(_([["The Baron is in not so cordial terms with the Imperial authorities (partially due to the fact that he declared Ulios independent not so long ago). By a complex alliance game, this might get him to back Lord Fatgun's views."
"He told me that he was impressed by the way the 'obstacle Agrippina Grosjean' was handled and needs us to provide him a pilot for some 'tiny special task' (those are his words). I have no idea what this task might be, but he required me to send you directly to {pnt} in {sys} to meet him."]]), {pnt=mem.baronpnt, sys=mem.baronsys}))

   vn.done()
   vn.run()

   -- Test acceptance
   if not doaccept then misn.finish(false) end

   -- Mission details
   misn.setTitle(_("Dvaered Negotiation 2"))
   misn.setReward( _("They did not tell you") )
   misn.setDesc( fmt.f(_("A Dvaered Warlord needs you to perform a tiny special task for the Baron Sauterfeldt. Only the Baron knows what that task is.")))
   mem.misn_marker = misn.markerAdd( mem.baronpnt )
   mem.misn_state = 0
   misn.osdCreate( _("Dvaered Negotiation 2"), {
      fmt.f(_("Go to {sys} and land on {pnt}."), {sys=mem.baronsys, pnt=mem.baronpnt} ),
   } )

   -- hooks
   hook.enter("enter")
   hook.land("land")
   hook.jumpout("escape")
   hook.load("loading")
end

function enter()
   -- Player will perform the Hallway to Hell
   if mem.misn_state == 2 and system.cur() == mem.convsys then
      local pp = player.pilot()
      local ps = pp:ship()
      if ps==ship.get("Hyena") or ps==ship.get("Pirate Hyena") then
         pilot.toggleSpawn()
         pilot.clear()
         local targetF = faction.dynAdd( "Independent", "targets", _("Targets"), {clear_enemies=true, clear_allies=true} )

         -- Put the targets
         local center = pp:pos()
         local t1 = center + vec2.new(0,5000)
         local t2 = center + vec2.new(-4000,-2000)
         local t3 = center + vec2.new(4000,-2000)
         mem.pil1 = pilot.add("Llama", targetF, t1, _("Kill Me"))
         mem.pil2 = pilot.add("Llama", targetF, t2, _("Kill Me"))
         mem.pil3 = pilot.add("Llama", targetF, t3, _("Kill Me"))
         mem.pil1:setHealth(10)
         mem.pil2:setHealth(10)
         mem.pil3:setHealth(10)
         mem.pil1:setHilight()
         pp:setTarget(mem.pil1)
         mem.pil1:disable()
         mem.pil2:disable()
         mem.pil3:disable()
         mem.pil1:setVisible()
         mem.pil2:setVisible()
         mem.pil3:setVisible()

         -- Compute timer
         -- Max vel of a top notch Hyena is 476 (without extra outfits). We give a bit of margin
         local dist = 5000 + vec2.dist( t1, t2 ) + vec2.dist( t2, t3 )
         mem.timer = dist/460
         misn.osdDestroy()
         misn.osdCreate( _("Dvaered Negotiation 2"), {
            fmt.f(_("Destroy the 3 targets \n ({time} s remaining)"), {time=mem.timer} ),
            fmt.f(_("Land on {pnt} and talk to Blue Belly Billy"), {pnt=mem.convpnt} ),
            _("Disable and board Blue Belly Billy"),
            fmt.f(_("Go back to {pnt} in {sys}"), {pnt=mem.baronpnt,sys=mem.baronsys} ),
         } )

         -- Put the platforms
         mem.pla1 = pilot.add("Zebra", targetF, .5*(t1+t2), _("Dodge Me"))
         mem.pla2 = pilot.add("Zebra", targetF, .5*(t1+t3), _("Dodge Me"))
         mem.pla3 = pilot.add("Zebra", targetF, .5*(t2+t3), _("Dodge Me"))
         mem.pla1:outfitRm( "all" )
         mem.pla1:outfitAdd( "Enygma Systems Turreted Fury Launcher", 3 ) -- Difficulty can be tuned here (1<=nb<=6)
         mem.pla1:outfitAdd( "Sensor Array", 4 ) -- Apparently, they do stack
         mem.pla1:setSpeedLimit( .0001 )
         mem.pla1:memory().ranged_ammo = 1 -- AI was initialized before the launchers did exist so it's not aware it has ammo
         mem.pla1:control()
         mem.pla2:outfitRm( "all" )
         mem.pla2:outfitAdd( "Enygma Systems Turreted Fury Launcher", 3 )
         mem.pla2:outfitAdd( "Sensor Array", 4 )
         mem.pla2:setSpeedLimit( .0001 )
         mem.pla2:memory().ranged_ammo = 1
         mem.pla2:control()
         mem.pla3:outfitRm( "all" )
         mem.pla3:outfitAdd( "Enygma Systems Turreted Fury Launcher", 3 )
         mem.pla3:outfitAdd( "Sensor Array", 4 )
         mem.pla3:setSpeedLimit( .0001 )
         mem.pla3:memory().ranged_ammo = 1
         mem.pla3:control()

         -- Block the player and set timers
         pp:control()
         pp:face( mem.pil1 )
         mem.countdown = 10
         mem.omsg = player.omsgAdd(tostring(mem.countdown), 0, 50)
         hook.timer( 1.0, "timerIncrement", _("GO!") )
         hook.timer( 10.0, "startHallway" )
         hook.pilot( mem.pil1, "death", "killPil1" )

         vn.clear()
         vn.scene()
         local sai = vn.newCharacter( tut.vn_shipai() )
         vn.transition( tut.shipai.transition )
         sai(_([["Hello, sir. I compiled the available data on the very stupid challenge you just accepted. During this challenge, you will have to destroy three disabled ships in a certain order. Each time you destroy a ship, I will automatically set the target focus to the next ship, so that you will be able to focus on piloting and shooting.
So, don't try to auto-target around, and just shoot at the targets I will be indicating, and it will be fine."]]))
         vn.done( tut.shipai.transition )
         vn.run()

      else
         vntk.msg("",_([[The Hallway to Hell must be performed on a Hyena. Come back with a Hyena.]]))
      end

   -- Player fights Blue Belly Billy
   elseif mem.misn_state == 4 and system.cur() == mem.convsys then

      local pp = player.pilot()
      local ps = pp:ship()
      if ps==ship.get("Hyena") or ps==ship.get("Pirate Hyena") then
         pilot.toggleSpawn()
         pilot.clear()

         local targetF = faction.dynAdd( "Marauder", "targets", _("Targets"))
         mem.bbb = pilot.add("Hyena", targetF, mem.convpnt, _("Blue Belly Billy"))
         mem.bbb:control()
         mem.bbb:moveto( mem.convpnt:pos() + vec2.new(3000, 0) )
         mem.misn_state = 5

         pp:control()
         pp:face(mem.bbb)

         mem.countdown = 10
         mem.omsg = player.omsgAdd(tostring(mem.countdown), 0, 50)
         hook.timer( 1.0, "timerIncrement", _("GO!") )
         hook.timer( 10.0, "startDuel" )
         hook.pilot( mem.bbb, "exploded", "killBBB" )
         hook.pilot( mem.bbb, "board", "boardBBB" )

      else
         vntk.msg("",_([[This is supposed to be a Hyena duel. Come back with a Hyena.]]))
      end

   -- Player enters Mason, where the Hyena festival is happening
   elseif system.cur() == mem.convsys then
      pir.clearPirates() -- No pirates

      -- Spawn the gangs
      for i, g in ipairs(gangs) do
         spawnGang(g)
      end
   end
end

function land()
   escape() -- Test if player is not escaping a situation

   -- Player makes it to Ulios.
   if mem.misn_state == 0 and spob.cur() == mem.baronpnt then
      vn.clear()
      vn.scene()
      vn.transition( ) -- TODO: rework that part once Baron Sauterfeldt has a portrait
      vn.na(_([[After landing, you are approached by a group of well-dressed people: "His Lordship, the Baron Sauterfeldt, will encounter you. Please follow us to the presidential palace."]]))
      vn.na(_([[They guide you to the urban transport station, and while a crowd of workers are waiting for the next train in a suffocating heat, you enter a small shuttle: "His Lordship decided the creation of the hypervelocity shuttles system last cycle. Since then, this system saves much time to first-class citizen who used to get stuck in traffic jams or in over-crowded heliports. His genial idea was to use the same tunnel net as the subway."]]))
      vn.na(_([["The shuttle system leads us directly under the presidential palace. I'm afraid you won't see its new pediment that His Lordship had built recently."]]))
      vn.na(_([[You proceed to follow your guides through a checkpoint into the administrative part of the palace, the kind of place where people wear moccasins and the carpets have no spots. You enter a seemingly common and empty meeting room and start to ask yourself where the Baron is.]])) -- Remark: implicitly, we suggest the baron is in his Gauss, the Pinnacle.
      vn.na(fmt.f(_([[Suddenly, a huge holographic face appears in the centre of the room:
"Hello, and welcome on the planet Ulios, {player}! I am the Baron Dovai Sauterfeldt. I hope you got a smooth travel to our very remote humble piece of land! I am truly delighted to meet you, {player}, truly… Or did we already meet before? Mmmm! I am afraid I am perfectly incapable to remember most of the astonishingly inspiring people I tend to meet."]])
         {player=player.name()}))
      vn.na(fmt.f(_([["Anyway, you are truly most certainly one very inspiring person, {player}, aren't you? Yes, you are! You know what? I am really happy to finally have time to discuss with such a notable person as you."]]),
         {player=player.name()}))
      vn.na(fmt.f(_([[You start to wonder if the "special service" you came to provide was simply to endure the baron's spiel, but he continues:
"{player}, do you know the Lady named Blue Belly Billy? Oh, of course you don't! For she is not the inspiring kind of person! She is the leader of a group… or rather a gang of young people. Nestor, do you like young people?"
The man on your right answers: "Certainly not, your Lordship."]]),
         {player=player.name()}))
      vn.na(_([[The baron continues: "I don't either, Nestor, they are rude and dirty and… young. Anyway, I got lost… So, this gang of young people have the particularity to fly Hyena interceptors. It is what one calls a Hyena bikers gang. Those bikers like to tune their ships, as they say, by using scavenged pieces of, preferably shiny, metal. A bit like magpies do with their nests, except they have worst taste."]]))
      vn.na(_([["That lady uses a fake pre-space-age trash top as left nozzle hubcap. My art historian, Flintley, examined it once, and noticed that this piece was constructed by the famous counterfeiter Themistocle Zweihundertshrittenausdaheim, who became later a famous artist. So this piece, although being a fake archaeological relic, is a true piece of Themistocle Zweihundertshrittenausdaheim!
And as such, it belongs to my collection, is that true, Nestor?"
The man on your right answers: "Certainly, your Lordship."]]))
      vn.na(fmt.f(_([[The baron explains his plan: "Hyena bikers will soon gather for a festival on {pnt} in {sys}. I want you to go there, find Blue Belly Billy, defy her in a Hyena duel, disable her ship, get her nozzle hubcap and come back." You ask if it would not be preferable to steal the hubcap while the ship is at dock, but the baron answers:
"Yes, maybe you are right, but when I offered her to sell it to me, she refused and said very unfriendly words to me, so I would prefer her to get humiliated in a duel. Besides, this humiliation will ensure that her gang will reject her and not try to avenge her."]]), {pnt=mem.convpnt,sys=mem.convsys}))
      vn.na(_([[After a final "good luck" wished to you by the Baron, the connection is cut and you are guided back to the spaceport and to your ship. It is time for you to buy a Hyena and go to that festival.]]))
      vn.done()
      vn.run()

      mem.misn_state = 1
      misn.osdDestroy()
      misn.osdCreate( _("Dvaered Negotiation 2"), {
         fmt.f(_("Go to {pnt} in {sys}"), {pnt=mem.convpnt,sys=mem.convsys} ),
         _("Find the gang leader Blue Belly Billy (you'll probably need to have a Hyena to get her attention)"),
         _("Somehow get your hands on the left nozzle hubcap of her ship"),
         fmt.f(_("Go back to {pnt} in {sys}"), {pnt=mem.baronpnt,sys=mem.baronsys} ),
      } )
      misn.markerMove( mem.misn_marker, mem.convpnt )
      misn.setReward( _("They still did not tell you") )
      misn.setDesc( fmt.f(_("The Baron Sauterfeldt wants you to defy a Hyena biker to steal her left nozzle hubcap, that happens to be sort of a piece of fine art.")))

   -- Player arrives at Hyena festival
   elseif mem.misn_state == 1 and spob.cur() == mem.convpnt then
      land_state1()

   -- Player lands after having beaten the Hallway to Hell
   elseif mem.misn_state == 3 and spob.cur() == mem.convpnt then
      land_state3()

   -- Player gets paid
   elseif mem.misn_state == 6 and spob.cur() == mem.baronpnt then
      local pay = 200e3
      vn.clear()
      vn.scene()
      vn.transition( )
      vn.na(_([[This time, after landing, you see the Colonel Okran among the people waiting for you. All of them accompany you once again to the meeting room in the presidential palace of Baron Sauterfeldt. They examine the nozzle hubcap, looking satisfied, while the Baron appears through the holoprojectors:
"That is awesome, my little colonel Orcon! The pilot you recruited for me really made it. I am very pleased. Now, I guess I have no further objections to let your little friend, the Lord Whatsoever, add a new toy to his arsenal. As promised, I will call the sales manager of Goddard right now!"]])) -- TODO: once everyone has a portrait, rework that part
      vn.na(_([[The Dvaered colonel and you get promptly dismissed, and he talks with you on the way back.]]))
      local sol = vn.newCharacter( _("Colonel Okran"), { image=portrait.getFullPath(agentPort) } )
      sol(fmt.f(_([["Citizen, you apparently did a pretty good job, out there. I did not fully understand why the Baron Sauterfeldt did want you to get your hands on that… piece of… stuff, but what is certain is that we can now count on him for getting our second battleship."
"You deserve your reward. Please receive {credits}."]]), {credits=fmt.credits(pay)}))
      vn.menu{
         {_("Talk with Okran"), "talk"},
         {_("Leave Okran ASAP"), "shutup"},
      }

      vn.label("talk")
      sol(_([["Hopefully, we should now have a sufficient number of supporters inside The Goddard Company to ensure we will get our ship. I will not be annoyed not to have to take part anymore to those shenanigans with that decadent galactic oligarchy."]]))
      sol(_([["You know, as a Dvaered, I have been raised in a much simpler environment, far from all those sophistications. Everything in a place like Ulios seems to have been made to make me feel uncomfortable. Look for example at this first-class suburban shuttle we took to go from spaceport to the palace, and back. This kind of service is an insult to the regular workers who have to use crowded, hot and clammy trains. No Dvaered in their right mind would endorse such a thing. Except for the decadent Warlords like Lady Bitterfly or Lord Chainsaw, of course."]]))
      -- Okran talks about the fact that Dvaered ideology is based on egalitarianism. But with time, an increasing number of Warlords and generals manage to obtain special privileges. Dvaered society is also haunted by the fear of decadence
      vn.jump("shutup")

      vn.label("shutup")
      vn.done()
      vn.run()

      -- TODO once the whole recruitment campaign is stabilized: faction.get("Dvaered"):modPlayerRaw(someQuantity)
      dv.addStandardLog( _([[You stole a nozzle hubcap to the leader of a gang of Hyena bikers for the Baron Sauterfeldt to help Lord Fatgun getting a second Goddard battlecruiser.]]) )
      player.pay(pay)
      misn.finish(true)
   end
end

-- Wrapper for loading
function loading()
   if mem.misn_state == 1 and spob.cur() == mem.convpnt then
      land_state1()
   elseif mem.misn_state == 3 and spob.cur() == mem.convpnt then
      land_state3()
   end
end

-- NPCs at bar
function land_state1()
   misn.npcAdd("biker1", _("Hyena Biker"), npc1, _("A Hyena pilot."))
   misn.npcAdd("biker2", _("Hyena Biker"), npc2, _("A Hyena pilot."))
   misn.npcAdd("biker3", _("Hyena Biker"), npc3, _("A Hyena pilot."))
   misn.npcAdd("bbb1", _("Hyena Biker"), BBB1Port, _("A Hyena pilot.")) -- bbb1 unlocks the challenge "Hallway to Hell"
   misn.npcAdd("biker4", _("Hyena Biker"), npc4, _("A Hyena pilot."))
end
function land_state3()
   misn.npcAdd("bbb1bis", _("Big Bunny Benny"), BBB1Port, _("A member of the Big Bang Band"))
   misn.npcAdd("bbb2", _("Blue Belly Billy"), BBB2Port, _("The leader of the fearsome bikers gang 'Big Bang Band' accepts to talk with you."))
end

-- Tests to determine if the player is running away
function escape()
   -- Player quits in the middle of the Hallway to Hell
   if mem.misn_state == 2 then
      vntk.msg(_("Mission Failure"),_([[You were supposed to cross the Hallway to Hell, not to cowardly run away.]]))
      misn.finish(false)

   -- Player escapes the fight with BBB
   elseif mem.misn_state == 5 then
      vntk.msg(_("Mission Failure"),_([[You were supposed to defeat Blue Belly Billy, not to run away.]]))
      misn.finish(false)
   end
end

-- Bikers say stuff
function biker1()
   vn.clear()
   vn.scene()
   local biker = vn.newCharacter( _("Hyena Biker"), { image=portrait.getFullPath(npc1) } )
   vn.transition()
   biker(_([["The Hyena is much more than just an interceptor. It's a way of life: speed, adrenalin, and leather jackets. That ship is awesome and unforgiving: any error can be fatal when you fly it out there."]]))
   vn.done()
   vn.run()
end
function biker2()
   vn.clear()
   vn.scene()
   local biker = vn.newCharacter( _("Hyena Biker"), { image=portrait.getFullPath(npc2) } )
   vn.transition()
   biker(_([["You came for the festival? What gang do you belong to? You don't wear any gang jacket."]]))
   vn.done()
   vn.run()
end
function biker3()
   vn.clear()
   vn.scene()
   local biker = vn.newCharacter( _("Hyena Biker"), { image=portrait.getFullPath(npc3) } )
   vn.transition()
   biker(_([["Of course, we are gangs! We make tons of totally criminal stuff! One day, I even parked my Hyena at a spot reserved for delivery ships!"]]))
   vn.done()
   vn.run()
end
function biker4()
   vn.clear()
   vn.scene()
   local biker = vn.newCharacter( _("Hyena Biker"), { image=portrait.getFullPath(npc4) } )
   vn.transition()
   biker(_([["Me and my buddies will show an incredible Hyena trick at this cycle's festival: two Hyenas fly side by side at full speed, then both pilots eject from their ship and enter the other one. I'm with the Stabbers Gang."]]))
   vn.done()
   vn.run()
end

-- Encounter Big Bunny Benny
function bbb1()
   vn.clear()
   vn.scene()
   local biker = vn.newCharacter( _("Hyena Biker"), { image=portrait.getFullPath(BBB1Port) } )
   vn.transition()
   biker(_([["My name is Big Bunny Benny. I never met you before. Are you a Hyena biker?"]]))
   vn.menu{
      {_([["Of course I am!"]]), "biker"},
      {_([["I am not, but I'd like to learn."]]), "noob"},
      {_([["No, I just came to steal someone's nozzle hubcap."]]), "steal"},
      {_([["No, I just came to defy someone."]]), "duel"},
   }

   vn.label("biker")
   biker(_([["For real? What gang do you belong to?"]]))
   vn.menu{
      {_([["The Torpedo Crew"]]), "noob"},
      {_([["The Heavy Headbutt Gang"]]), "headbutt"},
      {_([["The Boring Bears Band"]]), "noob"},
      {_([["The Earless Painters"]]), "noob"},
      {_([["I lied. I'm no biker."]]), "noob"},
   }

   vn.label("noob")
   biker(_([["Yeah, that confirms my first impression: you're a authentic noob. If you want to become a biker, you first need to join a gang. For that, you need to prove yourself by beating a special challenge."]])) -- Actually, later, one could allow the player to join one of the gangs
   vn.menu{
      {_([["What kind of challenge?"]]), "challenge"},
      {_([["I'd like to defy a gang member."]]), "duel"},
      {_([["Not interested, I just came to steal someone's nozzle hubcap."]]), "steal"},
   }

   vn.label("steal")
   biker(_([["Seriously? You must have been sent by the Count Sauerkartoffel then? For some reason, he keeps on sending private pilots after Blue Belly Billy's left nozzle hubcap. Usually, they pretend to want to defy Billy. Sauerkartoffel must be short of decently-smart henchmen given how subtlety-deficient you look. Wait? Have you been recruited through a Dvaered Warlord? That sounds very possible."]]))
   vn.jump("hallway")

   vn.label("duel")
   biker(_([["Ah! You came here to defy a gang member! Do you know that gang members only accept duels to death? Yo do! So, who do you want to defy?"]]))
   vn.na(_([[You answer you would like to defy Blue Belly Billy.]]))
   vn.jump("hallway")

   vn.label("headbutt") -- Player is lucky and gives a name that scares BBB
   biker(_([["The… Heavy Headbutt Gang? Really? Oh my… They did send someone to our Hyena festival!
Please tell your boss we will pay him very soon! I promise! Don't get angry, I can even pay you right now! Do you want my money?"]]))
   vn.menu{
      {_([["Exactly: I came for the money. Give it now."]]), "money"},
      {_([["I did not come for that. I want to defy Blue Belly Billy."]]), "hallway"},
   }

   vn.label("money")
   biker(_([["Actually, nothing tells me that you truly are a member of the Heavy Headbutt Gang. If you are, you'll have no problem to beat the Hallway to Hell challenge. Afterwards, I'll let you settle our affair with our leader Blue Belly Billy. I guess you will want to have a duel with her, no?"]]))
   vn.jump("hallway")

   vn.label("challenge")
   biker(_([["There are plenty of kinds of challenges! Some consist in doing acrobatic figures, other are races. There is also the 'Bad Boar Battle' challenge, it is the one you must beat to enter my gang: the Big Bang Band, and it consists in flying to Dvaer Prime, broadcasting messages of insults against House Dvaered, and then… if you manage to get to a civilized planet in one piece, you have beaten the challenge."]]))
   biker(_([["Oh, and there is also the 'Hallway to Hell'. It was invented by my gang leader, Blue Belly Billy, and nowadays, she requires anybody who wants to defy her to first beat the Hallway to Hell. Do you want to try it out?"]]))
   vn.na(_([[You answer you would like to try it out.]]))
   biker(_([["Hey? I was joking, you know. The Hallway to Hell is the most deadly of all challenges. And if you survive it, you'll have to fight to death against Blue Belly Billy. No-one ever won against her!"]]))
   vn.na(_([[You insist.]])) -- This is a bit too directive, any reorganization is welcome to get the player feel they can actually decide what they say.
   vn.jump("hallway")

   vn.label("hallway")
   biker(_([["If you want to approach Blue Belly Billy, you have to prove that you are worth of it. You need to cross the 'Hallway to Hell' inside a Hyena."
"It is a very simple challenge in principle, but only few manage to make it alive through the Hallway to Hell. And even fewer manage to make it in a time short enough for Blue Belly Billy to accept them to defy her."]]))
   biker(_([[Let me explain: there are 3 weak targets in a triangle, and you need to destroy all those 3 targets with your Hyena. Sounds pretty simple, no? But between those targets, there are platforms that fire fury missiles at you. You'll have to dodge the missiles and destroy the 3 buoys before the timer runs out… unless you are too scared for our challenge and you did already pee your pants.
We are preparing the set. Just take off to start the challenge!"]]))

   vn.done()
   vn.run()

   mem.misn_state = 2
   misn.osdDestroy()
   misn.osdCreate( _("Dvaered Negotiation 2"), {
      _("Destroy the 3 targets"),
      fmt.f(_("Land on {pnt} and talk to Blue Belly Billy"), {pnt=mem.convpnt} ),
      _("Disable and board Blue Belly Billy"),
      fmt.f(_("Go back to {pnt} in {sys}"), {pnt=mem.baronpnt,sys=mem.baronsys} ),
   } )
end

-- Spawn the gangs for a fun scene
function spawnGang( name )
   local dist = rnd.rnd() * system.cur():radius() * 0.5
   local pos1  = vec2.newP( dist, rnd.angle() )
   dist = rnd.rnd() * system.cur():radius() * 0.5
   local pos2  = vec2.newP( dist, rnd.angle() )

   local fct  = faction.dynAdd( "Independent", "bikers", _("Bikers"), {clear_enemies=true, clear_allies=true} )
   local gang = {}
   for i = 1, 6 do
      gang[i] = pilot.add( "Hyena", "Marauder", pos1, name )
      gang[i]:setFaction(fct)
      if i ~= 1 then
         gang[i]:setLeader( gang[1] )
      end
   end

   local j = gang[1] -- Gang leader
   j:control()
   j:moveto(pos2)

   -- Define a Mission Variable: 1 => moveto pos1, 2 => moveto pos2
   -- 3 => do tricks at pos1, 4 => do tricks at pos2
   local mem = j:memory()
   mem.MV_dest = 2
   mem.MV_pos1 = pos1
   mem.MV_pos2 = pos2
   mem.MV_hook = hook.pilot( j, "idle", "nextActivity" )
end

-- Manage activities of gangs squadrons.
function nextActivity( leader )
   local mem = leader:memory()
   hook.rm( mem.MV_hook ) -- Clear hook

   local gang = leader:followers()
   for i, p in ipairs(gang) do
      p:control( false ) -- Reinitialize them: they resume following the leader
   end
   gang[6] = leader -- Get everyone in the list
   leader:taskClear()

   local tricks = false

   if mem.MV_dest == 1 then
      mem.MV_dest = 3
      tricks = true
   elseif mem.MV_dest == 2 then
      mem.MV_dest = 4
      tricks = true
   elseif mem.MV_dest == 3 then
      mem.MV_dest = 2
      leader:moveto(mem.MV_pos2)
   else -- mem.MV_dest == 4
      mem.MV_dest = 1
      leader:moveto(mem.MV_pos1)
   end

   -- Pilots runaway from eachother. This might look like they perform tricks hopefully
   if tricks then
      for i, p in ipairs(leader:followers()) do
         p:control()
      end
      gang[1]:runaway( gang[2], true )
      gang[2]:runaway( gang[1], true )
      gang[3]:runaway( gang[4], true )
      gang[4]:runaway( gang[3], true )
      gang[5]:runaway( gang[6], true )
      gang[6]:runaway( gang[5], true )
      hook.timer( 3., "nextActivity", leader ) -- After 3 seconds, move to next location
   else -- Just wait for them to be idle
      mem.MV_hook = hook.pilot( leader, "idle", "nextActivity" )
   end
end

-- Increment the printing timer
function timerIncrement( msg )
   mem.countdown = mem.countdown - 1
   if mem.countdown == 0 then
      player.omsgChange(mem.omsg, msg, 3)
   else
      hook.timer( 1.0, "timerIncrement", msg)
      player.omsgChange(mem.omsg, tostring(mem.countdown), 0)
   end
end

-- Start the Hallway to Hell challenge
function startHallway()
   player.pilot():control(false)
   hook.timer( 5., "tick" )
   mem.finishH = hook.timer( mem.timer-10., "finishHallway" ) -- Final countdown
   mem.loseH = hook.timer( mem.timer, "timeOver" )
   mem.pla1:attack(player.pilot())
   mem.pla2:attack(player.pilot())
   mem.pla3:attack(player.pilot())
end

-- Final countdown for the Hallway to Hell
function finishHallway()
   mem.countdown = 10
   mem.omsg = player.omsgAdd(tostring(mem.countdown), 0, 50)
   hook.timer( 1.0, "timerIncrement", _("Time Over!") )
end

-- Manage timer in OSD
function tick()
   mem.timer = mem.timer - 5. -- TODO: I guess it will deviate with time
   if mem.misn_state == 2 then -- If player didnt already win
      misn.osdDestroy()
      misn.osdCreate( _("Dvaered Negotiation 2"), {
         fmt.f(_("Destroy the 3 targets \n ({time} s remaining)"), {time=mem.timer} ),
         fmt.f(_("Land on {pnt} and talk to Blue Belly Billy"), {pnt=mem.convpnt} ),
         _("Disable and board Blue Belly Billy"),
         fmt.f(_("Go back to {pnt} in {sys}"), {pnt=mem.baronpnt,sys=mem.baronsys} ),
      } )
      hook.timer( 5., "tick" )
   end
end

-- Player failed the Hallway to Hell because time is over
function timeOver()
   player.omsgChange(mem.omsg, _("Time Over!"), 3)
   vntk.msg( _("Mission Failure"), _([[Too late: you failed the Challenge.]]) )
   mem.pla1:taskClear()
   mem.pla2:taskClear()
   mem.pla3:taskClear()
   misn.finish(false)
end

-- Player destroys a target (Hallway to Hell)
function killPil1()
   player.omsgAdd(_("Target 1 Destroyed"), 3, 50)
   hook.pilot( mem.pil2, "death", "killPil2" )
   mem.pil2:setHilight()
   player.pilot():setTarget(mem.pil2)
end
function killPil2()
   player.omsgAdd(_("Target 2 Destroyed"), 3, 50)
   hook.pilot( mem.pil3, "death", "killPil3" )
   mem.pil3:setHilight()
   player.pilot():setTarget(mem.pil3)
end
function killPil3()
   player.omsgAdd(_("Target 3 Destroyed: you won!\n(Provided you dodge the remaining missiles)"), 3, 50)
   mem.misn_state = 3
   misn.osdActive(2)
   hook.rm(mem.loseH) -- One can not lose once one won
   mem.pla1:taskClear()
   mem.pla2:taskClear()
   mem.pla3:taskClear()

   -- Remove the final countdown timer
   hook.rm( mem.finishH )
   player.omsgRm( mem.omsg )
end

-- Player talks with Benny after having beaten the Hallway to Hell
function bbb1bis()
   vn.clear()
   vn.scene()
   local biker = vn.newCharacter( _("Big Bunny Benny"), { image=portrait.getFullPath(BBB1Port) } )
   vn.transition()
   biker(_([["Wow! you have beaten the Hallway to Hell challenge! That is quite impressive. I think you owned the right to get destroyed by Blue Belly Billy's Blasters!"]]))
   vn.done()
   vn.run()
end

-- Triggers the duel with Blue Belly Billy
function bbb2()
   vn.clear()
   vn.scene()
   local biker = vn.newCharacter( _("Blue Belly Billy"), { image=portrait.getFullPath(BBB2Port) } )
   vn.transition()
   biker(_([["I am Blue Belly Billy. Do you really want to defy me? You know I never lost a duel, right?
Well, actually, I never fought a duel before because no one has never beaten the Hallway to Hell. But anyway, I am nonetheless a dangerous pilot… hem… yes, I am. Are you sure you don't want to abandon? I'll let you live. No?
Fine, let's get that behind us. Just take off with your Hyena and I'll follow you."]])) -- Actually, BBB is not that strong. The true challenge is the Hallway to Hell (and we're quite early game)
   vn.done()
   vn.run()

   mem.misn_state = 4
   misn.osdDestroy()
   misn.osdCreate( _("Dvaered Negotiation 2"), {
      _("Wait for the signal"),
      _("Disable and board Blue Belly Billy"),
      fmt.f(_("Go back to {pnt} in {sys}"), {pnt=mem.baronpnt,sys=mem.baronsys} ),
   } )
end

-- Start duel against Blue Belly Billy
function startDuel()
   player.pilot():control(false)
   mem.bbb:taskClear()
   mem.bbb:attack(player.pilot())
end

-- Player killed BBB
function killBBB()
   vntk.msg("",_([[You feel satisfaction while watching the remains of Billy's ship being scattered in all directions by the final explosion of her ship…
Then, you remember you had to disable and board her ship in order to recover a nozzle hubcap.
Your mission failed!]]))
   misn.finish(false)
end

-- Player boards BBB
function boardBBB()
   vntk.msg("",_([[Once your ship is docked with Billy's interceptor, you send your extra-vehicular android to recover the left nozzle hubcap. Soon, the robot comes back with what indeed looks like a very primitive trash top. Before undocking, you realize it might be an occasion to loot a bit around in Billy's ship.]]))
   -- We don't de-board the player: looting the ship is allowed
   mem.misn_state = 6
   misn.osdActive(3)
   misn.markerMove( mem.misn_marker, mem.baronpnt )
   pilot.toggleSpawn()

   local c = commodity.new( N_("Nozzle Hubcap"), N_("This looks like a prehistoric trash top.") )
   misn.cargoAdd( c, 0 )
end
