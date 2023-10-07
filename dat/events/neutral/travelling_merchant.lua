--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Travelling Merchant">
 <location>enter</location>
 <chance>5</chance>
 <cond>
   if require("common.pirate").systemPresence() &lt; 100 or system.cur():presence("Independent") &lt; 100 then
      return false
   end
   if system.cur():tags().restricted then
      return false
   end
   if player.credits() &lt; 1e6 and not var.peek("travelling_trader_hailed") then
      return false
   end
   return true
 </cond>
</event>
--]]
--[[

   Travelling Merchant Event

Spawns a travelling merchant that can sell the player if interested.

--]]
local vn = require 'vn'
local fmt = require "format"
local love_shaders = require 'love_shaders'
local der = require "common.derelict"
local poi = require "common.poi"

local p, broadcastid, hailed_player, second_hail, timerdelay, broadcast_first -- Non-persistent state
local gen_outfits -- Forward declaration

local trader_name = _("Machiavellian Misi") -- Mireia Sibeko
local trader_image = "misi.png"
local trader_colour = {1, 0.3, 1}
local store_name = _("Machiavellian Misi's \"Fine\" Wares")
local broadcastmsg = {
   _("Machiavellian Misi's the name and selling fine shit is my game! Come get your outfits here!"),
   _("Get your fiiiiiiiine outfits here! Guaranteed 3 space lice or less or your money back!"),
   _("Recommended by the Emperor's pet iguana's third cousin! High quality outfits sold here!"),
   _("Best outfits in the universe! So freaking good that 50% of my clients lose their hair from joy!"),
   _("Sweeet sweet space outfits! Muaha hahaha ha ha ha erk…"),
   _("…and that's how I was able to get a third liver haha. Oops is this on? Er, nevermind that. Outfits for sale!"),
}

