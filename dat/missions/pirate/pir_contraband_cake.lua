--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Pirate Smuggle Cake">
 <flags>
  <unique />
 </flags>
 <avail>
  <priority>3</priority>
  <chance>20</chance>
  <location>Bar</location>
  <cond>system.cur():presence("Pirate") &gt; 0</cond>
 </avail>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[

   Opens up Pirate contraband missions. The player is asked to take a cake
   (illegal) to a random system.

]]--

local portrait = require 'portrait'
local vn = require 'vn'
require "cargo_common"
require "numstring"


-- TODO replace images with something meant for the VN
local givername = _("Sweaty Individual")
local giverportrait = portrait.get()
local giverimage = portrait.getFullPath(giverportrait)
local receivername = _("Burly Individual")
local receiverimage = portrait.getFullPath(portrait.get())

-- Use hidden jumps
cargo_use_hidden = false

-- Always available
cargo_always_available = true

-- This is in cargo_common, but we need to increase the range
function cargo_selectMissionDistance ()
   return rnd.rnd( 5, 10 )
end


function create()
   -- Note: this mission does not make any system claims.

   origin_p, origin_s = planet.cur()
   local routesys = origin_s
   local routepos = origin_p:pos()

   -- target destination
   destplanet, destsys, numjumps, traveldist, cargo, avgrisk, tier = cargo_calculateRoute()
   if destplanet == nil or destplanet:faction() == faction.get("Pirate") then
      misn.finish(false)
   end
   
   -- Choose reward
   finished_mod = 2.0 -- Modifier that should tend towards 1.0 as Naev is finished as a game
   jumpreward = 2000
   distreward = 0.40
   reward    = 1.5^tier * (numjumps * jumpreward + traveldist * distreward) * finished_mod * (1. + 0.05*rnd.twosigma())
  
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
   g(_([[They pant for a while and then try to recover somewhat their composure as they sit down once more. They seem sweatier than before.
"Hey, say, you look like a pilot. You see, I was asked to deliver a package, but I'm not feeling so well…"
They coughs to the side somewhat exaggeratedly while looking at you from the corner of the eye.]]))
   g(string.format(_([[They lean closer to you and lower their voice.
"I was asked to deliver a… a…"
They furrow their brows for a second.
"…a cake! To %s in the %s system."]]), destplanet, destsys))
   g(_([["It's nothing out of the ordinary, I swear! Only that this cake you see, it's got some special… icing that is really sensitive. Like super sensitive. The slightest disturbance can melt it and lay it all to waste. In particular, any radiation can easily melt it even though it is shielded. Especially stuff like scanning radiation. You catch my drift?"]]))
   g(string.format(_([["All you would have to do is take the cake and go to %s in the %s system, without getting any of that nasty scanning radiation on you."
They shiver with disgust to emphasize and you can see some of their sweat fly off onto the bar floor.
"Once you deliver it I'll split the money half and half with you."]]), destplanet, destsys))
   vn.na("You stare at them coldly.")
   g(string.format(_([["Fine fine, take it all. It should be %s. Just remember no scanning means no problems."
They extend their sweaty hand towards you.
"So, are you in?"]]), creditstring(reward)))
   vn.menu{
      {_("Accept"), "accept"},
      {_("Decline"), "decline"},
   }

   vn.label("decline")
   vn.na(_("You decline their offer."))
   g(_([[They slump down in the chair in defeat.
"Shit, what am I going to do. Think Sam think!"]]))
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
   g(_([["Oh boy, you're not going to deliver this cake in one piece without stealth. It's very simple, all ships have three main statistics: range they are detected at, evasion range, and stealth range. Detection determines how far away ships can detect your presence, while evasion range controls how well they can target your ship and identify it. When within evasion range, ships can then scan you which does nasty things like melting cakes."]]))
   g(string.format(_([["To avoid getting spotted and scanned, you can go into stealth with %s. When in stealth, you move much slower than normal, however, ships can only detect you when they are within your stealth range. No detection, no scanning, no problems. You can only stealth if there are no ships nearby, and it is easier to stealth in asteroids or systems with interference. And if you get detected while in stealth, your cover will be blown."]]),
      string.format("#b%s#0",naev.keyGet("stealth"))))
   g(string.format(_([["So as long as you stealth with %s and stay away from ships, you won't be scanned and the cake will be alright."]]),
      string.format("#b%s#0",naev.keyGet("stealth"))))

   vn.label("notut")
   g(string.format(_([["Great. One second, let me get the cake."
They go to the restroom and come back holding a nondescript brown box that seems to have 'Cake' hastily scribbled on it. They promptly hand it over to you while looking both ways.
"OK, so that's it. Make sure to take this to %s in the %s system, and watch out for scanning!"]]), destplanet, destsys))
   vn.na(_("As leave them behind you can hear them let out a big sigh of what you can only assume is relief."))
   vn.run()

   if not accepted then
      return
   end

   misn.accept()

   local c = misn.cargoNew( N_("Cake"), N_("A cake that is supposedly sensitive to scanning radiation. Don't let anyone scan it.") )
   c:illegalto( {"Empire", "Dvaered", "Soromid", "Sirius", "Za'lek"} )
   carg_id = misn.cargoAdd( c, 0 )

   misn.osdCreate( _("Deliver Cake"), { string.format(_("Fly to %s in the %s system without getting scanned"), destplanet, destsys) } )

   misn.setTitle(_("Deliver Cake"))
   misn.setReward( creditstring(reward) )
   misn.setDesc( string.format(_("Deliver a cake to %s in the %s system. Apparently it has a special frosting and will be damaged if you are scanned. Use stealth to avoid getting scanned."), destplanet, destsys ) )
   misn.markerAdd(destsys)

   hook.land( "land" ) -- only hook after accepting
end

-- Land hook
function land()
   if planet.cur() ~= destplanet then
      return
   end
     
   vn.clear()
   vn.scene()
   local b = vn.newCharacter( receivername, { image=receiverimage } )
   vn.na(_("After you land you are promptly greeted by a burly looking individual."))
   b(_([["So you're the guy Sam sent? That weasel, I knew he wasn't cut out for this work."]]))
   vn.na(_([[You guess that this is the person you were supposed to deliver the cake to and hand it over.]]))
   b(_([[They take a look at the word 'Cake' written on the box and burst into laughter. After a while they calm down and turn to you.
"This was his idea wasn't it? Always been a bit weird that one."]]))
   b(_([["I guess you're not naïve enough to think this is a cake right?"
They chuckle.
"Anyway, you did good work bringing it here. I'll get you your reward wired and if you're interested in doing new jobs look into the mission computers. I've white-listed you to act as a courier."]]))
   b(_([["Take care."
As they lumber away, you suddenly notice that quite a few suspicious figures in the background disappear and follow them away. What have you gotten into?]]))
   vn.run()

   player.pay(reward)

   -- increase faction
   faction.modPlayerSingle("Pirate", rnd.rnd(2, 4))
   misn.finish(true)
end
