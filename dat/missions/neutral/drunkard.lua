--[[

  Drunkard
  Author: geekt

  A drunkard at the bar has gambled his ship into hock, and needs you to do a mission for him.

]]--

include "dat/scripts/numstring.lua"

-- Bar Description
bar_desc = _("You see a drunkard at the bar mumbling about how he was so close to getting his break.")

-- Mission Details
misn_title = _("Drunkard")
misn_reward = _("More than it's worth!")
misn_desc = _("You've decided to help some drunkard at the bar by picking up some goods for some countess. Though you're not sure why you accepted.")

-- OSD
OSDtitle = _("Help the Drunkard")
OSDdesc = {}
OSDdesc[1] = _("Go pickup some goods at %s in the %s system.")
OSDdesc[2] = _("Drop off the goods at %s in the %s system.")

payment = 500000

-- Cargo Details
cargo = "Goods"
cargoAmount = 45

title = {}  --stage titles
text = {}   --mission text

title[1] = _("Spaceport Bar")
text[1] = _([[You sit next to the drunk man at the bar and listen to him almost sob into his drink. "I was so close! I almost had it! I could feel it in my grasp! And then I messed it all up! Why did I do it? Hey, wait! You! You can help me!" The man grabs your collar. "How'd you like to make a bit of money and help me out? You can help me! It'll be good for you, it'll be good for me, it'll be good for everyone! Will you help me?"]])

