--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Hired Escort Handler">
 <trigger>load</trigger>
 <chance>100</chance>
 <flags>
  <unique />
 </flags>
</event>
--]]
--[[

   Escort Handler Event

   This event runs constantly in the background and manages escorts
   hired from the bar, including generating NPCs at the bar and managing
   escort creation and behavior in space.

--]]
local fmt = require "format"
local portrait = require "portrait"
local pir = require "common.pirate"
local pilotname = require "pilotname"

-- Unsaved global tables
local npcs

local logidstr = "log_hiredescort"

local npctext = {}
npctext[1] = _([["Hi there! I'm looking to get some piloting experience. Here are my credentials. Would you be interested in hiring me?"]])
npctext[2] = _([["Hello! I'm looking to join someone's fleet. Here's my credentials. What do you say, would you like me on board?"]])
npctext[3] = _([["Hi! You look like you could use a pilot! I'm available and charge some of the best rates in the galaxy, and I promise you I'm perfect for the job! Here's my info. Well, what do you think? Would you like to add me to your fleet?"]])

function create ()
   lastplanet = nil
   lastsys = system.cur()
   npcs = {}
   escorts = {}
   escorts["__save"] = true

   hook.land("land")
   hook.load("land")
   hook.jumpout("jumpout")
   hook.enter("enter")
   hook.pay("pay")
end


