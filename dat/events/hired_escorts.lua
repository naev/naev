--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Hired Escort Handler">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]
--[[

   Escort Handler Event

   This event runs constantly in the background and manages escorts
   hired from the bar, including generating NPCs at the bar and managing
   escort creation and behavior in space.

   TODO remove entirely in 0.11.0

--]]
local fmt = require "format"

local npcs -- Non-persistent state

local logidstr = "log_hiredescort"

function create ()
   mem.lastplanet = nil
   mem.lastsys = system.cur()
   npcs = {}
   mem.escorts = {}

   hook.land("land")
   hook.load("land")
   hook.jumpout("jumpout")
   hook.enter("enter")
   hook.pay("pay")
end


local function getTotalRoyalties ()
   local royalties = 0
   for i, edata in ipairs(mem.escorts) do
      if edata.alive then
         royalties = royalties + edata.royalty
      end
   end
   return royalties
end


local function getOfferText( approachtext, edata )
   local _credits, scredits = player.credits(2)
   local credentials = _([[
Pilot name: {name}
Ship: {ship}
Deposit: {deposit_text}
Royalty: {royalty_percent:.1f}% of mission earnings]])

   local finances = _([[
Money: {credits}
Current total royalties: {total:.1f}% of mission earnings]])
   return (approachtext .. "\n\n" .. fmt.f( credentials, edata ) .. "\n\n" ..
	   fmt.f( finances, {credits=scredits, total=getTotalRoyalties()*100} ))
end


function land ()
   mem.lastplanet = spob.cur()
   npcs = {}
   hook.rm(mem.standing_hook)
   mem.standing_hook = nil

   -- Clean up dead escorts so it doesn't build up, and create NPCs for
   -- existing escorts.
   local new_escorts = {}
   for i, edata in ipairs(mem.escorts) do
      if edata.alive then
         local j = #new_escorts + 1
         edata.pilot = nil
         edata.temp = nil
         edata.armor = nil
         edata.shield = nil
         edata.stress = nil
         edata.energy = nil
         local id = evt.npcAdd( "approachEscort", edata.name, edata.portrait,
               _("This is one of the pilots currently under your wing."), 8 )
         npcs[id] = edata
         new_escorts[j] = edata
      end
   end
   mem.escorts = new_escorts

   if #mem.escorts <= 0 then
      evt.save(false)
   end

   -- Ignore on uninhabited and planets without bars
   local pnt = spob.cur()
   local services = pnt:services()
   local flags = pnt:flags()
   if not services.inhabited or not services.bar or flags.nomissionspawn then
      return
   end

   -- Create NPCs for pilots you can hire.
   --createPilotNPCs()
end


function jumpout ()
   for i, edata in ipairs(mem.escorts) do
      if edata.alive and edata.pilot ~= nil and edata.pilot:exists() then
         edata.temp = edata.pilot:temp()
         edata.armor, edata.shield, edata.stress = edata.pilot:health()
         edata.energy = edata.pilot:energy()
         edata.pilot:rm()
         edata.pilot = nil
      else
         edata.alive = false
      end
   end
end


-- Pilot is no longer employed by the player
local function pilot_disbanded( edata )
   edata.alive = false
   local p = edata.pilot
   if p and p:exists() then
      p:setLeader(nil)
      p:setVisplayer(false)
      p:setNoClear(false)
      p:setFriendly(false)
      p:hookClear()
   end
end


