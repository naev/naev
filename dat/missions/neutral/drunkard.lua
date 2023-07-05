--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Drunkard">
 <unique />
 <priority>4</priority>
 <chance>3</chance>
 <location>Bar</location>
 <cond>
   return require("misn_test").reweight_active()
 </cond>
 <notes>
  <tier>1</tier>
 </notes>
</mission>
--]]
--[[

  Drunkard
  Author: geekt

  A drunkard at the bar has gambled his ship into hock, and needs you to do a mission for him.

]]--
local fmt = require "format"
local neu = require "common.neutral"
local portrait = require "portrait"
local vn = require "vn"
local vntk = require "vntk"
local ccomm = require "common.comm"

local payment = 500e3

local willie -- Non-persistent state

local npc_name = _("Drunkard")
local npc_portrait = "neutral/unique/drunkard.webp"
local npc_image = portrait.getFullPath( npc_portrait )

local cargo_amount = 45

function create ()
   -- Note: this mission does not make any system claims.

   misn.setNPC( npc_name, npc_portrait, _("You see a drunkard at the bar mumbling about how he was so close to getting his break.") )  -- creates the drunkard at the bar

   -- Planets
   mem.pickupWorld, mem.pickupSys  = spob.getS("INSS-2")
   mem.delivWorld, mem.delivSys    = spob.getS("Darkshed")
   mem.origWorld, mem.origSys      = spob.cur()
end