title[2] = _("Pick Up the Countess' Goods")
text[2] = _([["Oh, thank the ancestors! I knew you were the man to help me!" The man relaxes considerably and puts his arm around you. "Have a drink while I explain it to you.", he motions to the bartender to bring two drinks over. "You see, I know this countess, she's like...whoa...you know what I mean?", he nudges you. "But she's rich, like personal escort fleet rich, golden shuttles, diamond laser turrets rich.
    Well, occasionally she needs some things shipped that she can't just ask her driver to go get for her. So, she asks me to go get this package. I don't know what it is; I don't ask; she doesn't tell me; that's the way she likes it. I had just got off this 72 hour run through pirate infested space though, and I was all hopped up on grasshoppers without a hatch to jump. So I decided to get a drink or two and hit the hay. Turned out those drinks er two got a little procreatin goin on and turned into three or twelve. Maybe twenty. I don't know, but they didn't seem too liking to my gamblin, as next thing I knew, I was wakin up with water splashed on my face, bein tellered I gots in the hock, and they gots me ship, ye know? But hey, all yous gotta do is go pick up whatever it is she wants at %s in the %s system. I doubt it's anything too hot, but I also doubt it's kittens and rainbows. All I ask is 25 percent. So just go get it, deliver it to %s in the %s system, and don't ask any questions. And if she's there when you drop it off, just tell her I sent you. And don't you be lookin at her too untoforward, or um, uh, you know what I mean." You figure you better take off before the drinks he's had take any more hold on him, and the bottle sucks you in.]])

title[3] = _("Deliver the Goods")
text[3] = _([[You land on the planet and hand the manager of the docks the crumpled claim slip that the drunkard gave you, realizing now that you don't think he even told you his name. The man looks at the slip, and then gives you an odd look before motioning for the dockworkers to load up the cargo that's brought out after he punches in a code on his electronic pad.]])

title[4] = _("Success")
text[4] = _([[You finally arrive at your destination, bringing your ship down to land right beside a beautiful woman with long blonde locks in a long extravagant gown. You know this must be the countess, but you're unsure how she knew you were going to arrive, to be waiting for you. When you get out of your ship, you notice there are no dock workers anywhere in sight, only a group of heavily armed private militia that weren't there when you landed.
    You gulp as she motions to them without showing a hint of emotion. In formation, they all raise their weapons. As you think your life is about to end, every other row turns and hands off their weapon, and then marches forward and quickly unloads your cargo onto a small transport carrier, and march off. The countess smirks at you and winks before walking off. You breath a sigh of relief, only to realize you haven't been paid. As you walk back onto your ship, you see a card laying on the floor with simply her name, Countess Amelia Vollana.]])

title[5] = _("Takeoff")
text[5] = _([[As you finish your takeoff procedures and once again enter the cold black of space, you can't help but feel relieved. You might not have gotten paid, but you're just glad to still be alive. Just as you're about to punch it to the jump gate to get as far away from whatever you just dropped off, you see the flashing light of an incoming hail.]])

title[6] = _("Drunkard's Call")
text[6] = _([["Hello again. It's Willie. I'm just here to inform you that the countess has taken care of your payment and transfered it to your account. And don't worry about me, the countess has covered my portion just fine. I'm just glad to have Ol' Bessy here back."]])

title[7] = _("Bonus")
text[7] = _([["Oh, and she put in a nice bonus for you of %d credits for such a speedy delivery."]])

title[8] = _("Check Account")
text[8] = _([[You check your account balance as he closes the comm channel to find yourself %s credits richer. Just being alive felt good, but this feels better. You can't help but think that she might have given him more than just the 25 percent he was asking for, judging by his sunny disposition. At least you have your life though.]])

title[9] = _("No Room")
text[9] = _([[You don't have enough cargo space to accept this mission.]])

function create ()
   -- Note: this mission does not make any system claims.

   misn.setNPC( _("Drunkard"), "neutral/unique/drunkard" )  -- creates the drunkard at the bar
   misn.setDesc( bar_desc )           -- drunkard's description

   -- Planets
   pickupWorld, pickupSys  = planet.getLandable("INSS-2")
   delivWorld, delivSys    = planet.getLandable("Darkshed")
   if pickupWorld == nil or delivWorld == nil then -- Must be landable
      misn.finish(false)
   end
   origWorld, origSys      = planet.cur()

--   origtime = time.get()
end

function accept ()
   if not tk.yesno( title[1], text[1] ) then
      misn.finish()

   elseif player.pilot():cargoFree() < 45 then
      tk.msg( title[9], text[9] )  -- Not enough space
      misn.finish()

   else
      misn.accept()

      -- mission details
      misn.setTitle( misn_title )
      misn.setReward( misn_reward )
      misn.setDesc( misn_desc:format(pickupWorld:name(), pickupSys:name(), delivWorld:name(), delivSys:name() ) )

      -- OSD
      OSDdesc[1] =  OSDdesc[1]:format(pickupWorld:name(), pickupSys:name())
      OSDdesc[2] =  OSDdesc[2]:format(delivWorld:name(), delivSys:name())

      pickedup = false
      droppedoff = false

      marker = misn.markerAdd( pickupSys, "low" )  -- pickup
      misn.osdCreate( OSDtitle, OSDdesc )  -- OSD

      tk.msg( title[2], text[2]:format( pickupWorld:name(), pickupSys:name(), delivWorld:name(), delivSys:name() ) )

      landhook = hook.land ("land")
      flyhook = hook.takeoff ("takeoff")
   end
end

function land ()
   if planet.cur() == pickupWorld and not pickedup then
      if player.pilot():cargoFree() < 45 then
         tk.msg( title[9], text[9] )  -- Not enough space
         misn.finish()

      else

         tk.msg( title[3], text[3] )
         cargoID = misn.cargoAdd(cargo, cargoAmount)  -- adds cargo
         pickedup = true

         misn.markerMove( marker, delivSys )  -- destination

         misn.osdActive(2)  --OSD
      end
   elseif planet.cur() == delivWorld and pickedup and not droppedoff then
      tk.msg( title[4], text[4] )
      misn.cargoRm (cargoID)

      misn.markerRm(marker)
      misn.osdDestroy ()

      droppedoff = true
   end
end

function takeoff()
   if system.cur() == delivSys and droppedoff then

      willie = pilot.add( "Trader Mule", "trader", player.pilot():pos() + vec2.new(-500,-500))[1]
      willie:rename(_("Ol Bess"))
      willie:setFaction("Civilian")
      willie:setFriendly()
      willie:setInvincible()
      willie:setVisplayer()
      willie:setHilight(true)
      willie:hailPlayer()
      willie:control()
      willie:goto(player.pilot():pos() + vec2.new( 150, 75), true)
      tk.msg( title[5], text[5] )
      hailhook = hook.pilot(willie, "hail", "hail")
   end
end

function hail()
   tk.msg( title[6], text[6] )

--   eventually I'll implement a bonus
--   tk.msg( title[7], text[7]:format( bonus ) )

   hook.timer("1", "closehail")
end

function closehail()
   bonus = 0
   player.pay( payment )
   tk.msg( title[8], text[8]:format( numstring(payment) ) )
   willie:setVisplayer(false)
   willie:setHilight(false)
   willie:setInvincible(false) 
   willie:hyperspace()
   misn.finish(true)
end

function abort()
   hook.rm(landhook)
   hook.rm(flyhook)
   if hailhook then hook.rm(hailhook) end
   misn.finish()
end