local function createPilotNPCs ()
   local ship_choices = {
      --{ ship = "Llama",       royalty = 0.05 },
      { ship = "Hyena",       royalty = 0.1 },
      { ship = "Shark",       royalty = 0.15 },
      { ship = "Vendetta",    royalty = 0.2 },
      { ship = "Lancelot",    royalty = 0.2 },
      { ship = "Ancestor",    royalty = 0.25 },
      { ship = "Admonisher",  royalty = 0.35 },
      { ship = "Phalanx",     royalty = 0.35 },
      { ship = "Pacifier",    royalty = 0.45 },
      { ship = "Vigilance",   royalty = 0.45 },
   }
   local num_pilots = rnd.rnd(0, 5)
   local fac = faction.get("Mercenary")
   local name_func = pilotname.generic
   local portrait_arg = nil

   local pf = planet.cur():faction()
   if pir.factionIsPirate( pf ) then
      ship_choices = {
         { ship = "Hyena",             royalty = 0.1 },
         { ship = "Pirate Shark",      royalty = 0.15 },
         { ship = "Pirate Vendetta",   royalty = 0.2 },
         { ship = "Pirate Ancestor",   royalty = 0.25 },
         { ship = "Pirate Admonisher", royalty = 0.35 },
         { ship = "Pirate Phalanx",    royalty = 0.35 },
      }
      fac = faction.get("Pirate")
      name_func = pilotname.pirate
      portrait_arg = "Pirate"
   elseif pf == faction.get("Thurion") then
      ship_choices = {
         { ship = "Thurion Ingenuity",     royalty = 0.15 },
         { ship = "Thurion Scintillation", royalty = 0.25 },
         { ship = "Thurion Virtuosity",    royalty = 0.35 },
         { ship = "Thurion Apprehension",  royalty = 0.5 },
      }
      fac = faction.get("Thurion")
      portrait_arg = "Thurion"
   end

   if fac == nil or fac:playerStanding() < 0 then
      return
   end

   for i=1, num_pilots do
      local newpilot = {}
      local shipchoice = ship_choices[rnd.rnd(1, #ship_choices)]
      local p = pilot.add(shipchoice.ship, fac)
      local _n, deposit = p:ship():price()
      newpilot.outfits = {}
      newpilot.outfits["__save"] = true

      for j, o in ipairs(p:outfits()) do
         deposit = deposit + o:price()
         newpilot.outfits[#newpilot.outfits + 1] = o:nameRaw()
      end

      deposit = math.floor((deposit + 0.1*deposit*rnd.sigma()) / 4)
      if deposit <= player.credits() then
         newpilot.ship = ship.get(shipchoice.ship)
         newpilot.deposit = deposit
         newpilot.deposit_text = fmt.credits(newpilot.deposit)
         newpilot.royalty = (
               shipchoice.royalty + 0.05*shipchoice.royalty*rnd.sigma() )
         newpilot.royalty_percent = newpilot.royalty * 100
         newpilot.name = name_func()
         newpilot.portrait = portrait.get(portrait_arg)
         newpilot.faction  = fac
         newpilot.approachtext = npctext[rnd.rnd(1, #npctext)]
         local id = evt.npcAdd(
            "approachPilot", _("Pilot for Hire"), newpilot.portrait,
            fmt.f(_([[This pilot seems to be looking for work.

Ship: {ship}
Deposit: {deposit_text}
Royalty: {royalty_percent:.1f}% of mission earnings]]), newpilot), 9 )

         npcs[id] = newpilot
      end
   end
end


local function getTotalRoyalties ()
   local royalties = 0
   for i, edata in ipairs(escorts) do
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
   lastplanet = planet.cur()
   npcs = {}
   if standing_hook ~= nil then
      hook.rm(standing_hook)
      standing_hook = nil
   end

   -- Clean up dead escorts so it doesn't build up, and create NPCs for
   -- existing escorts.
   local new_escorts = {}
   new_escorts["__save"] = true
   for i, edata in ipairs(escorts) do
      if edata.alive then
         local j = #new_escorts + 1
         edata.pilot = nil
         edata.temp = nil
         edata.armor = nil
         edata.shield = nil
         edata.stress = nil
         edata.energy = nil
         local id = evt.npcAdd(
               "approachEscort", edata.name, edata.portrait,
               _("This is one of the pilots currently under your wing."), 8 )
         npcs[id] = edata
         new_escorts[j] = edata
      end
   end
   escorts = new_escorts

   if #escorts <= 0 then
      evt.save(false)
   end

   -- Ignore on uninhabited and planets without bars
   local pnt = planet.cur()
   local services = pnt:services()
   local flags = pnt:flags()
   if not services.inhabited or not services.bar or flags.nomissionspawn then
      return
   end

   -- Create NPCs for pilots you can hire.
   createPilotNPCs()
end


function jumpout ()
   for i, edata in ipairs(escorts) do
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

   if standing_hook == nil then
      standing_hook = hook.standing("standing")
   end

   local spawnpoint
   if lastsys == system.cur() then
      spawnpoint = lastplanet
   else
      spawnpoint = player.pos()
      for i, sys in ipairs(lastsys:adjacentSystems()) do
         if sys == system.cur() then
            spawnpoint = lastsys
         end
      end
   end
   lastsys = system.cur()

   local pp = player.pilot()
   for i, edata in ipairs(escorts) do
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
   for i, edata in ipairs(escorts) do
      if edata.alive and edata.royalty then
         royalty = royalty + amount * edata.royalty
      end
   end
   player.pay( -royalty, true )
end


function standing ()
   for i, edata in ipairs(escorts) do
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
   local edata = escorts[arg]
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
         for i, edata in ipairs(escorts) do
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
   local edata = escorts[arg]
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

-- Approaching unhired pilot at the bar
function approachPilot( npc_id )
   local pdata = npcs[npc_id]
   if pdata == nil then
      evt.npcRm(npc_id)
      return
   end

   if not tk.yesno("", getOfferText(pdata.approachtext, pdata)) then
      return -- Player rejected offer
   end

   if pdata.deposit and pdata.deposit > player.credits() then
      tk.msg("", _("You don't have enough credits to pay for this pilot's deposit."))
      return
   end
   if getTotalRoyalties() + pdata.royalty > 1 then
      if not tk.yesno("", _("Hiring this pilot will lead to you paying more in royalties than you earn from missions, meaning you will lose credits when doing missions. Are you sure you want to hire this pilot?")) then
         return
      end
   end

   if pdata.deposit then
      player.pay(-pdata.deposit, true)
   end

   local i = #escorts + 1
   pdata.alive = true
   escorts[i] = pdata
   evt.npcRm(npc_id)
   npcs[npc_id] = nil
   local id = evt.npcAdd(
         "approachEscort", pdata.name, pdata.portrait,
         _("This is one of the pilots currently under your wing."), 8 )
   npcs[id] = pdata
   evt.save(true)

   local edata = escorts[i]
   shiplog.create( logidstr, _("Hired Escorts"), _("Hired Escorts") )
   shiplog.append( logidstr, fmt.f(_("You hired a {ship} ship named '{name}' for {deposit_text} and {royalty_percent:.1f}% of mission earnings."), edata ) )
end

