--[[

   Anxious Merchant
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
   text[1] = [[    As you sit next to the merchant"]]

   title[2] = "Pick Up the Countess' Goods"
   text[2] = [[    "]]

   title[3] = "Deliver the Goods"
   text[3] = [[    You]]

   title[4] = "Success"
   text[4] = [[    You]]

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
         tk.msg(title[3], text[3])
         player.pay(payment)
      else
         tk.msg(title[4], text[4])
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