local misi_outfits
function create ()
   local scur = system.cur()

   -- Inclusive claim
   if not naev.claimTest( scur, true ) then evt.finish() end

   -- Check to see if a nearby spob is inhabited
   local function nearby_spob( pos )
      for _k,pk in ipairs(scur:spobs()) do
         if pk:services().inhabited and pos:dist( pk:pos() ) < 1000 then
            return true
         end
      end
      return false
   end

   -- Find uninhabited planet
   local planets = {}
   for _k,pk in ipairs(scur:spobs()) do
      if not pk:services().inhabited and not nearby_spob( pk:pos() ) then
         table.insert( planets, pk )
      end
   end
   local spawn_pos
   if #planets==0 then
      -- Try to find something not near any inhabited planets
      local rad = scur:radius()
      local tries = 0
      while tries < 30 do
         local pos = vec2.newP( rnd.rnd(0,rad*0.5), rnd.angle() )
         if not nearby_spob( pos ) then
            spawn_pos = pos
            break
         end
         tries = tries + 1
      end
      -- Failed to find anything
      if not spawn_pos then
         evt.finish()
      end
   else
      local pnt = planets[rnd.rnd(1,#planets)]
      spawn_pos = pnt:pos() + vec2.newP( pnt:radius()+100*rnd.rnd(), rnd.angle() )
   end

   local fctmisi = faction.dynAdd( "Independent", "fmisi", _("???"),
         {clear_enemies=true, clear_allies=true} )

   -- Create pilot
   p = pilot.add( "Mule", fctmisi, spawn_pos, trader_name )
   p:setFriendly()
   p:setInvincible()
   p:setVisplayer()
   p:setHilight(true)
   p:setActiveBoard(true)
   p:control()
   p:brake()

   -- Get outfits
   misi_outfits = gen_outfits()

   -- Set up hooks
   broadcast_first = false
   timerdelay = 10
   broadcastid = 1
   broadcastmsg = rnd.permutation( broadcastmsg )
   hook.timer( timerdelay, "broadcast" )
   hailed_player = false
   hook.pilot( p, "hail", "hail" )
   hook.pilot( p, "board", "board" )

   hook.jumpout("leave")
   hook.land("leave")
end

--event ends on player leaving the system or landing
function leave ()
    evt.finish()
end

function gen_outfits ()
   --[[
      Ideas
   * Vampiric weapon that removes shield regen, but regenerates shield by doing damage.
   * Hot-dog launcher (for sale after Reynir mission): does no damage, but has decent knockback and unique effect
   * Money launcher: does fairly good damage, but runs mainly on credits instead of energy
   * Mask of many faces: outfit that changes bonuses based on the dominant faction of the system you are in (needs event to handle changing outfit)
   * Weapon that does double damage to the user if misses
   * Weapon that damages the user each time it is shot (some percent only)
   * Space mines! AOE damage that affects everyone, but they don't move (useful for missions too!)
   --]]

   -- Always available outfits
   -- TODO add more
   local outfits = {
      'Air Freshener',
      'Valkyrie Beam',
      'Hades Torch',
   }

   -- Bonus for killing executors
   if player.outfitNum("Executor Shield Aura")<1 and (var.peek("executors_killed") or 0) >= 3 then
      table.insert( outfits, "Executor Shield Aura" )
   end

   -- TODO add randomly chosen outfits, maybe conditioned on the current system or something?

   -- Give mission rewards the player might not have for a reason
   local mission_rewards = {
      { "Drinking Aristocrat",      "Swamp Bombing" },
      { "The Last Detail",          "Sandwich Holder" },
      { "Prince",                   "Ugly Statue" },
      { "Destroy the FLF base!",    "Star of Valor" },
      { "Nebula Satellite",         "Satellite Mock-up" },
      { "The one with the Runaway", "Toy Drone" },
      { "Deliver Love",             "Love Letter" },
      --{ "Racing Skills 2",          "Racing Trophy" }, -- This is redoable so no need to give it again
      { "Operation Cold Metal",     "Left Boot" },
      { "Black Cat",                "Black Cat Doll" },
      { "Terraforming Antlejos 10", "Commemorative Stein" },
      { "DIY Nerds",                "Homebrew Processing Unit" },
   }
   local event_rewards = {
   }
   -- Special case: this mission has multiple endings, and only one gives the reward.
   if var.peek( "flfbase_intro" ) == nil and var.peek( "invasion_time" ) == nil then
      table.insert( mission_rewards, { "Disrupt a Dvaered Patrol", "Pentagram of Valor" } )
   end
   for i,r in ipairs(mission_rewards) do
      local m = r[1]
      local o = r[2]
      if player.misnDone(m) and player.outfitNum(o)<1 then
         table.insert( outfits, o )
      end
   end
   for i,r in ipairs(event_rewards) do
      local e = r[1]
      local o = r[2]
      if player.evtDone(e) and player.outfitNum(o)<1 then
         table.insert( outfits, o )
      end
   end

   -- Special items when POI are done, has to be boarded to do the data message
   if poi.data_get_gained() > 0 and var.peek("travelling_trader_boarded") then
      local olist = {
         "Veil of Penelope",
         "Daphne's Leap",
      }
      for k,v in ipairs(olist) do
         table.insert( outfits, v )
      end

      -- Defeated all the "Goatee" pirates
      if player.evtDone("Quai Pirates") and player.evtDone("Levo Pirates") and player.evtDone("Capricorn Pirates") and player.evtDone("Surano Pirates") and player.evtDone("Dendria Pirates") then
         table.insert( outfits, "Goatee Marker" )
      end

      -- Chapter 1
      if player.fleetCapacity() > 0 then
         table.insert( outfits, "Squadron Synchronizer Module" )
      end
   end

   -- Other special cases
   local rr = "Rackham's Razor"
   if player.outfitNum(rr) <= 0 and (var.peek("poi_red_rackham")>=3) then
      table.insert( outfits, rr )
   end

   return outfits
end

local newoutfits = false
function broadcast ()
   -- End the event if for any reason the trader stops existing
   if not p:exists() then
      evt.finish()
      return
   end

   -- Check to see if has new outfits
   for k,o in ipairs(misi_outfits) do
      if not var.peek("misi_o_"..o) then
         newoutfits = true
         break
      end
   end

   -- Cycle through broadcasts
   if broadcastid > #broadcastmsg then broadcastid = 1 end

   if not hailed_player and not var.peek('travelling_trader_hailed') then
      p:hailPlayer()
      hailed_player = true

   elseif poi.data_get_gained() > 0 and
         var.peek("travelling_trader_boarded") and
         not var.peek("travelling_trader_hail2") and
         not var.peek("travelling_trader_data") then
      p:hailPlayer()
      hailed_player = true
      second_hail = true

   end

   if not hailed_player and newoutfits and not broadcast_first then
      -- With new outfits point it out to the player
      p:comm(_("I have new wares you might want to see!"), true)
      player.autonavReset(1)
      broadcast_first = true
   else
      p:broadcast( broadcastmsg[broadcastid], true )
   end

   broadcastid = broadcastid+1
   timerdelay = timerdelay * 1.5
   hook.timer( timerdelay, "broadcast" )
end

function hail ()
   if not var.peek('travelling_trader_hailed') then
      vn.clear()
      vn.scene()
      local mm = vn.newCharacter( trader_name,
         { image=trader_image, color=trader_colour, shader=love_shaders.hologram() } )
      vn.transition("electric")
      mm:say( _([["Howdy Human! Er, I mean, Greetings! If you want to take a look at my wonderful, exquisite, propitious, meretricious, effulgent, … wait, what was I talking about? Oh yes, please come see my wares on my ship. You are welcome to board anytime!"]]) )
      vn.done("electric")
      vn.run()

      var.push('travelling_trader_hailed', true)
      player.commClose()
   elseif second_hail then
      vn.clear()
      vn.scene()
      local mm = vn.newCharacter( trader_name,
         { image=trader_image, color=trader_colour, shader=love_shaders.hologram() } )
      vn.transition("electric")
      mm:say(_([["Howdy Human! I have new propitiuous and meretricious wares available. Come see the wares on my ship!"]]))
      vn.done("electric")
      vn.run()

      var.push('travelling_trader_hail2', true)
      player.commClose()

      second_hail = false
   end
end

function board ()
   -- Boarding sound
   der.sfx.board:play()

   vn.clear()
   vn.scene()
   local mm = vn.newCharacter( trader_name, { image=trader_image, color=trader_colour } )
   vn.transition()
   if not var.peek('travelling_trader_boarded') then
      vn.na(_([[You open the airlock and are immediately greeted by an intense humidity and heat, almost like a jungle. As you advance through the dimly lit ship you can see all types of mold and plants crowing in crevices in the wall. Wait, was that a small animal scurrying around? Eventually you reach the cargo hold that has been re-adapted as a sort of bazaar. As you look around the mess of different wares, most seemingly to be garbage, you suddenly notice a mysterious figure standing infront of you. You're surprised at how you didn't notice them getting so close to you, almost like a ghost.]]))
      mm(_([[You stare dumbfounded at the figure who seems to be capturing your entire essence with a piercing gaze, when suddenly you can barely make out what seems to be a large grin.
"You look a bit funky for a human, but all are welcome at Misi's Fabulous Bazaar!"
They throw their hands up in the air, tossing what seems to be some sort of confetti. Wait, is that ship mold?]]))
      mm(_([["In my travels, I've collected quite a fair amount of rare and expensive jun… I mean trinkets from all over the galaxy. Not many appreciate my fine wares, so I peddle them as I travel around. If you see anything you fancy, I'll let it go for a fair price. You won't find such a good bargain anywhere else!"]]))
      var.push( "travelling_trader_boarded", true )
   else
      vn.na(_([[You open the airlock and are immediately greeted by an intense humidity and heat, almost like a jungle. As you advance through the dimly lit ship you can see all types of mold and plants crowing in crevices in the wall. Wait, was that a small animal scurrying around? Eventually you reach the cargo hold bazaar where Misi is waiting for you.]]))
      if poi.data_get_gained() > 0 and not var.peek("travelling_trader_data") then
         mm(_([[Suddenly they starts sniffing the air.
"Wait is that…?"
They get uncomfortably close
"Say, you smell like… something odd…]]))
         mm(_([[They seem to mutter something under their breath as the pace around trying to remember. Suddenly, they stop, turn around and point at you and exclaim:
"Data Matrices!"]]))
         mm(_([["You wouldn't have happened to come about some Data Matrices?"]]))
         vn.menu{
            {p_("not to come", [["No"]]), "matrices_no"},
            {p_("not to come", [["Yes"]]), "matrices_yes"},
            {_([["…"]]), "matrices_silent"},
         }

         vn.label("matrices_yes")
         mm(_([["I knew it! I would never forget that smell in a lifetime."]]))
         vn.jump("matrices_cont01")

         vn.label("matrices_no")
         mm(_([["Hah, I can clearly see through your lies. You can't fool my nose!"]]))
         vn.jump("matrices_cont01")

         vn.label("matrices_silent")
         mm(_([["Cat's got your tongue? No point in wasting breath when the answer is clear."]]))
         vn.jump("matrices_cont01")

         vn.label("matrices_cont01")
         mm(_([[They sniff the air as to confirm their findings.
"You haven't been able to de-encrypt them have you? Old technology is a bugger, and most of the cipher codes have been lost, not making it an easy task. I don't think the Encrypted Data Matrices will be of any use to you. However, if you're willing to trade, I have some better offerings that may pique your interest."]]))
         mm(_([["If you aren't interested in material goods, I also have some special services up my sleeves that should suit any of your ships. Just ask me about them."]]))
         vn.func( function ()
            var.push("travelling_trader_data",true)
         end )
      else
         local sillyphrases = {
            _("Guaranteed to not smell like they were plundered from some stinky old derelict!"),
            _("Part of a complete breakfast!"),
            _("Buy one and get zero free!"),
            _("Now free of flesh-eating bacteria!"),
            _("Goes well with a side of rice!"),
            _("Now with fewer side effects!"),
         }

         mm(fmt.f(_([["I haven't seen you in a while old friend! I've gotten some new wares you may want to see. Always the best quality! {sillyphrase}"]]),
            {sillyphrase = sillyphrases[ rnd.rnd(1,#sillyphrases) ]}))
      end
   end

   vn.label("menu")
   vn.na(_("What do you wish to do?"))
   vn.label("menu_direct")
   vn.menu( function ()
      local opts = {
         { _("Leave"), "leave" },
      }
      if newoutfits then
         table.insert( opts, 1, { _("Shop (#rNew Wares!#0)"), "bazaar" } )
      else
         table.insert( opts, 1, { _("Shop"), "bazaar" } )
      end
      if var.peek("travelling_trader_data") then
         table.insert( opts, 2, { _("Special Services"), "special" } )
      end
      return opts
   end )

   vn.label("bazaar")
   vn.func( function ()
      -- Mark outfits as seen
      for k,o in ipairs(misi_outfits) do
         var.push("misi_o_"..o,true)
      end
      newoutfits = false
      -- Open store
      tk.merchantOutfit( store_name, misi_outfits )
   end )
   vn.jump("menu")

   local upgrade_cost = 2
   local upgrade_list = {
      special_necessity = outfit.get("Machiavellian Necessity"),
      special_fortune   = outfit.get("Machiavellian Fortune"),
      special_virtue    = outfit.get("Machiavellian Virtue"),
   }
   vn.label("special")
   mm(fmt.f(_([["So you're interested in my special services. Quite a bargain might I say. Each services costs only {cost}."]]),
      {cost=poi.data_str(upgrade_cost)}))
   vn.menu( function ()
      local opts = {
         { _("Info"), "special_info" },
         { _("Back"), "menu" },
      }
      for s,o in pairs( upgrade_list ) do
         table.insert( opts, 1, { fmt.f(_("{intrinsic} Service"),{intrinsic=o}), s } )
      end
      return opts
   end )
   vn.jump("menu")

   local replacement = nil
   for s,o in pairs( upgrade_list ) do
      vn.label( s )
      local upgrade = o
      vn.func( function ()
         local pp = player.pilot()
         replacement = nil
         for k,v in ipairs(pp:outfitsList("intrinsic")) do
            if v == upgrade then
               vn.jump( s.."_exists" )
               return
            end
            for i,uo in pairs(upgrade_list) do
               if uo == v then
                  replacement = uo
                  vn.jump( s.."_replace" )
                  return
               end
            end
         end
      end )
      mm(fmt.f(_([["I would be able to provide my special services for, let's say, {cost}, how does that sound?"

{upgrade_desc}

You have {amount}. Pay {cost} for {upgrade}?]]),
         {amount=poi.data_str(poi.data_get()), cost=poi.data_str(upgrade_cost), upgrade=upgrade, upgrade_desc=upgrade:summary()} ))
      vn.menu{
         { _("Pay"), s.."_yes" },
         { _("Back"), s.."_no" },
      }
      vn.jump("special")

      -- TODO better visualization of the stats and details, maybe an outfit that on mouse over shows all the stats or something?
      vn.label( s.."_exists" )
      mm(fmt.f(_([["It seems like I have already upgraded your current ship with {upgradename}."]]),
            {upgradename=upgrade}))
      vn.jump("special")

      vn.label( s.."_replace" )
      mm( function ()
         return fmt.f(_([["It seems like I have already upgraded your current ship with {replacement}. Would you like to replace it with {upgrade} for 2 Encrypted Data Matrices?"

{upgrade_desc}

You have {amount}. Pay {cost} for replacing {replacement} with {upgrade}?]]),
            {amount=poi.data_str(poi.data_get()), cost=poi.data_str(upgrade_cost), upgrade=upgrade, replacement=replacement, upgrade_desc=upgrade:summary()} )
      end )
      vn.menu{
         { _("Pay"), s.."_yes" },
         { _("Back"), s.."_no" },
      }

      vn.label( s.."_yes" )
      vn.func( function ()
         if poi.data_get() < upgrade_cost then
            vn.jump("special_broke")
            return
         end
      end )
      mm(_([["This will take a second."
They grab a toolbox and rush over to your boarded ship. You decide not to follow as somethings are best left not known. At least they know what they are doing right?]]))
      mm(_([[Eventually, they come back covered in what seems to be fish parts and slime.
"That was fun! However, when I put it back together, I found some extra screws. Oh well, it does seem to hold together fairly well. Hope you enjoy the upgrades! The fish smell will go away in a few periods hopefully."]]))
      vn.func( function ()
         local pp = player.pilot()
         -- Remove old and add new
         for k,v in pairs(upgrade_list) do
            pp:outfitRmIntrinsic( v )
         end
         pp:outfitAddIntrinsic( upgrade )
         poi.data_take( upgrade_cost )
      end )
      vn.jump("special")

      vn.label( s.."_no" )
      mm(_([["OK, tell me if you change your mind."]]))
      vn.jump("special")
   end

   vn.label("special_broke")
   mm(_([["You'll need to get some Encrypted Data Matrices first to be able to get an upgrade!"]]))
   vn.jump("special")

   vn.label("special_info")
   mm(_([["Throughout my travels, I've been in many a tight spot. I remember getting stuck with a broken engine capacitor in a lost asteroid field, no food, no equipment, and a hull leaking solar radiation. It was either adapt or perish, and me being here is a testament to my ship modification skills acquired under pressure."]]))
   mm(fmt.f(_([["Or it could be the Toni's Discount Ship Repair Certification Course I did for {cost} in a back-alley on Darkshed. Still never really understood why we need to slap fish skin on the radiators. Bug juice would work much better, but anyway, I got my license and I get spruce up your ship for all your space needs!"]]),
      {cost=fmt.credits(99)}))
   mm(_([["I can offer you three different services, however, you can only have one active at a time on your ship. It is possible to change them if you wish though. In particular, I offer three services: Machiavellian Necessity, Machiavellian Fortune, and Machiavellian Virtue."]]))
   mm(_([["Machiavellian Necessity focuses on tweaking your weapon systems to be able to take decisive action when necessary."
"Machiavellian Fortune will help you minimize the risks of fickleness of fortune."
"Finally, Machiavellian Virtue will make you a bulwark against unforeseen mishaps."]]))
   for k,v in pairs(upgrade_list) do
      mm(v:summary())
   end
   vn.jump("special")

   vn.label("leave")
   vn.run()
   player.unboard()

   -- Boarding sound
   der.sfx.unboard:play()
end