function accept ()
   local accepted = false

   vn.clear()
   vn.scene()
   local drunk = vn.newCharacter( npc_name, {image=npc_image} )
   vn.transition()
   drunk(_([[You sit next to the drunk man at the bar and listen to him almost sob into his drink. "I was so close! I almost had it! I could feel it in my grasp! And then I messed it all up! Why did I do it? Hey, wait! You! You can help me!" The man grabs your collar. "How'd you like to make a bit of money and help me out? You can help me! It'll be good for you. It'll be good for me. It'll be good for everyone! Will you help me?"]]))
   vn.menu{
      {_([[Yes]]), "yes"},
      {_([[No]]), "no"},
   }

   vn.label("no")
   vn.done()

   vn.label("nospace")
   vn.na(fmt.f(_([[You neeed an additional {space} of free space if you want to accept the mission.]]),
      {space=fmt.tonnes(cargo_amount - player.pilot():cargoFree())}))
   vn.done()

   vn.label("yes")
   vn.func( function ()
      if player.pilot():cargoFree() < cargo_amount then
         vn.jump("nospace")
      end
   end )
   drunk(_([["Oh, thank the ancestors! I knew you would help me!" The man relaxes considerably and puts his arm around you. "Have a drink while I explain it to you.", he motions to the bartender to bring two drinks over. "You see, I know this countess. She's like...whoa...you know what I mean?", he nudges you. "But she's rich, like personal escort fleet rich, golden shuttles, diamond laser turrets rich."]]))
   drunk(_([[Well, occasionally she needs some things shipped that she can't just ask her driver to go get for her. So, she asks me to go get this package. I don't know what it is; I don't ask; she doesn't tell me; that's the way she likes it. I had just got off this 72 hour run through pirate infested space though, and I was all hopped up on grasshoppers without a hatch to jump. So I decided to get a drink or two and hit the hay. Turned out those drinks er two got a little procreation goin' on and turned into three or twelve. Maybe twenty. I don't know, but they didn't seem too liking to my gamblin', as next thing I knew, I was wakin' up with water splashed on my face, bein' tellered I gots in the hock, and they gots me ship, ye know?]]))
   drunk(fmt.f(_([[But hey, all yous gotta do is go pick up whatever it is she wants at {pickup_pnt} in the {pickup_sys} system. I doubt it's anything too hot, but I also doubt it's kittens and rainbows. All I ask is 25 percent. So just go get it, deliver it to {dropoff_pnt} in the {dropoff_sys} system, and don't ask any questions. And if she's there when you drop it off, just tell her I sent you. And don't you be lookin' at her too untoforward, or um, uh, you know what I mean." You figure you better take off before the drinks he's had take any more hold on him, and the bottle sucks you in.]]),
      {pickup_pnt=mem.pickupWorld, pickup_sys=mem.pickupSys, dropoff_pnt=mem.delivWorld, dropoff_sys=mem.delivSys} ) )
   vn.func( function () accepted = true end )
   vn.run()

   if not accepted then
      return
   end

   misn.accept()

   -- mission details
   misn.setTitle( _("Drunkard") )
   misn.setReward( _("More than it's worth!") )
   misn.setDesc( _("You've decided to help some drunkard at the bar by picking up some goods for some countess. Though you're not sure why you accepted.") )

   mem.pickedup = false
   mem.droppedoff = false

   mem.marker = misn.markerAdd( mem.pickupWorld, "low" )  -- pickup
   -- OSD
   misn.osdCreate( _("Help the Drunkard"), {
      fmt.f(_("Go pick up some goods at {pnt} in the {sys} system"), {pnt=mem.pickupWorld, sys=mem.pickupSys}),
      fmt.f(_("Drop off the goods at {pnt} in the {sys} system"), {pnt=mem.delivWorld, sys=mem.delivSys}),
   } )

   mem.landhook = hook.land ("land")
   mem.flyhook = hook.takeoff ("takeoff")
end

function land ()
   if spob.cur() == mem.pickupWorld and not mem.pickedup then
      if player.pilot():cargoFree() < cargo_amount then
         vntk.msg( _("No Room"), fmt.f(_([[You need at least {amount} of free space to pick up the cargo.]]), {amount=cargo_amount}))  -- Not enough space
         misn.finish()

      else
         vntk.msg( _("Deliver the Goods"), _([[You land on the planet and hand the manager of the docks the crumpled claim slip that the drunkard gave you, realizing now that you don't think he even told you his name. The man looks at the slip, and then gives you an odd look before motioning for the dockworkers to load up the cargo that's brought out after he punches in a code on his electronic pad.]]) )
         local c = commodity.new( N_("Goods"), N_("A package of unknown goods for delivery to a countess.") )
         mem.cargoID = misn.cargoAdd( c, cargo_amount )  -- adds cargo
         mem.pickedup = true

         misn.markerMove( mem.marker, mem.delivWorld )  -- destination

         misn.osdActive(2)  --OSD
      end
   elseif spob.cur() == mem.delivWorld and mem.pickedup and not mem.droppedoff then
      vn.clear()
      vn.scene()
      vn.transition()
      vn.na(_([[You finally arrive at your destination, bringing your ship down to land right beside a beautiful woman with long blonde locks in a long extravagant gown. You know this must be the countess, but you're unsure how she knew you were going to arrive, to be waiting for you. When you get out of your ship, you notice there are no dock workers anywhere in sight, only a group of heavily armed private militia that weren't there when you landed.]]))
      vn.na(_([[You gulp as she motions to them without showing a hint of emotion. In formation, they all raise their weapons. As you think your life is about to end, every other row turns and hands off their weapon, and then marches forward and quickly unloads your cargo onto a small transport carrier, and march off. The countess smirks at you and winks before walking off. You breath a sigh of relief, only to realize you haven't been paid. As you walk back onto your ship, you see a card laying on the floor with simply her name, Countess Amelia Vollana.]]))
      vn.run()
      misn.cargoRm (mem.cargoID)

      misn.markerRm(mem.marker)
      misn.osdDestroy ()

      mem.droppedoff = true
   end
end

function takeoff()
   if system.cur() == mem.delivSys and mem.droppedoff then

      willie = pilot.add( "Mule", "Trader", player.pos() + vec2.new(-500,-500), _("Ol Bess") )
      willie:setFaction("Independent")
      willie:setFriendly()
      willie:setInvincible()
      willie:setVisplayer()
      willie:setHilight(true)
      willie:hailPlayer()
      willie:control()
      willie:moveto(player.pos() + vec2.new( 150, 75), true)
      vntk.msg( _("Takeoff"), _([[As you finish your takeoff procedures and once again enter the cold black of space, you can't help but feel relieved. You might not have gotten paid, but you're just glad to still be alive. Just as you're about to punch it to the jump gate to get as far away from whatever you just dropped off, you see the flashing light of an incoming hail.]]) )
      mem.hailhook = hook.pilot(willie, "hail", "hail")
   end
end

function hail()
   vn.clear()
   vn.scene()
   local p = ccomm.newCharacter( vn, willie )
   vn.transition()
   p(_([["Hello again. It's Willie. I'm just here to inform you that the countess has taken care of your payment and transferred it to your account. And don't worry about me, the countess has covered my portion just fine. I'm just glad to have Ol' Bessy here back."]]))

--   eventually I'll implement a bonus
--   p(fmt.f(_([["Oh, and she put in a nice bonus for you of {credits} for such a speedy delivery."]]), {credits=fmt.credits(mem.bonus)}))

   vn.disappear( p )

   vn.sfxVictory()
   vn.func( function ()
      player.pay( payment )
   end )
   vn.na(fmt.f(_([[You check your account balance as he closes the comm channel to find yourself {credits} richer. Just being alive felt good, but this feels better. You can't help but think that she might have given him more than just the 25 percent he was asking for, judging by his sunny disposition. At least you have your life though.]]),
      {credits=fmt.credits(payment)} ) )
   vn.na(fmt.reward(payment))

   vn.run()

   willie:setVisplayer(false)
   willie:setHilight(false)
   willie:setInvincible(false)
   willie:hyperspace()

   mem.bonus = 0
   neu.addMiscLog( _([[You helped some drunkard deliver goods for some countess. You thought you might get killed along the way, but you survived and got a generous payment.]]) )

   player.commClose()
   misn.finish(true)
end
