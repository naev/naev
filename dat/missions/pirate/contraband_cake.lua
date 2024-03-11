--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Pirate Smuggle Cake">
 <unique />
 <priority>3</priority>
 <chance>20</chance>
 <location>Bar</location>
 <cond>
   if require("common.pirate").systemPresence() &lt; 0 then
      return false
   end
   return require("misn_test").reweight_active()
 </cond>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[

   Opens up Pirate contraband missions. The player is asked to take a cake
   (illegal) to a random system.

]]--
local pir = require "common.pirate"
local vn = require 'vn'
local vni = require "vnimage"
local car = require "common.cargo"
local fmt = require "format"

-- TODO replace images with something meant for the VN
local givername = _("Sweaty Individual")
local giverimage, giverportrait = vni.generic()
local receivername = _("Burly Individual")
local receiverimage = vni.generic()

function create()
   -- Note: this mission does not make any system claims.

   mem.origin_p, mem.origin_s = spob.cur()

   -- target destination. Override "always_available" to true.
   mem.destplanet, mem.destsys, mem.numjumps, mem.traveldist, mem.cargo, mem.avgrisk, mem.tier = car.calculateRoute( rnd.rnd(5, 10), {always_available=true} )
   if mem.destplanet == nil or pir.factionIsPirate( mem.destplanet:faction() ) then
      misn.finish(false)
   end

   -- Choose reward
   local jumpreward = 3000
   local distreward = 0.50
   mem.reward    = 1.5^mem.tier * (mem.numjumps * jumpreward + mem.traveldist * distreward) * (1 + 0.05*rnd.twosigma())

   misn.setNPC( givername, giverportrait, _("You see a nervous looking individual that seems to be sweating profusely.") )
end

