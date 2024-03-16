--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 02">
 <unique />
 <priority>0</priority>
 <chance>0</chance>
 <location>None</location>
 <notes>
  <done_evt>Onion Society 02 Trigger</done_evt>
  <campaign>Onion Society</campaign>
 </notes>
</mission>
--]]
--[[
   Onion02
--]]
local fmt = require "format"
local vn = require "vn"
local onion = require "common.onion"
local strmess = require "strmess"
local dv = require "common.dvaered"

local dstspb1, dstsys1 = spob.getS("Ulios")
local dstspb2, dstsys2 = spob.getS("The Frontier Council")
local dstspb3, dstsys3 = spob.getS("DVNN Central")
local money_reward = onion.rewards.misn02

local title01 = _("Onion Delivery")
local title02 = _("Onion's Revenge")

--[[
   Mission States
   0: mission accepted
   1: item picked up
   2: landed on destspb2
--]]
mem.state = 0

-- Create the mission
function create()
   -- Automatically accepted
   misn.accept()

   misn.setTitle(title01)
   misn.setDesc(fmt.f(_([[You have been tasked with picking up something from {spb1} ({sys1} system) and delivering it to {spb2} ({sys2} system).]]),
      {spb1=dstspb1, sys1=dstsys1, spb2=dstspb2, sys2=dstsys2}))
   misn.setReward(_("Unknown"))

   misn.osdCreate( title01, {
      fmt.f(_([[Pick up the cargo at {spb} ({sys})]]), {
         spb=dstspb1, sys=dstsys1 }),
      fmt.f(_([[Deliver the cargo to {spb} ({sys})]]), {
         spb=dstspb2, sys=dstsys2 }),
   } )

   misn.markerAdd( dstspb1 )
   hook.land("land")
   hook.enter("enter")
end

