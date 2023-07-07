--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Dvaered Shopping">
 <unique/>
 <priority>2</priority>
 <chance>50</chance>
 <location>Bar</location>
 <faction>Dvaered</faction>
 <done>Dvaered Negotiation 2</done>
 <cond>
   if faction.playerStanding("Dvaered") &lt; 20 then
      return false
   end
   if system.get("Goddard"):jumpDist() &gt;= 10 then
      return false
   end
   if spob.cur():services().shipyard == nil then
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
-- Dvaered Shopping
-- This is the last mission of the Dvaered Recruitment arc.
-- The player escorts the fleet of Colonel Okran in route to Zhiru in Goddard.
-- Mission is really straightforward, until, on Goddard, the Trickster appears and reveals that Lord Fatgun has been assasinated.
-- Okran tries to kill the player with the Goddard cruiser and player meets the trickster.

   Stages :
   0) Way to Zhiru
   1) Way back
   2) Way to trickster rendezvous and kill Okran
   3) Way to any Dvaered planet to get rewarded
--]]

require "proximity"
local fmt      = require "format"
local portrait = require "portrait"
local vn       = require 'vn'
local vntk     = require 'vntk'
local dv       = require "common.dvaered"
local pir      = require "common.pirate"
local lmisn    = require "lmisn"

local DvFleet, noPirates

local agentPort = "dvaered/dv_military_m2.webp"
local trickPort = "pirate/pirate2.webp"
local solPort = portrait.getMil("Dvaered")

function create()
   mem.godpnt, mem.godsys  = spob.getS("Zhiru") -- Target of the escort
   mem.tripnt, mem.trisys  = spob.getS("Darkshed") -- Where you meet the trickster

   if not misn.claim({mem.godsys,mem.trisys}) then misn.finish(false) end -- Claim

   misn.setNPC( _("Colonel Okran"), agentPort, _("Colonel Okran, the second of Lord Fatgun") )
end