function accept()

   local accepted = false
   vn.clear()
   vn.scene()
   local g = vn.newCharacter( givername, { image=giverimage } )
   vn.transition()
   vn.na(_("You approach the giddy looking character."))
   g(_([[They are fairly absorbed in thought and take a while to notice you. Suddenly, without warning, they jump out of their chair and almost fall to the ground.
"Crikes! You scared the shit out of me."
They clutch at their shirt as if trying to hold their heart in their ribcage.]]))
   g(_([[They pant for a while and then try to somewhat recover their composure as they sit down once more. They seem sweatier than before.
"Hey, say, you look like a pilot. You see, I was asked to deliver a package, but I'm not feeling so well…"
They cough to the side somewhat exaggeratedly while looking at you from the corner of their eye.]]))
   g(fmt.f(_([[They lean closer to you and lower their voice.
"I was asked to deliver a… a…"
They furrow their brows for a second.
"…a cake! To {pnt} in the {sys} system."]]), {pnt=mem.destplanet, sys=mem.destsys}))
   g(_([["It's nothing out of the ordinary, I swear! Only that this cake you see, it's got some special… icing that is really sensitive. Like super sensitive. The slightest disturbance can melt it and lay it all to waste. In particular, any radiation can easily melt it even though it is shielded. Especially stuff like scanning radiation. You catch my drift?"]]))
   g(fmt.f(_([["All you would have to do is take the cake and go to {pnt} in the {sys} system, without getting any of that nasty scanning radiation on you."
They shiver with disgust to emphasize and you can see some of their sweat fly off onto the bar floor.
"Once you deliver it I'll split the money half and half with you."]]), {pnt=mem.destplanet, sys=mem.destsys}))
   vn.na(_("You stare at them coldly."))
   g(fmt.f(_([["Fine fine, take it all. It should be {credits}. Just remember no scanning means no problems."
They extend their sweaty hand towards you.
"So, are you in?"]]), {credits=fmt.credits(mem.reward)}))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   vn.na(_("You decline their offer."))
   g(_([[They slump down in the chair in defeat.
"Shit, what am I going to do. Think, Sam, think!"]]))
   vn.na(_("As you take your leave, you notice they have once again started to profusely sweat."))
   vn.done()

   vn.label("accept")
   vn.na(_("You accept their mission without shaking their hand."))
   g(_([["You're a lifesaver!"
They lean forward and get a bit more serious.
"One thing though, you do know how to stealth to get around scanning right?"]]))
   vn.func( function () accepted = true end )
   vn.menu{
      {_("Stealth?"), "stealthtut"},
      {_("Of course!"), "notut"},
   }

   vn.label("stealthtut")
   g(_([["Oh boy, you're not going to deliver this cake in one piece without stealth. It's very simple. All ships have three main statistics: range they are detected at, signature range, and stealth range. Detection determines how far away ships can detect your presence, while signature range controls how well they can target your ship and identify it. When within signature range, ships can then scan you which does nasty things like melting cakes."]]))
   g(fmt.f(_([["To avoid getting spotted and scanned, you can go into stealth with {key}. When in stealth, you move much slower than normal, however, ships can only detect you when they are within your stealth range. No detection, no scanning, no problems. You can only stealth if there are no ships nearby, and it is easier to stealth in asteroids or systems with interference. And if you get detected while in stealth, your cover will be blown."]]),
      {key=string.format("#b%s#0",naev.keyGet("stealth"))}))
   g(fmt.f(_([["So as long as you stealth with {key} and stay away from ships, you won't be scanned and the cake will be alright."]]),
      {key=string.format("#b%s#0",naev.keyGet("stealth"))}))
   g(_([["If a ship starts to scan you, it'll be marked on your radar and overlay map. Furthermore, if you're carrying stuff that you don't want scanned, your autonav system will automatically be paused so you react. Make sure to get away and stealth so that they can't scan you anymore. Don't want to spoil the cake!"]]))

   vn.label("notut")
   g(fmt.f(_([["Great. One second, let me get the cake."
They go to the restroom and come back holding a nondescript brown box that seems to have 'Cake' hastily scribbled on it. They promptly hand it over to you while looking both ways.
"OK, so that's it. Make sure to take this to {pnt} in the {sys} system, and watch out for scanning!"]]), {pnt=mem.destplanet, sys=mem.destsys}))
   vn.na(_("As you leave, behind you can hear them let out a big sigh of what you can only assume is relief."))
   vn.run()

   if not accepted then
      return
   end

   misn.accept()

   local c = commodity.new( N_("Cake"), N_("A cake that is supposedly sensitive to scanning radiation. Don't let anyone scan it.") )
   c:illegalto( {"Empire", "Dvaered", "Soromid", "Sirius", "Za'lek", "Frontier"} )
   mem.carg_id = misn.cargoAdd( c, 0 )

   misn.osdCreate( _("Deliver Cake"), { fmt.f(_("Fly to {pnt} in the {sys} system without getting scanned"), {pnt=mem.destplanet, sys=mem.destsys}) } )

   misn.setTitle(_("Deliver Cake"))
   misn.setReward(mem.reward)
   misn.setDesc( fmt.f(_("Deliver a cake to {pnt} in the {sys} system. Apparently it has a special frosting and will be damaged if you are scanned. Use stealth to avoid getting scanned."), {pnt=mem.destplanet, sys=mem.destsys} ) )
   misn.markerAdd(mem.destplanet)

   hook.land( "land" ) -- only hook after accepting
end

-- Land hook
function land()
   if spob.cur() ~= mem.destplanet then
      return
   end

   vn.clear()
   vn.scene()
   local b = vn.newCharacter( receivername, { image=receiverimage } )
   vn.transition()
   vn.na(_("After you land you are promptly greeted by a burly looking individual."))
   b(_([["So you've been sent by Sam? That weasel, I knew they weren't cut out for this work."]]))
   vn.na(_([[You guess that this is the person you were supposed to deliver the cake to and hand it over.]]))
   b(_([[They take a look at the word 'Cake' written on the box and burst into laughter. After a while they calm down and turn to you.
"This was their idea wasn't it? Always been a bit weird that one."]]))
   b(_([["I guess you're not naïve enough to think this is a cake right?"
They chuckle.
"Anyway, you did good work bringing it here. I'll get you your reward wired and if you're interested in doing new jobs look into the mission computers. I've white-listed you to act as a courier."]]))
   b(_([["Take care."
As they lumber away, you suddenly notice that quite a few suspicious figures in the background disappear and follow them away. What have you gotten into?]]))
   vn.sfxVictory()
   vn.na( fmt.reward(mem.reward) )
   vn.run()

   player.pay(mem.reward)

   pir.addMiscLog(_("You helped deliver a 'cake' for some shady individuals. Your success has opened up more 'special' delivery missions from the mission computer."))

   -- increase faction
   faction.modPlayerSingle("Pirate", rnd.rnd(2, 4))
   misn.finish(true)
end