function land ()
   if mem.state==0 and spob.cur()==dstspb1 then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(fmt.f(_([[You land on the {spb1} spacedock and find a puzzled dockworker holding a small package outside your ship. It seems like something is confusing them, but they quickly give you the package and disappear before you can answer anything. Looks like they just wanted to get it over with.]]),
         {spb1=dstspb1}))
      vn.na(fmt.f(_([[The box looks similar to the one you delivered to {spb}. It seems to be sealed tight and emits some sort of faint beep at somewhat random intervals.]]),
         {spb=spob.get("Gordon's Exchange")}))
      vn.menu{
         {_([[Get on your ship]]), "01_ship"},
         {_([[Try to open the box]]), "01_box"},
      }

      vn.label("01_ship")
      vn.na(fmt.f(_([[You decide it's best to leave sleeping dogs lie, and take the box with you to the ship. Time to head to {spb2} to finish the job.]]),
         {spb2=dstspb2}))
      vn.done()

      vn.label("01_box")
      vn.na(fmt.f(_([[You try to open the box by hand, but find out it's too hard to open. Looks like whoever packed it really did not want it to be opened. You try to use several utensils to open it, but figure there's no easy way to open it without damaging it. It may be best to just get this over with. Time to head to {spb2} to finish the job.]]),
         {spb2=dstspb2}))

      vn.run()

      local c = commodity.new( N_("Another Small Box"), N_("Another suspicious box sealed tight. You think you can hear a faint beeping sound occasionally.") )
      mem.carg_id = misn.cargoAdd( c, 0 )
      misn.osdActive(2)
      misn.markerRm()
      misn.markerAdd( dstspb2 )
      mem.state = 1

   elseif mem.state==1 and spob.cur()==dstspb2 then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(fmt.f(_([[You land on {spb}, however, before you can get off your ship, your holoscreen flashes on.]]),
         {spb=dstspb2}))

      vn.music( onion.loops.circus )
      local ogre = onion.vn_onion()
      vn.appear( ogre, "electric" )
      ogre:rename(_("Ogre"))
      ogre(_([["How are you gentlemen?"]]))
      vn.menu{
         {_([["I haven't even got off the ship..."]]), "01_cont"},
         {_([["I'm not a gentleman!"]]), "01_cont"},
         {_([[Give out a big sigh.]]), "01_cont"},
      }

      vn.label("01_cont")
      ogre(_([["Err, is this plugged in? Wait..."]]))

      vn.move( ogre, "right" )
      local l337 = onion.vn_l337b01{pos="left"}
      vn.music( onion.loops.hacker )
      vn.appear( l337, "electric" )
      l337:rename(_("New Onion?"))
      l337(_([["And we're live!"
They pause for emphasis.]]))
      l337(_([["How are you doing Ogre? Enjoying your free trial of l337 b01's script kiddy's deck?"]]))
      ogre(_([["Wait, what!?! How did you!"
You hear the sound of mashing keys on a console.
"Shit, it's not disconnecting. What have you done?"]]))
      l337(_([["Ah, to be naïve once again like our dear Ogre. It's elemental, did you not care to check for back doors? Quite brave, as expected from House Dvaered"]]))
      ogre(_([["Shit shit shit!"]]))
      l337(_([["So much profanity. A bit flustered are we not? But of course, we were caught impersonating the Onion Society, were we not?"]]))
      ogre(_([[You hear yelling over the channel.]]))
      l337(_([["Wow, quite noisy today. Might as well get this over with before I get another headache."]]))
      l337(_([["Graeme Tildor of Haldr, consider yourself peeled!"]]))
      l337(_([["Ah bollocks, they've run out of their room. Not a good idea. notasockpuppet is going to have them for lunch. Oh well. Might as well close their connection."]]))
      vn.disappear( ogre, "electric" )
      vn.move( l337, "center" )
      vn.menu{
         {_([["What was that?"]]), "02_cont"},
         {_([["Where's Ogre?"]]), "02_cont"},
         {_([["Onion Society?"]]), "02_cont"},
      }

      vn.label("02_cont")
      l337(_([["You really had no idea what you were getting dragged into, did you? I'm l337 b01."]]))
      l337:rename(_("l337 b01"))
      l337(_([["Anyway, tl;dr, Ogre was a Dvaered stump, trying to mess with the Frontier and posing as part of the Onion Society. That's a big no-no."]]))
      l337(_([["They've been peeled, err, have had their personal information dumped on the darkwebs. Quite dangerous for them. Pretty sure notasockpuppet has dealt with them already. They have no tolerance for people messing with the Onion Society."]]))
      l337(_([["Now all that's left is to do the opposite of what Ogre wanted to do. Jam and mess with the frontier? Sounds like it's time to say hello te our friends at the Dvaered mass media."]]))
      l337(fmt.f(_([[The transmitter you're carrying should be enough for me to work with. On to {spb}!"]]),
         {spb=dstspb3}))
      vn.disappear( l337, "electric" )
      vn.na(_([[You have no idea what you're getting into, but it seems like you have no choice now but to play along.]]))
      vn.run()

      misn.markerRm()
      misn.markerAdd( dstspb3 )
      misn.osdCreate( title02, {
         fmt.f(_([[Deliver the cargo to {spb} ({sys})]]), {
            spb=dstspb3, sys=dstsys3 }),
      } )
      mem.state = 2

   elseif mem.state==2 and spob.cur()==dstspb3 then

      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(fmt.f(_([[You land on {spb} and get off your ship with the box in hand to deliver it as told.]]),
         {spb=dstspb3}))
      vn.na(_([[Before you reach the drop-off point, as you are crossing the spacedocks, the lights start to flicker a bit. That was fast.]]))

      local l337 = onion.vn_l337b01()
      local warlords = rnd.permutation( dv.warlords() )
      vn.music( "snd/sounds/songs/news.ogg" ) -- TODO 8-bit news?
      vn.appear( l337, "electric" )
      l337(_([["We interrupt you to bring you a recent news flash!"]]))
      l337(fmt.f(_([["House Dvaered economy hit a new low as {warlord1} and {warlord2} have been found to be hoarding gauss guns and railguns, which will likely cause a shortage across House Dvaered."]]),
         {warlord1=warlords[1], warlord2=warlords[2]}))
      l337(_([["Supplies are forecasted to only last 10 periods as Warlords scramble to supply their endless feuds. Dvaered High Command has denied the situation, claiming that the free market will regulate itself."]]))
      l337(fmt.f(_([["{warlord} has called the statement by Dvaered High Command, 'bollocks', and threatened to fly over there themselves and shell the station until it becomes a ball of molten steel."]]),
         {warlord=warlords[3]}))
      l337(_([["Until the situation clears up, we can only recommend you to acquire as many weapons as possible, and remember, don't panic. That was all."]]))
      vn.music( "snd/sounds/crowdpanic03.ogg" )
      vn.disappear( l337, "electric" )

      vn.na(_([[As expected, chaos erupts around you as everyone tries to get in their ship to fly to the nearest outfitter to acquire more weapons, lest they be relegated to a life of pacifism.]]))
      vn.na(_([[You return to your ship to try to wait for things to settle down, as you hear a large explosion in the distance and the station seems to tilt slightly. It seems like l337 b01 really knows how to rile up the Dvaered.]]))
      vn.na(_([[You are sitting at your console, overviewing the frenzy outside, when your holodeck starts up.]]))

      vn.music( onion.loops.hacker )
      vn.appear( l337, "electric" )
      l337(_([["Hahaha! Man, what a rush! Can't believe they made it so easy to communicate with all the boar space. I would have kept going, but they apparently blew up the entire DVNN mainframe to stop the transmission. What barbarism! Quite effective though."]]))
      l337(_([["Hope the boar intelligence will get the message to not mess with the Onion Society. Although I worry they're too thick-headed for it to get across."]]))
      l337(_([["You should also learn to trust people less, that Ogre was playing you as a tool. Nobody is what they seem on the grid."]]))
      l337(_([["I've been looking for a hand to help me out off grid. You look like you feel the job."]]))
      l337(_([["Until we meet again. Toodaloo〜♪."]]))
      vn.music()
      vn.disappear( l337, "electric" )

      vn.na(_([[The holodeck goes still, and you receive a notification of a credit transfer.]]))
      vn.sfxVictory()
      vn.func( function ()
         player.pay( money_reward )
      end )
      vn.na( fmt.reward(money_reward) )

      vn.run()

      misn.finish(true)
   end
end

local hacked_plts = {}
local timer
function enter ()
   if mem.state==1 then
      if timer then
         hook.rm(timer)
      end
      hacked_plts = {}
      timer = hook.timer(3, "strange_things")
   end
end

function strange_things ()
   local fct_indep = faction.get("Independent")
   for k,p in ipairs(pilot.getInrange(player.pos(), 3000)) do
      local id = p:id()
      if not hacked_plts[id] and p:faction()==fct_indep and p:memory().natural and rnd.rnd() < 0.1 then
         p:control( true )
         p:follow( player.pilot() )
         p:effectAdd("Onionized")
         hook.timer( 15+rnd.rnd()*15, "undo_hack", p )
         hook.pilot( p, "hail", "hail_hack" )
         hook.pilot( p, "attacked", "undo_hack" )
      end
   end
   timer = hook.timer(1, "strange_things")
end

function undo_hack( p )
   if p and p:exists() then
      p:control( false )
      p:effectRm("Onionized")
   end
end

mem.hack_msg = 1
local hack_msglst = {
   _([["Do you know what you're doing?"]]),
   _([["You're just being used, a mere tool."]]),
   _([["Ugh, I'm going to have to do this the hard way, ain't I."]]),
   _([["Why does everyone have to be a pain in the ass?"]]),
   _([["I didn't develop this software to be crapped onto by script kiddies."]]),
   _([["Not looking forward to having to clean this up."]]),
   fmt.f(_([["I've got my eyes on you, see you on {spb}."]]),{spb=dstspb2}),
}
function hail_hack( p )
   if p:effectHas("Onionized") then
      vn.clear()
      vn.scene()
      local o = vn.newCharacter( onion.vn_onion() )
      vn.music( onion.loops.circus )
      vn.transition("electric")
      vn.na(_([[Once again an onion hologram appears before you.]]))
      o(strmess.messup(hack_msglst[mem.hack_msg], 0.3))
      vn.na(_([[As soon as you receive the message, the channel cuts out.]]))
      vn.done("electric")
      vn.run()

      mem.hack_msg = math.min( #hack_msglst, mem.hack_msg+1 )
      undo_hack(p)
      player.commClose()
   end
end
