--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Derelict">
 <location>enter</location>
 <chance>45</chance>
 <cond>system.cur():faction() ~= nil</cond>
 <notes>
  <tier>1</tier>
 </notes>
</event>
--]]
--[[
   Derelict Event

   Creates a derelict ship that spawns random events.
--]]
local vntk = require 'vntk'
local fmt = require 'format'
local tut = require 'common.tutorial'
local der = require 'common.derelict'
local pir = require "common.pirate"
local vn = require 'vn'
local lf = require "love.filesystem"

local badevent, goodevent, specialevent, neutralevent, derelict_msg -- forward-declared functions
local derelict -- Non-persistent state

local special_list, dospecial

function create ()
   local cursys = system.cur()
   local _nebu_dens, nebu_vol = cursys:nebula()
   if nebu_vol > 0 then
      evt.finish()
   end

   -- Don't spawn in restricted space
   if cursys:tags().restricted then
      evt.finish()
   end

   -- Don't spawn in Thurion space
   if cursys:faction()==faction.get("Thurion") then
      evt.finish()
   end

   -- Ignore claimed systems (don't want to ruin the atmosphere)
   if not naev.claimTest( cursys, true ) then evt.finish() end


   special_list = {}
   --[[
   return {
      mission = "Mission Name",
      func = function (),
         -- Function to run if defined, if it returns false it tries to run a neutral event instead
         return true
      end
      weight = 1, -- how it should be weighted, defaults to 1
   },
   --]]
   for k,v in ipairs(lf.enumerate("events/derelict")) do
      local sp = require ("events.derelict."..string.gsub(v,".lua","") )()
      if sp then
         sp.weight = sp.weight or 1
         table.insert( special_list, sp )
      end
   end
   table.sort( special_list, function( a, b )
      return a.weight > b.weight
   end )

   -- Have at least one special event
   if #special_list > 0 and rnd.rnd() < 0.4 then
      local weights = 0
      for _k,sp in ipairs(special_list) do
         weights = weights + sp.weight
         sp.chance = weights
      end
      local r = rnd.rnd()
      for _k,sp in ipairs(special_list) do
         if r < sp.chance / weights then
            dospecial = sp
            break
         end
      end
   end

   -- If player is low on fuel make it more likely they get fuel
   local pp = player.pilot()
   local stats = pp:stats()
   if stats.fuel < stats.fuel_consumption and rnd.rnd() < 0.8 then
      dospecial = {
         func = function ()
            derelict_msg(_("Lucky find!"), _([[The derelict appears deserted, with almost everything of value long gone. As you explore the ship you suddenly pick up a back-up fuel tank hidden in the walls. The fuel is in a good state and you siphon it off to fill your ship's fuel tanks. Talk about good timing.]]), fmt.f(_([[Just as you were running out of fuel you found a derelict with some to spare in {sys}, what good fortune!]]), {sys=system.cur()}))
            pp:setFuel( true )
            return true
         end
      }
   end

   -- Get the derelict's ship.
   local dship
   if dospecial then
      dship = dospecial.ship
   end
   if not dship then
      local r = rnd.rnd()
      if r < 0.2 then
         dship = "Llama"
      elseif r < 0.3 then
         dship = "Hyena"
      elseif r < 0.5 then
         dship = "Koala"
      elseif r < 0.7 then
         dship = "Quicksilver"
      elseif r < 0.9 then
         dship = "Mule"
      else
         dship = "Gawain"
      end
   end

   -- Create the derelict.
   local dist  = rnd.rnd() * cursys:radius() * 0.8
   local pos   = vec2.newP( dist, rnd.angle() )
   derelict    = pilot.add(dship, "Derelict", pos, p_("ship", "Derelict"), {ai="dummy", naked=true})
   derelict:disable()
   derelict:intrinsicSet( "ew_hide", -75 ) -- Much more visible
   hook.pilot(derelict, "board", "board")
   hook.pilot(derelict, "death", "destroyevent")
   hook.jumpout("destroyevent")
   hook.land("destroyevent")
end

function derelict_msg( title, text, log )
   vntk.msg( title, text, {
      pre = function ()
         vn.music( der.sfx.ambient )
         vn.sfx( der.sfx.board )
      end,
      post = function ()
         vn.sfx( der.sfx.unboard )
      end,
   } )

   der.addMiscLog( log )
end

local function islucky ()
   local pp = player.pilot()
   if pp:ship():tags().lucky then
      return true
   end
   for k,o in ipairs(pp:outfitsList("all")) do
      if o:tags().lucky then
         return true
      end
   end
   return false
end

function board()
   player.unboard()

   -- In case of special event loaded, just use that
   if dospecial then
      return specialevent( dospecial )
   end

   -- Roll for events
   local neuprob, goodprob
   if islucky() then
      goodprob = 0.75
      neuprob = 0.95
   else
      goodprob = 0.6
      neuprob = 0.8
   end
   local prob = rnd.rnd()
   if prob <= goodprob then
      goodevent()
   elseif prob <= neuprob then
      neutralevent()
   else
      badevent()
   end
end

function neutralevent()
   -- neuevent is a table of tables { {title of event, text of event, log of event}, {...}, ... }
   local neuevent = {
      {
         _("Empty derelict"),
         _([[You spend some time searching the derelict, but it doesn't appear there are any remaining passengers, nor is there anything of interest to you. You decide to return to your ship.]]),
         fmt.f(_([[You searched an empty derelict in {sys}.]]), {sys=system.cur()})
      },
      {
         _("Distracting Derelict"),
         fmt.f(_([[This ship has clearly been abandoned a long time ago. Looters have been here many times and all of the primary and backup systems are down. However, there is one console that is still operational. It appears to be running an ancient computer game about space exploration, trade and combat. Intrigued, you decide to give the game a try, and before long you find yourself hooked on it. You spend many fun periods hauling cargo, upgrading your ship, fighting enemies and exploring the universe. But of course you can't stay here forever and besides the chirping over the comms from your ship's AI, {shipai}, is getting annoying. You regretfully leave the game behind and return to the reality of your everyday life.]]), {shipai=tut.ainame()}),
         fmt.f(_([[You searched a derelict in {sys} and ended up spending an inordinate amount of time playing an ancient computer game!]]), {sys=system.cur()})
      },
      {
         _("Errie Derelict"),
         _([[You are exploring the derelict ship when you hear a strange creaking noise. You decide to follow it to see what is causing it, but you never seem to be able to pinpoint the source. After about an hour of fruitlessly opening up panels, pressing your ear against the deck and running hull scans from your own ship, you decide to give up and leave the derelict to its creepy creaking.]]),
         fmt.f(_([[You searched an empty, creaking derelict in {sys}.]]), {sys=system.cur()})
      },
      {
         _("Distracting Derelict"),
         fmt.f(_([[While exploring the cockpit of this derelict, you come across the captain's logs. Hoping to find some information that could be of use to you, you decide to play back the most recent entries. Unfortunately, it turns out the captain's logs are little more than recordings of the captain having heated arguments with her co-pilot. It isn't long before you decide you probably aren't going to find anything worthwhile here. Having returned to your ship {shipai}, your Ship AI, confirms there is little but "emotional human chatter" in the captain's logs.]]), {shipai=tut.ainame()}),
         fmt.f(_([[You searched a derelict in {sys} but found little but for "emotional human chatter" in the captain's logs.]]), {sys=system.cur()})
      },
      {
         _("Wrecked Derelict"),
         fmt.f(_([[This derelict is not deserted. The crew are still onboard. Unfortunately for them, they didn't survive whatever wrecked their ship, neither did much of their ship. Your ship AI, {shipai}, and you decide to give the crew and their ship a decent space burial before moving on.]]), {shipai=tut.ainame()}),
         fmt.f(_([[You found a wrecked derelict in {sys} and gave it and its crew a decent space burial.]]), {sys=system.cur()})
      },
      {
         _("Empty Derelict"),
         _([[This derelict seems to have been visited by looters already. You find a message carved into the wall near the airlock. It reads: "I WUS HEAR". Below it is another carved message that says "NO U WASNT". Otherwise, there is nothing of interest left on this ship.]]),
         fmt.f(_([[You searched an empty derelict in {sys} and found only graffiti.]]), {sys=system.cur()})
      },
      {
         _("Not-so-empty Derelict"),
         _([[This derelict seems to have been, at one time, used as an illegal casino. There are roulette tables and slot machines set up in the cargo hold. However, it seems the local authorities caught wind of the operation; there are scorch marks on the furniture and walls and assault rifle shells underfoot. You don't reckon you're going to find anything useful here so you leave.]]),
         fmt.f(_([[You boarded a derelict in {sys} and found an ex-hive of illegal activity.]]), {sys=system.cur()})
      },
      {
         _("Not-so-empty Derelict"),
         _([[When the airlock opens, you are hammered in the face by an ungodly smell that almost makes you pass out on the spot. You hurriedly close the airlock again and flee back into your own ship. Whatever is on that derelict, you don't want to find out!]]),
         fmt.f(_([[An ungodly smell stopped you from searching a derelict in {sys}.]]), {sys=system.cur()})
      },
      {
         _("Wrecked Derelict"),
         fmt.f(_([[This derelict has been really badly beaten up. Most of the corridors are blocked by mangled metal and your ship AI, {shipai}, tells you there are depressurized compartments all over the ship. There's not much you can do here, so you decide to leave the derelict to itself.]]), {shipai=tut.ainame()}),
         fmt.f(_([[You tried to search a wrecked derelict in {sys} but there wasn't much left.]]), {sys=system.cur()})
      },
      {
         _("Distracting Derelict"),
         _([[The interior of this ship is decorated in a gaudy fashion. There are cute plushes hanging from the doorways, drapes on every viewport, coloured pillows in the corners of most compartments and cheerful graffiti on almost all the walls. A scan of the ship's computer shows that this ship belonged to a trio of adventurous young ladies who decided to have a wonderful trip through space. Sadly, the ship's log tells you none of them really knew how to fly a space ship, so they ended up stranded and had to be rescued. Shaking your head, you return to your own ship.]]),
         fmt.f(_([[You searched a gaudily decorated derelict in {sys} and found the previous occupants had already been rescued]]), {sys=system.cur()})
      },
      {
         _("Eerie Derelict"),
         _([[The artificial gravity on this ship has bizarrely failed, managing to somehow reverse itself. As soon as you step aboard you fall upwards and onto the ceiling, getting some nasty bruises in the process. Annoyed, you search the ship but without result. You return to your ship - but forget about the polarized gravity at the airlock, so you, again, smack against the deck plates.]]),
         fmt.f(_([[Having searched a derelict in {sys} you remind yourself always to check the gravity before boarding another ship. Ow.]]), {sys=system.cur()})
      },
      {
         _("Empty derelict"),
         _([[The cargo hold of this ship contains several heavy, metal chests. You pry them open but they are empty. Whatever was in them must have been pilfered by other looters already. You decide not to waste any time on this ship and return to your own.]]),
         fmt.f(_([[You found some empty chests on an empty derelict in {sys} system.]]), {sys=system.cur()})
      },
      {
         _("Wrecked Derelict"),
         fmt.f(_([[You have attached your docking clamp to the derelict's airlock but the door refuses to open. {shipai}, your ship AI, tells you that the other side isn't pressurized and the doors are trying to save your life. The derelict must have suffered too many hull breaches over the years. It doesn't seem like there's much you can do here.]]), {shipai=tut.ainame()}),
         fmt.f(_([[You tried to search an empty derelict in {sys} but your airlock doors and the vacuum of space defeated you!]]), {sys=system.cur()})
      },
      {
         _("Eerie Derelict"),
         _([[As you walk through the corridors of the derelict, you can't help but notice the large scratch marks on the walls, the floor and even the ceiling. It's as if something went on a rampage throughout this ship - something big, with a lot of very sharp claws and teeth… You feel it might be best to leave as soon as possible, despite your ship's assurances of sensing no signs of life onboard, so you abandon the search and swiftly disengage your boarding clamp.]]),
         fmt.f(_([[You left an empty, torn-up (literally) derelict you found in {sys} system to itself.]]),
         {sys=system.cur()})
      },
      {
         _("Eerie Derelict"),
         fmt.f(_([[Entering your airlock you see the derelict's own airlock, oddly, spiral open onto a faintly-purple glowing, octagonal corridor. Noting this slightly strange interior design choice you continue along the faintly radiating corridor to find yourself in an octagonal ("Truncated cuboctahedronal" chirps in {shipai}, your ship AI) room emanating a rather stranger purple-green or green-purple eldritch colour. What furnishings you can see are angular in a way that makes your eyes water. These design choices go from strange too… "Thump"! You land on the floor(?) as you trip over your gravity confused feet! Seriously!?! They rigged gravity to the outside "walls" of the room!?! You decide it might be better not to discover anything more about this ship without an appropriately equipped boarding party, maybe one with a magician!]]), {shipai=tut.ainame()}),
         fmt.f(_([[An eldritch, possibly derelict, {shp} in {sys} system unnerved you, its true story is no longer your concern, thankfully!]]), {shp=derelict:ship(), sys=system.cur()})
      },
      {
         _("Empty Derelict"),
         _([[Your airlock doors slide open starting, what turns out to be, the most uneventful, dull, mundane, entirely-boring and mind-numbingly unhelpful waste of time that any boarding of a derelict ship has ever been, it even tempts you not to do this again on the off chance you run across another such distressingly worthless derelict.]]),
         fmt.f(_([[You became disenchanted with boarding derelicts on a ship in {sys}… next time better to watch paint dry.]]), {sys=system.cur})
      },
   }

   -- Pick a random message from the list, display it, unboard.
   local chosen = rnd.rnd(1, #neuevent)
   derelict_msg( neuevent[chosen][1], neuevent[chosen][2], neuevent[chosen][3] )
   destroyevent()
end

function goodevent()
   local gtitle = _("Lucky find!")

   local goodevent_list = {
      function ()
         derelict_msg( gtitle, _([[The derelict appears deserted, its passengers long gone. However, they seem to have left behind a small amount of credit chips in their hurry to leave! You decide to help yourself to them and leave the derelict.]]), fmt.f(_([[You found a derelict in {sys}, it was empty but for some scattered credit chips. Lucky you!]]), {sys=system.cur()}) )
         player.pay( rnd.rnd(5e3,30e3) )
      end,
      function ()
         local csys = system.cur()
         local sysset = { [csys:nameRaw()] = csys:faction() }
         for i=1,3 do
            for s,f in pairs(sysset) do
               for j,n in ipairs(system.get(s):adjacentSystems()) do
                  sysset[ n:nameRaw() ] = n:faction()
               end
            end
         end
         local whitelist = {
            ["Empire"]  = true,
            ["Dvaered"] = true,
            ["Sirius"]  = true,
            ["Soromid"] = true,
            ["Za'lek"]  = true,
            ["Frontier"]= true,
            ["Goddard"] = true
         }
         local sysfct = {}
         for s,f in pairs(sysset) do
            if f then
               local nr = f:nameRaw()
               if whitelist[nr] then
                  sysfct[ nr ] = true
               end
            end
         end
         local fcts = {}
         for f,i in pairs(sysfct) do
            table.insert( fcts, f )
         end
         if #fcts == 0 then
            for f,i in pairs(whitelist) do
               table.insert( fcts, f )
            end
         end
         local rndfactRaw = fcts[ rnd.rnd(1, #fcts) ]
         local rndfact = _(rndfactRaw)
         derelict_msg(gtitle, fmt.f(_([[This ship looks like any old piece of scrap at a glance, but it is actually an antique, one of the very first of its kind ever produced according to your ship AI, {shipai}! Museums all over the galaxy would love to have a ship like this. You plant a beacon on the derelict to mark it for salvaging and contact the {fct} authorities. Your reputation with them has slightly improved.]]), {shipai=tut.ainame(), fct=rndfact}), fmt.f(_([[In the {sys} system you found a very rare antique derelict {shp} and reported it to the, happy to hear from you, {fct} authorities.]]), {sys=system.cur(), shp=derelict:ship(), fct=rndfact}))
         faction.modPlayerSingle(rndfactRaw, 2, "script")
      end,
   }

   -- Only give cargo if enough space
   if player.fleetCargoFree() > 10 then
      table.insert( goodevent_list, function ()
         local commodities = {
            -- Uncommon
            "Gold",
            "Platinum",
            "Rhodium",
            "Yttrium",
            -- Rare
            "Vixilium",
            "Therite",
            "Kermite",
         }
         local c = commodity.get( commodities[ rnd.rnd(1,#commodities) ] )
         derelict_msg( gtitle, fmt.f(_([[You explore the empty derelict without much success. As you cross a hallway you hear a weird echo of your footsteps. After some careful investigation. you find that there is an entire hidden cargo hold in the ship full of {cargo}! This should fetch a pretty penny on the market. You load as much of it as you can fit in your hold.]]),{cargo=c}), fmt.f(_([[You found a derelict in {sys}, it had a hidden cargo hold full of {cargo}!]]), {sys=system.cur(),cargo=c}) )
         player.fleetCargoAdd( c, rnd.rnd(30,100) )
      end )
   end

   -- See if we should add maps
   local maps = {
      ["Local System Map"] = _("local system"),
      ["Map: Empire Core"] = _("Empire core systems"),
      ["Map: Za'lek Core"] = _("Za'lek core systems"),
      ["Map: Dvaered Core"] = _("Dvaered core systems"),
      ["Map: Empire-Soromid Trade Route"] = _("Dvaered-Soromid trade route"),
      ["Map: Sirius Core"] = _("Sirius core systems"),
      ["Map: Sirian Border Systems"] = _("Sirian border systems"),
      ["Map: The Frontier"] = _("Frontier systems"),
      ["Map: Nebula Edge"] = _("Imperial Nebula edge"),
   }
   if __debugging then -- Make sure map names are valid
      for k,v in pairs(maps) do
         local _o = outfit.get(k)
      end
   end

   local unknown = {}
   for k,v in pairs(maps) do
      if player.outfitNum(k) == 0 then
         table.insert( unknown, k )
      end
   end
   -- There are unknown maps
   if #unknown > 0 then
      table.insert( goodevent_list, function ()
         local choice = unknown[rnd.rnd(1,#unknown)]
         derelict_msg(gtitle, fmt.f(_([[The derelict is empty and seems to have been thoroughly picked over by other space buccaneers. However, the ship's computer contains a map of the {smp}! You download it to your own computer.]]), {smp=maps[choice]}), fmt.f(_([[You found a derelict in {sys}, it was empty but the nav system contained a {smp} map! Nice!]]), {sys=system.cur(), smp=maps[choice]}))
         player.outfitAdd(choice, 1)
      end )
   end

   -- If the player has little fuel left allow for refueling
   local pp = player.pilot()
   local stats = pp:stats()
   if stats.fuel < 2*stats.fuel_consumption then
      table.insert( goodevent_list, function ()
         derelict_msg(gtitle, _([[The derelict appears deserted, with almost everything of value long gone. As you explore the ship your handyscan suddenly picks up a back-up fuel tank hidden in the walls. The fuel is in a good state and you siphon it off to fill your own ship's fuel tanks.]]), fmt.f(_([[A derelict you found in {sys} was empty but for a reserve fuel tank you fortuitously discovered!]]), {sys=system.cur()}))
         pp:setFuel(true)
      end )
   end

   -- If the player is low on health and not regenerating, offer armour repairs
   local armour, shield = pp:health()
   if armour < 50 and stats.armour_regen <= 0 then
      table.insert( goodevent_list, function ()
         derelict_msg(gtitle, fmt.f(_([[The derelict is deserted and striped of everything of value, however, you notice that the ship hull is in very good shape. In fact, it is rather suspicious that a ship in such good repair became a derelict. Hushing {shipai}, your ship AI, and without thinking too deeply about it you strip some of the hull components and are able to repair your own ship's armour.]]), {shipai=tut.ainame()}), fmt.f(_([[The hull of a derelict you found in {sys} provided you with the resources to repair your own, very useful in the circumstances!]]), {sys=system.cur()}))
         pp:setHealth( 100, shield )
      end )
   end

   -- Run random good event
   goodevent_list[ rnd.rnd(1,#goodevent_list) ]()
   destroyevent()
end

function badevent()
   local btitle = _("Oh no!")
   local badevent_list = {
      function ()
         derelict:hookClear() -- So the pilot doesn't end the event by dying.
         derelict_msg(btitle, _([[The moment you affix your boarding clamp to the derelict ship, it triggers a booby trap! The derelict explodes, severely damaging your ship. You escaped death this time, but it was a close call!]]), fmt.f(_([[It was a trap! A derelict you found in {sys} was rigged to explode when your boarding clamp closed! That was a little too close for comfort.]]), {sys=system.cur()}))
         derelict:setHealth(-1,-1)
         player.pilot():control(true)
         hook.pilot(derelict, "exploded", "derelict_exploded")
      end,
      function ()
         derelict_msg(btitle, _([[You board the derelict ship and search its interior, but you find nothing. When you return to your ship, however, your ship's sensors finds there were Space Leeches onboard the derelict - and they've now attached themselves to your ship! You scorch them off with a plasma torch, but it's too late. The little buggers have already drunk all of your fuel. You're not jumping anywhere until you find some more!]]), fmt.f(_([[Space Leeches attached to a derelict you found in {sys} sucked your ship empty of jump fuel before you could get rid of them! Rough break.]]), {sys=system.cur()}))
         player.pilot():setFuel(false)
         destroyevent()
      end,
   }
   if pir.systemPresence() > 0 then
      table.insert( badevent_list,
         function ()
            derelict_msg(btitle, fmt.f(_([[You affix your boarding clamp and walk aboard the derelict ship. You've only spent a little time searching the interior when {shipai}, your ship AI sounds a proximity alarm from your ship! Pirates are closing on your position! Clearly this derelict was a trap! You run back onto your ship and prepare to undock, but you've lost precious time. The pirates are already in firing range…]]), {shipai=tut.ainame()}), fmt.f(_([[It was a trap! Pirates baited you with a derelict ship in {sys}, fortunately you lived to tell the tale but you'll be more wary next time you board a derelict.]]), {sys=system.cur()}))

            local s = player.pilot():ship():size()
            local enemies_tiny = {
               "Pirate Hyena",
               "Pirate Hyena",
               "Pirate Hyena",
            }
            local enemies_sml = {
               "Pirate Vendetta",
               "Pirate Shark",
               "Pirate Shark",
            }
            local enemies_med1 = {
               "Pirate Vendetta",
               "Pirate Vendetta",
               "Pirate Ancestor",
               "Pirate Ancestor",
            }
            local enemies_med2 = {
               "Pirate Admonisher",
               "Pirate Shark",
               "Pirate Shark",
               "Pirate Shark",
            }
            local enemies_hvy = {
               "Pirate Starbridge",
               "Pirate Vendetta",
               "Pirate Vendetta",
               "Pirate Ancestor",
               "Pirate Ancestor",
            }
            local enemies_dng = {
               "Pirate Kestrel",
               "Pirate Vendetta",
               "Pirate Vendetta",
               "Pirate Ancestor",
               "Pirate Ancestor",
            }
            local enemies
            local r = rnd.rnd()
            if s == 1 then
               enemies = enemies_tiny
            elseif s == 2 then
               if r < 0.5 then
                  enemies = enemies_sml
               else
                  enemies = enemies_tiny
               end
            elseif s == 3 then
               if r < 0.1 then
                  enemies = enemies_med2
               elseif r < 0.6 then
                  enemies = enemies_med1
               else
                  enemies = enemies_sml
               end
            elseif s == 4 then
               if r < 0.1 then
                  enemies = enemies_hvy
               elseif r < 0.6 then
                  enemies = enemies_med2
               else
                  enemies = enemies_sml
               end
            elseif s == 5 then
               enemies = enemies_hvy
            else
               enemies = enemies_dng
            end
            local pirates = {}
            local pos = player.pos()
            local leader
            for _k,v in ipairs(enemies) do
               local dist = 800 + 200 * ship.get(v):size()
               local p = pilot.add( v, "Marauder", pos + vec2.newP( dist + rnd.rnd()*0.5*dist, rnd.angle() ) )
               p:setHostile( true ) -- Should naturally attack the player
               table.insert( pirates, p )
               if not leader then
                  leader = p
               else
                  p:setLeader( leader )
               end
            end
            destroyevent()
         end
      )
   end

   -- Run random bad event
   badevent_list[ rnd.rnd(1,#badevent_list) ]()
end

function derelict_exploded()
   local pp = player.pilot()
   pp:control(false)
   local pa, _ps = pp:health()
   pp:setHealth(math.max(1,0.42*pa), 0)
   camera.shake()
   destroyevent()
end

function specialevent( sp )
   if sp.func then
      if not sp.func() then -- Failed to run
         return neutralevent()
      end
   else
      naev.missionStart( sp.mission )
   end
   destroyevent()
   return
end

function destroyevent()
   evt.finish()
end