function enter ()
   if var.peek( "hired_escorts_disabled" ) then return end

   if mem.standing_hook == nil then
      mem.standing_hook = hook.standing("standing")
   end

   local spawnpoint
   if mem.lastsys == system.cur() then
      spawnpoint = mem.lastplanet
   else
      spawnpoint = player.pos()
      for i, sys in ipairs(mem.lastsys:adjacentSystems()) do
         if sys == system.cur() then
            spawnpoint = mem.lastsys
         end
      end
   end
   mem.lastsys = system.cur()

   local pp = player.pilot()
   for i, edata in ipairs(mem.escorts) do
      if edata.alive then
         local f = faction.get(edata.faction)

         edata.pilot = pilot.add(edata.ship, f, spawnpoint, edata.name, {naked=true})
         for j, o in ipairs(edata.outfits) do
            edata.pilot:outfitAdd(o)
         end
         edata.pilot:fillAmmo()
         edata.pilot:setFriendly()
         -- This should be set by the player and/or part of the pilot's innate characteristics
         local aimem = edata.pilot:memory()
         aimem.atk_kill = false

         local temp = 250
         local armor = 100
         local shield = 100
         local stress = 0
         local energy = 100
         if edata.temp ~= nil then
            temp = edata.temp
         end
         if edata.armor ~= nil then
            armor = edata.armor
         end
         if edata.shield ~= nil then
            shield = edata.shield
         end
         if edata.stress ~= nil then
            -- Limit this to 99 so we don't have the weirdness of a
            -- disabled ship warping in.
            stress = math.min(edata.stress, 99)
         end
         if edata.energy ~= nil then
            energy = edata.energy
         end
         edata.pilot:setTemp(temp, true)
         edata.pilot:setHealth(armor, shield, stress)
         edata.pilot:setEnergy(energy)
         edata.pilot:setFuel(true)

         if f == nil or f:playerStanding() >= 0 then
            edata.pilot:setLeader(pp)
            edata.pilot:setVisplayer(true)
            edata.pilot:setNoClear(true)
            hook.pilot(edata.pilot, "death", "pilot_death", i)
            hook.pilot(edata.pilot, "attacked", "pilot_attacked", i)
            hook.pilot(edata.pilot, "hail", "pilot_hail", i)
         else
            shiplog.append( logidstr, fmt.f(_("'{name}' ({ship}) has left your employment."), edata) )
            pilot_disbanded( edata )
         end
      end
   end
end


function pay( amount, reason )
   -- Ignore negative amounts
   if amount <= 0 then return end

   -- Whitelist control of reasons
   local whitelist = {
   }
   if not reason or not whitelist[reason] then return end

   local royalty = 0
   for i, edata in ipairs(mem.escorts) do
      if edata.alive and edata.royalty then
         royalty = royalty + amount * edata.royalty
      end
   end
   player.pay( -royalty, true )
end


function standing ()
   for i, edata in ipairs(mem.escorts) do
      if edata.alive and edata.faction ~= nil and edata.pilot ~= nil
            and edata.pilot:exists() then
         local f = faction.get(edata.faction)
         if f ~= nil and f:playerStanding() < 0 then
            shiplog.append( logidstr, fmt.f(_("'{name}' ({ship}) has left your employment."), edata) )
            pilot_disbanded( edata )
         end
      end
   end
end

-- Asks the player whether or not they want to fire the pilot
local function pilot_askFire( edata, npc_id )
   local approachtext = _([[Would you like to do something with this pilot?

Pilot credentials:]])

   local n, _s = tk.choice( "", getOfferText(approachtext, edata), _("Fire pilot"), _("Do nothing") )
   if n == 1 and tk.yesno(
         "", fmt.f(
            _("Are you sure you want to fire {name}? This cannot be undone."),
	    edata ) ) then
      if npc_id then
         evt.npcRm(npc_id)
         npcs[npc_id] = nil
      end
      shiplog.append( logidstr, fmt.f(_("You fired '{name}' ({ship})."), edata) )
      pilot_disbanded( edata )
   end
end

-- Pilot was hailed by the player
function pilot_hail( _p, arg )
   local edata = mem.escorts[arg]
   if not edata.alive then
      return
   end

   player.commClose()
   pilot_askFire( edata )
end

-- Check if player attacked his own escort
function pilot_attacked( p, attacker, _dmg, _arg )
   -- Must have an attacker
   if attacker then
      local l = p:leader()
      -- Either the attacker or the attacker's leader should be their leader
      if attacker == l or attacker:leader() == l then
         -- Since all the escorts will turn on the player, we might as well
         -- just have them all disband at once and attack.
         for i, edata in ipairs(mem.escorts) do
            if edata.pilot and edata.pilot:exists() then
               shiplog.append( logidstr, fmt.f(_("You turned on your hired escort '{name}' ({ship})."), edata) )
               pilot_disbanded( edata )
               edata.pilot:setHostile()
            end
         end
      end
   end
end

-- Escort got killed
function pilot_death( _p, _attacker, arg )
   local edata = mem.escorts[arg]
   shiplog.append( logidstr, fmt.f(_("'{name}' ({ship}) was killed in combat."), edata) )
   pilot_disbanded( edata )
end

-- Approaching hired pilot at the bar
function approachEscort( npc_id )
   local edata = npcs[npc_id]
   if edata == nil then
      evt.npcRm(npc_id)
      return
   end

   pilot_askFire( edata, npc_id )
end