function accept()
   misn.accept()
   local pay = 100e3

   -- Discussion
   vn.clear()
   vn.scene()
   local sol = vn.newCharacter( _("Colonel Okran"), { image=portrait.getFullPath(agentPort) } )
   local doaccept = false

   vn.transition()
   sol(fmt.f(_([["Hello, citizen. We finally got the formal agreement of The Goddard Company for the obtention of our second battlecruiser. We would like you to escort our convoy to and from Zhiru. I can pay you {credits} Are you interested for that mission?"]]), {credits=fmt.credits(pay)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Refuse"), "refuse"},
   }

   vn.label("refuse")
   sol(_([["Really? Well… I was not expecting that. Anyway, come back if you change your mind."]]))
   vn.func( function () doaccept = false end )
   vn.done()

   vn.label("accept")
   sol(_([["Thank you, citizen. Your help will be most appreciated as I can not mobilize our entire fleet for this escort mission. Many of our ships are needed to protect Lord Fatgun's planets from attacks.
I would like you to specially take the anti-bomber role. For that, I recommend you to fly a light ship."]])) -- That's actually a pretext for the player to fly a ship that can run away when Okran decides to attack the player
   vn.func( function () doaccept = true end )
   vn.done()
   vn.run()

   -- Test acceptance
   if not doaccept then misn.finish(false) end

   -- Mission details
   local nextsys = lmisn.getNextSystem(system.cur(), mem.godsys)
   misn.setTitle(_("Dvaered Shopping"))
   misn.setReward( pay )
   misn.setDesc( fmt.f(_("You accepted to fly with a fleet of Dvaered ships that will escort a freshly-bought Dvaered Goddard battlecruiser.")))
   mem.misn_marker = misn.markerAdd( system.cur() )
   mem.misn_state = 0
   misn.osdCreate( _("Dvaered Shopping"), {
      _("Protect the Dvared cruiser."),
      fmt.f(_("Jump to {sys}."), {sys=nextsys} ),
      fmt.f(_("Land on {pnt}."), {pnt=mem.godpnt} ),
   } )

   mem.next_sys = system.cur() -- Next system player has to cross
   mem.origin = spob.cur()

   -- hooks
   hook.enter("enter")
   hook.land("land")
   hook.jumpout("escape")
end

function enter()
   -- Player escorts the convoy
   if mem.misn_state == 0 then

      if mem.next_sys == system.cur() then

         mem.conv_leader = pilot.add( "Dvaered Retribution", "Dvaered", mem.origin, _("Colonel Okran") )
         mem.conv_leader:control()
         mem.convLeft = false
         mem.conv_leader:memory().formation = "vee"
         DvFleet( false, mem.origin ) -- ideally, escort ships should have permadeath...

         if system.cur() == mem.godsys then -- Next: land on Zhiru
            mem.conv_leader:land( mem.godpnt )
            misn.osdActive(2)
            misn.markerMove( mem.misn_marker, mem.godpnt )
         else -- Next: move to next system
            local nextsys = lmisn.getNextSystem(system.cur(), mem.godsys)
            mem.conv_leader:hyperspace( nextsys )
            misn.osdDestroy()
            misn.osdCreate( _("Dvaered Shopping"), {
               _("Protect the Dvared cruiser."),
               fmt.f(_("Jump to {sys}."), {sys=nextsys} ),
               fmt.f(_("Land on {pnt}."), {pnt=mem.godpnt} ),
            } )
            misn.markerMove( mem.misn_marker, nextsys )
            mem.next_sys = nextsys -- Increment system
         end

         mem.origin = system.cur()
         hook.pilot( mem.conv_leader, "exploded", "OkranDeath" )
         hook.pilot( mem.conv_leader, "jump", "OkranMoves" )
         hook.pilot( mem.conv_leader, "land", "OkranMoves" )
      else -- Player went to the wrong system
         vntk.msg(_("Wrong system"),_("It appears you jumped in the wrong system somehow. you lost your convoy and failed your mission."))
         misn.finish(false)
      end

   -- Player takes off for the second part of the trip
   elseif mem.misn_state == 1 then -- normally, system.cur == mem.godsys
      noPirates()
      mem.conv_leader = pilot.add( "Dvaered Goddard", "Dvaered", mem.godpnt, _("Colonel Okran") )
      mem.conv_leader:memory().formation = "vee" -- This is less of a death trap if the player is just behind the leader
      DvFleet( true, mem.godpnt )

      local target = system.get("Ogat")
      mem.conv_leader:control()
      mem.conv_leader:hyperspace( target )
      misn.osdCreate( _("Dvaered Shopping"), {
         _("Protect the Dvared battleship."),
         fmt.f(_("Jump to {sys}."), {sys=target} ),
      } )
      misn.markerMove( mem.misn_marker, target )
      hook.pilot( mem.conv_leader, "exploded", "OkranDeath" )

      hook.timer( 30., "spawnTrick" )
      hook.timer( 15., "OkranTalks" )

   -- Player Jumps in for the rendezvous with the trickster
   elseif mem.misn_state == 2 and system.cur() == mem.trisys then
      noPirates()
      mem.trickster = pilot.add( "Hyena", "Independent", mem.tripnt, _("The Trickster") )
      mem.trickster:control() -- Do nothing
      mem.trickster:setHilight()
      hook.pilot( mem.trickster, "hail", "discussWithTrickster" )
      mem.trickster:setInvincible() -- Not really satisfactory but well.

   -- Player Jumps in Goddard to fight Okran
   elseif mem.misn_state == 2 and system.cur() == mem.godsys then
      noPirates()
      mem.okran = pilot.add( "Dvaered Goddard", "Dvaered", nil, _("Colonel Okran") ) -- This time, he is alone
      mem.okran:setFaction("Mercenary")
      mem.okran:setHilight()
      mem.okran:setHostile()
      hook.pilot( mem.okran, "exploded", "OkranDeath" )
   end
end

function land()
   escape() -- Test if player is not escaping a situation

   -- Player makes it to Zhiru.
   if mem.misn_state == 0 and spob.cur() == mem.godpnt then
      vn.clear()
      vn.scene()
      vn.transition( )
      vn.na(_([[After landing, Okran tells you and the rest of the escort pilots to wait for him at the spaceport's bar. As a member of his crew, you are granted access to the special VIP customers restricted bar.
The walls are covered with pictures of different versions of the Goddard battlecruiser, each of which is accompanied by a small explicative text and a series of important facts: "Did you know that 365 Goddards Mk VII-DM were constructed between UST-510 and UST-567? This model is the most successful ever, and about 120 of them are reportedly still in use at date of UST-601."]]))
      vn.na(_([[Some time later, Colonel Okran finally comes and shows you a fancy key-chain representing a crossed-eyed gorilla lifting a Goddard cruiser. "They have nice goodies!" Before you have time to ask one for yourself, it is time to depart.]]))
      vn.done()
      vn.run()

      mem.misn_state = 1

   -- Player gets rewarded
   elseif mem.misn_state == 3 and spob.cur():faction() == faction.get("Dvaered") then
      local amount = 1e6 -- Actually a lot of money

      vn.clear()
      vn.scene()
      vn.transition( )
      vn.na(fmt.f(_([[While landing, you receive a message from the Dvaered military control:
"Congratulations, citizen {player}. Our data suggests you have won your first honourable fight against a gentleman. Please reach the spaceport's military contact office for further information."
When you arrive at the said office, a soldier greets you.]]), {player=player.name()}))
      local sol = vn.newCharacter( _("Dvaered Soldier"), { image=portrait.getFullPath(solPort) } )
      sol(fmt.f(_([["Good day, citizen. We got the information that you destroyed the Colonel Okran in a fair space fight in the framework of a Warlords rivalry. This achievement gives you the respect of the Dvaered High Command. As such, you will be allowed to purchase the Heavy Combat Vessel License if you did not already have it. Moreover, our cleaning team did find a fancy key chain among the remains of Okran's ship. As the customs requires it, this item is now yours, for what it's worth."
"And finally, two Warlords, namely Lady Proserpina and Lord Richthofen, required us to give you the sum of {credits} and their greetings. I wish you to stay right, loyal and strong, citizen."]]), {credits=fmt.credits(amount)}))
      vn.done()
      vn.run()

      if diff.isApplied( "heavy_combat_vessel_license" ) then -- Player already has the license
         dv.addStandardLog( _([[You were supposed to escort the Dvaered Colonel Okran who wanted to purchase a battlecruiser for his Warlord. Instead, the Warlord got assassinated and an imperial pilot convinced the Colonel Okran that you betrayed him. He tried to kill you with the battlecruiser in question. You managed to destroy Colonel Okran.]]) )
      else
         dv.addStandardLog( _([[You were supposed to escort the Dvaered Colonel Okran who wanted to purchase a battlecruiser for his Warlord. Instead, the Warlord got assassinated and an imperial pilot convinced the Colonel Okran that you betrayed him. He tried to kill you with the battlecruiser in question. You managed to destroy Colonel Okran and was rewarded with the ability to buy the Heavy Combat Vessel License by Okran's enemies among the Dvaered.]]) )
         diff.apply("heavy_combat_vessel_license")
      end

      player.pay(amount)
      player.outfitAdd( "Fancy Key Chain" )
      misn.finish(true)
   end
end

-- Tests to determine if the player is running away
function escape()
   if (mem.misn_state == 0 and not mem.convLeft) or mem.misn_state == 1 then
      vntk.msg(_("You left the convoy"),_([[You were supposed to escort the convoy, not to go away. You failed your mission!]]))
      misn.finish(false)
   end
end

-- Add the Dvared fleet
function DvFleet( cruiser, origin )
   mem.convoy = {}
   local j = 1

   if cruiser then
      mem.convoy[1] = pilot.add( "Dvaered Retribution", "Dvaered", origin, _("Escort") )
      j = 2
   end

   mem.convoy[j  ] = pilot.add( "Dvaered Vigilance", "Dvaered", origin, _("Escort") )
   mem.convoy[j+1] = pilot.add( "Dvaered Phalanx", "Dvaered", origin, _("Escort") )
   mem.convoy[j+2] = pilot.add( "Dvaered Phalanx", "Dvaered", origin, _("Escort") )
   mem.convoy[j+3] = pilot.add( "Dvaered Phalanx", "Dvaered", origin, _("Escort") )
   local no = j+4
   for i = 1, 4 do
      mem.convoy[no  ] = pilot.add( "Dvaered Vendetta", "Dvaered", origin, _("Escort") )
      mem.convoy[no+1] = pilot.add( "Dvaered Ancestor", "Dvaered", origin, _("Escort") )
      no = no + 2
   end

   for i, p in ipairs(mem.convoy) do
      p:setLeader( mem.conv_leader )
   end
end

-- Manage Okran's death
function OkranDeath()
   -- Mission failure (not sure how it can happend that way)
   if mem.misn_state == 0 or mem.misn_state == 1 then
      vntk.msg(_("Mission Failure"), _([[You're not sure how this routine escort mission came to the point where a cruiser was lost, but the result is there: you failed.]]))
      misn.finish(false)
   else
      vntk.msg(_("Finally got him"), _([[The destruction of a battlecruiser is certainly an impressive sight. It is like a small town being annihilated before your eyes, irradiating the whole star system with light and heat. This is the Dvaered way of life, apparently.
Now, it's time to land on a Dvaered planet to get recognized as the pilot who killed Colonel Okran.]]))
      mem.misn_state = 3
      misn.osdCreate( _("Dvaered Shopping"), {
         _("Land on any Dvaered planet"),
      } )
      misn.markerRm( mem.misn_mark )
   end
end

-- Move on to next system
function OkranMoves()
   mem.convLeft = true
   if system.cur() == mem.godsys then
      misn.osdActive(3)
      player.msg(_("Convoy landed"))
   else
      misn.osdActive(2)
      player.msg(_("Convoy jumped out"))
   end
end

-- Spawn the trickster
function spawnTrick()
   mem.trickster = pilot.add( "Hyena", "Independent", mem.godpnt, _("???") )
   mem.trickster:control()
   mem.trickster:follow( player.pilot() )
   hook.timer(0.5, "proximity", {anchor = mem.trickster, radius = 400, funcname = "veryBadThings", focus = player.pilot()})
end

-- Trickster makes his show
function veryBadThings()
   vn.clear()
   vn.scene()
   vn.na(_([[You get hailed by a Hyena and a communication channel is opened between you, Colonel Okran and the Hyena in question.]]))
   local sol = vn.newCharacter( _("Colonel Okran"), { image=portrait.getFullPath(agentPort) } )
   local trick = vn.newCharacter( _("???"), { image=portrait.getFullPath(trickPort) } )
   trick(_([["Yo, Okran! How are you, mate? Enyoyin' your new toy? Hewhewhew!"]]))
   sol(_([["I am sorry, but I do not know you, citizen."]]))
   trick(_([["Ya do not know me? Hewhewhew! Of course! But really, you were expectin' to do all your shameful manoeuvring without ever meeting me? Ya all know that you are still subjects to his Imperial Majesty, right? No! You forgot that! Luckily, I'm here to remind that to you!"]]))
   sol(_([["I see. You are the… trickster, true? Lord Fatgun told me once I could run into you at some point.]]))
   trick(_([["Hewhewhew! Different people call me differently. You boars tend to use that name indeed. So? happy to finally meet me?"]]))
   sol(_([["Look. I am running some serious business here. And I am not in need of a buffoon right now, so please proceed getting lost."]]))
   trick(_([["Your Fatgun bro got killed one period ago."]]))
   sol(_([[…]]))
   trick(_([["Ya all are jobless jerks, folks!]]))
   sol(_([[…]]))
   trick(_([["Hewhewhew! Proserpina and the other asshole, Richthofen, they conducted a raid on Castellan III knowing that you were away. They managed to shoot a volley of hypersonic bombs at his bunker. And… Kaboom! One less moronic warlord in this galaxy!"]]))
   sol(_([["You… you must be kidding? Right, you are kidding!"]]))
   trick(_([["I aint ever kidding when it's about hypersonic bombs. We got the confirmation of the death by your Dvaered High Command. Your guy is down, bro! Totally down. D.O.W.N."]]))
   sol(_([["How did they get the information I was here?"]]))
   trick(fmt.f(_([["{player} told them."]]), {player=player.name()}))
   vn.na(_([[You scratch your head, trying to remember if you were part to a scheming to kill Lord Fatgun when you notice Colonel Okran has been ejected from the communication channel. It's only you and the wild newcomer now.]]))
   trick(fmt.f(_([["I hope you got an afterburner, mate! Hewhewhew!
So, seriously, what do you think of that? You chose the Dvaered way, we make it the Dvaered way! Ya know what? Meet me in orbit of {pnt} in {sys} and I'll explain you why I actually just made you a huge favour!
Oh. If ya manage to kill Okran right now, you won't need to meet me. Just land on any Dvaered planet and you'll understand everything! All ya got to do is destroying a Dvaered Goddard! Piece of cake, right?
Bye, mate!"]]), {pnt=mem.tripnt,sys=mem.trisys}))
   vn.na(_([[The Hyena goes away, letting you alone with the group of angry Dvaered.]]))
   vn.done()
   vn.run()

   -- Update mission details
   misn.osdDestroy()
   misn.osdCreate( _("Dvaered Shopping"), {
      fmt.f(_("Meet and hail the Trickster in orbit of {pnt} in {sys}"), {pnt=mem.tripnt,sys=mem.trisys} ),
   } )
   misn.markerMove( mem.misn_marker, mem.tripnt )
   misn.setReward( _("Staying alive?") )
   misn.setDesc( fmt.f(_("The shopping mission has gone wild: you have to try to survive and encounter a shady Imperial pilot who apparently might give you information")))
   mem.misn_state = 2

   -- Make the Dvaered Angry
   mem.conv_leader:setFaction( "Mercenary" )
   mem.conv_leader:setHostile()
   for i, p in ipairs(mem.convoy) do
      p:setFaction( "Mercenary" )
      p:setHostile()
   end

   mem.conv_leader:control(false)
   mem.trickster:taskClear()
   mem.trickster:runaway( mem.conv_leader, true ) -- Trickster just runs away without a goal because he is a freak
   hook.timer( 2., "Hewhewhew" )
   mem.trickster:setInvincible()
end

-- Trickster explains what to do
function discussWithTrickster()
   player.commClose()

   -- Some recurrent text
   local leave = _("Leave")
   local why   = _([["Why did you tell Okran I had betrayed him?"]])
   local who   = _([["Who are you actually?"]])
   local what  = _([["What is your motivation?"]])
   local how   = _([["How am I supposed to destroy a Goddard now?"]])
   local where = _([["Where am I supposed to find him?"]])

   vn.clear()
   vn.scene()
   local trick = vn.newCharacter( _("The Trickster"), { image=portrait.getFullPath(trickPort) } )
   trick(fmt.f(_([["Aye, {player}, how do you do?"]]), {player=player.name()}))
   vn.menu{
      {_([["Very well, thank you. And you?"]]), "thank"},
      {_([["Very well for someone you just tried to get murdered"]]), "murder"},
      {_([["I will feel much better once I'll have killed you"]]), "kill"},
      {why, "why"},
      {who, "who"},
   }

   vn.label("thank")
   trick(fmt.f(_([["Kof kof kof! For real? You asked how I am? It's… It's the first time someone cares about me.
Hewhewhew! You're a singular person, {player}, for sure! I wish you to never end on the bad end of a firing gun. Oh yeah! Or at least the latest possible!
So I guess you want to know why I told the pack of boars that you betrayed them, right?"]]), {player=player.name()}))
   vn.jump("why")

   vn.label("murder")
   trick(_([["Come on… Come on… You survived, right? I would not have put you in a situation where you could have been killed! Never! We are good ol' friends, after all, aren't we? Hewhewhew!"]]))
   vn.jump("why")

   vn.label("kill")
   trick(fmt.f(_([["Hewhewhew! Aren't we overreacting a bit over there, {player}? Yes we are. You won't kill me… I mean… I cannot exclude that ya kill me one day, but not today. Instead, ya will rather kill good ol' Okran. You know why?"]]), {player=player.name()}))
   vn.jump("why")

   vn.label("why")
   trick(_([["Guess what, mate? When I told Okran you betrayed him, I made you the greatest favour possible! Yep for real!
You know how to work your way up among the Dvaereds? Hewhewhew! By killing Dvaereds! Totally true! Hundred percent fact-checked! Dvaereds weirdos are the only kind of weirdos that will love you more if you kill other Dvaered weirdos.
But watch out, mate! For killing Dvaereds weirdos is a subtle art, ya know? You can't just attack random Vendettas in space and win the High Command's respect. You need to kill people in the context of a "honorable fight between two respectable gentlemen". Yo, because Dvaered are for sure respectable gentlemen!"]]))
   trick(_([["So, I gave you the occasion to fight a respectable asshol… hem… gentleman. And if you kill him, the Dvaered will fully accept you among them. You'll be like their mate, ya know? Hewhewhew! For what it's worth. But you decided to join them.
So here I am! Your white knight on his white horse. I give you not only the occasion to fight one Colonel, but also one Colonel in a Goddard. Ya'll never have any better occasion!"]]))
   vn.menu{
      {_([["Thanks, genius. And how do I destroy a Goddard now?"]]), "how"},
      {where, "where"}, {who, "who"}, {what, "what"}, {leave, "leave"},
   }

   vn.label("who")
   trick(_([["Who I am actually? Hewhewhew! Kof kof kof! I am a loyal servant of our Emperor! Some call me 'the flying clown', because I lead a squadron named the 'flying circus'. People are funny, ya know? I'm presently Laughing My Ass Off! And your Dvaered friends call me the trickster. No clue why, Hewhewhew! Oh, and some call me 'Captain Hew Hew'.
But that's all bullshit! For I am a real human being, with a heart in my chest. A heart that cries when people use mean words against me… Hewhewhew! Did I make you sad? Don't lie! You were sympathizing! I saw you!
I'm so annoying! Am I not? Hewhewhew! Yep I totally am!"]]))
   vn.menu{
      {how, "how"}, {where, "where"}, {why, "why"}, {what, "what"}, {leave, "leave"},
   }

   vn.label("how")
   trick(_([["C'mon! You're a adult, no? Ya know how to obliterate a Goddard. Don't you? Oh, I'm dumb. Maybe you don't even have the Heavy Combat Vessel License! Hewhewhew! Did I put you in a shitty situation? Kof kof kof!
No, of course not! Now that Lord Fatass… Fatgun died, all his soldiers have been put to the Dvaered Military Reserve, and Okran don't command them anymore. Officially, he was also sent to the Reserve, but custom needs him to avenge his lord first, by at least killing one person (you). So he will be flying an unescorted Goddard.
And as they teach at school, an unescorted capship is a dead capship. I recommend to use a corvette with torpedoes. Unless of course you have an other capship stronger than a Dvaered Goddard. Of course. Hewhewhew!"]]))
   vn.menu{
      {who, "who"}, {where, "where"}, {why, "why"}, {what, "what"}, {leave, "leave"},
   }

   vn.label("where")
   trick(_([["Ah! Good question! Okran wants to kill you. I am pretty sure he will stay at Zhiru and try to intercept you each time you enter the system. I would totally do that if I was him. The Goddard system is a very central place in our galaxy. And it's a good spot for the maintenance of his ship."]]))
   vn.menu{
      {who, "who"}, {how, "how"}, {why, "why"}, {what, "what"}, {leave, "leave"},
   }

   vn.label("what")
   trick(_([["My motivation? But… I just wanted to be kind to you. I am a philanthropist, you know? Hewhewhew!"]])) -- He certainly has a doubtful motivation that has to be determined later
   vn.menu{
      {who, "who"}, {how, "how"}, {why, "why"}, {where, "where"}, {leave, "leave"},
   }

   vn.label("leave")
   vn.done()
   vn.run()

   -- Update mission details
   misn.osdDestroy()
   misn.osdCreate( _("Dvaered Shopping"), {
      fmt.f(_("Encounter and kill Okran in {sys}"), {sys=mem.godsys} ),
   } )
   misn.markerMove( mem.misn_marker, mem.godsys )
   misn.setReward( _("Staying alive?") )
   misn.setDesc( fmt.f(_("The shopping mission has gone wild: you have to destroy the freshly bought Goddard battlecruiser.")))

   mem.trickster:taskClear(false)
   mem.trickster:land( mem.tripnt )
end

-- Just get pple tell things
function OkranTalks()
   mem.conv_leader:broadcast( _("Aaah. I feel so good! Nothing is like flying a Goddard.") )
end
function Hewhewhew()
   mem.trickster:broadcast( _("Hewhewhew!") )
end

-- Remove the Pirates form the equation
function noPirates()
   for k,f in ipairs(pir.factions) do
      pilot.toggleSpawn(f)
      pilot.clearSelect(f)
   end
end
