--[[

   Slow Merchant
   Author: PhoenixRiver (from an idea on the wiki)

   A merchant with a slow ship suddenly realizes he can't make the delivery and
   implores the player to do it for him. Since he has to look good with his
   employers he'll pay the player a bonus if he does it.

   Note: Variant of the Drunkard and Rush Cargo missions combined

]]--

include "numstring.lua"
include "jumpdist.lua"
include "numstring.lua"

lang = naev.lang()
if lang == "es" then
elseif lang == "de" then
else -- default to English
   -- Bar Description
   bar_desc = "You see a merchant at the bar in a clear state of anxiety."

   -- Mission Details
   misn_title = "Anxious Merchant"
   misn_reward = "Peace of mind (and %d credits)!"
   misn_desc = "You decided to help a fraught merchant at a bar by delivering some goods to %s for him."

   -- OSD
   osd_title = "Help the Merchant"
   osd_desc = {}
   osd_desc[1] = "Drop off the goods at %s in the %s system. You have %s remaining."
   osd_desc[2] = "Drop off the goods at %s in the %s system. You are late."

   -- Cargo Details
   cargo = "Goods"

   title = {}  --stage titles
   text = {}   --mission text

   title[1] = "Spaceport Bar"
   text[1] = [[    You sit next to the drunk man at the bar and listen to him almost sob into his drink. "I was so close! I almost had it! I could feel it in my grasp! And then I messed it all up! Why did I do it? Hey, wait! You! You can help me!" The man grabs your collar. "How'd you like to make a bit of money and help me out? You can help me! It'll be good for you, it'll be good for me, it'll be good for everyone! Will you help me?"]]

   title[2] = "Pick Up the Countess' Goods"
   text[2] = [[    "Oh, thank the ancestors! I knew you were the man to help me!" The man relaxes considerably and puts his arm around you. "Have a drink while I explain it to you.", he motions to the bartender to bring two drinks over. "You see, I know this countess, she's like...whoa...you know what I mean?", he nudges you. "But she's rich, like personal escort fleet rich, golden shuttles, diamond laser turrets rich.
    Well, occasionally she needs some things shipped that she can't just ask her driver to go get for her. So, she asks me to go get this package. I don't know what it is; I don't ask; she doesn't tell me; that's the way she likes it. I had just got off this 72 hour run through pirate infested space though, and I was all hopped up on grasshoppers without a hatch to jump. So I decided to get a drink or two and hit the hay. Turned out those drinks er two got a little procreatin goin on and turned into three or twelve. Maybe twenty. I don't know, but they didn't seem too liking to my gamblin, as next thing I knew, I was wakin up with water splashed on my face, bein tellered I gots in the hock, and they gots me ship, ye know? But hey, all yous gotta do is go pick up whatever it is she wants at %s in the %s system. I doubt it's anything too hot, but I also doubt it's kittens and rainbows. All I ask is 25 percent. So just go get it, deliver it to %s in the %s system, and don't ask any questions. And if she's there when you drop it off, just tell her I sent you. And don't you be lookin at her too untoforward, or um, uh, you know what I mean." You figure you better take off before the drinks he's had take any more hold on him, and the bottle sucks you in.]]

   title[3] = "Deliver the Goods"
   text[3] = [[    You land on the planet and hand the manager of the docks the crumpled claim slip that the drunkard gave you, realizing now that you don't think he even told you his name. The man looks at the slip, and then gives you an odd look before motioning for the dockworkers to load up the cargo that's brought out after he punches in a code on his electronic pad.]]

   title[4] = "Success"
   text[4] = [[    You finally arrive at your destination, bringing your ship down to land right beside a beautiful woman with long blonde locks in a long extravagant gown. You know this must be the countess, but you're unsure how she knew you were going to arrive, to be waiting for you. When you get out of your ship, you notice there are no dock workers anywhere in sight, only a group of heavily armed private militia that weren't there when you landed.
    You gulp as she motions to them without showing a hint of emotion. In formation, they all raise their weapons. As you think your life is about to end, every other row turns and hands off their weapon, and then marches forward and quickly unloads your cargo onto a small transport carrier, and march off. The countess smirks at you and winks before walking off. You breath a sigh of relief, only to realize you haven't been paid. As you walk back onto your ship, you see a card laying on the floor with simply her name, Countess Amelia Vollana.]]

   title[5] = "Takeoff"
   text[5] = [[    As you finish your takeoff procedures and once again enter the cold black of space, you can't help but feel relieved. You might not have gotten paid, but you're just glad to still be alive. Just as you're about to punch it to the jump gate to get as far away from whatever you just dropped off, you see the flashing light of an incoming hail.]]

   title[6] = "Drunkard's Call"
   text[6] = [[    "Hello again. It's Willie. I'm just here to inform you that the countess has taken care of your payment and transfered it to your account. And don't worry about me, the countess has covered my portion just fine. I'm just glad to have Ol' Bessy here back."]]

   title[7] = "Bonus"
   text[7] = [["Oh, and she put in a nice bonus for you of %d credits for such a speedy delivery."]]

   title[8] = "Check Account"
   text[8] = [[    You check your account balance as he closes the comm channel to find yourself %s credits richer. Just being alive felt good, but this feels better. You can't help but think that she might have given him more than just the 25 percent he was asking for, judging by his sunny disposition. At least you have your life though.]]

   full_title = "No Room"
   full_text = [[You don't have enough cargo space to accept this mission.]]

   slow_title = "Too slow"
   slow_text = [[The goods have to arrive by %s but it will take until at least %s for your ship to reach %s.

Accept the mission anyway?]]
end

function create()
   -- Note: this mission does not make any system claims.

   -- Calculate the route, distance, jumps and cargo to take
   dest_planet, dest_sys, num_jumps, travel_dist, cargo, tier = cargo_calculateRoute()
   if dest_planet == nil then
      misn.finish(false)
   end
   orig_planet, orig_sys = planet.cur()

   local merchant_type = {}
   mechant_type = {"neutral/male1",
                   "neutral/female1",
                   "neutral/thief1",
                   "neutral/thief3"
                  }
   merchant = rnd.rnd(1, #merchant_type)
   misn.setNPC( "Merchant", merchant )  -- creates the merchant at the bar
   misn.setDesc( bar_desc )           -- merchant's description

   stu_distance = 0.2 * travel_dist
   stu_jumps = 10300 * num_jumps
   stu_takeoff = 10300
   time_limit = time.get() + time.create(0, 0, stu_distance + stu_jumps + stu_takeoff)
   payment = stu_distance + (stu_jumps / 10)
   cargo_size = tier^3
end

function accept()
   if not tk.yesno(title[1], text[1]:format(cargo_size, travel_dist, num_jumps, (time_limit - time.get()):str(), dest_planet:name(), dest_sys:name(), payment) then
      misn.finish()
   end
   if player.pilot():cargoFree() < cargo_size then
      tk.msg(full_title, full_text)  -- Not enough space
      misn.finish()
   end
   pilot.cargoAdd(player.pilot(), cargo, cargo_size) 
   local player_best = cargoGetTransit(time_limit, num_jumps, travel_dist)
   pilot.cargoRm(player.pilot(), cargo, cargo_size) 
   if time_limit < player_best then
      if not tk.yesno(slow_title, slow_text:format(time_limit:str(), player_best:str(), destplanet:name())) then
         misn.finish()
      end
   end
      
   misn.accept()

   -- mission details
   misn.setTitle(misn_title)
   misn.setReward(misn_reward:format(payment))
   misn.setDesc(misn_desc:format(dest_planet:name()))
   marker = misn.markerAdd(dest_sys, "low") -- destination
   cargo_ID = misn.cargoAdd(cargo, cargo_size) -- adds cargo

   -- OSD
   osd_msg = osd_desc[1]:format(dest_planet:name(), dest_sys:name(), time_limit:str())
   osd = misn.osdCreate(osd_title, osd_msg)

   tk.msg(title[2], text[2]:format(dest_planet:name(), dest_sys:name()))

   intime = true
   land_hook = hook.land("land")
   tick_hook = hook.tick(time.create(0, 0, 42), "tick") -- 42STU per tick
end

function land()
   if planet.cur() == dest_planet then
      if intime then
         tk.msg(title[4], text[4])
         player.pay(payment)
      else
         tk.msg(title[5], text[5])
      end
      misn.cargoRm(cargo_ID)
      misn.osdDestroy(osd)
      misn.markerRm(marker)
      hook.rm(land_hook)
      hook.rm(tick_hook)
      misn.finish(true)
   end
end

function tick()
    if time_limit >= time.get() then -- still in time
        osd_msg = osd_desc[1]:format(dest_planet:name(), dest_sys:name(), time_limit:str())
    else -- missed deadline
        osd_msg = osd_desc[2]:format(dest_planet:name(), dest_sys:name())
        intime = false
        hook.rm(tick_hook)
    end
    misn.osdCreate(osd_title, osd_msg)
end

function abort()
   misn.cargoRm(cargo_ID)
   misn.osdDestroy(osd)
   misn.markerRm(marker)
   hook.rm(land_hook)
   hook.rm(tick_hook)
   misn.finish(false)
end