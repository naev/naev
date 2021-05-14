--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Escort Handler">
 <trigger>load</trigger>
 <chance>100</chance>
 <flags>
  <unique />
 </flags>
</event>
--]]
--[[

   Escort Handler Event

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

--

   This event runs constantly in the background and manages escorts
   hired from the bar, including generating NPCs at the bar and managing
   escort creation and behavior in space.

--]]

local portrait = require "portrait"
require "pilot/generic"
require "pilot/pirate"


npctext = {}
npctext[1] = _([["Hi there! I'm looking to get some piloting experience. Here are my credentials. Would you be interested in hiring me?"]])
npctext[2] = _([["Hello! I'm looking to join someone's fleet. Here's my credentials. What do you say, would you like me on board?"]])
npctext[3] = _([["Hi! You look like you could use a pilot! I'm available and charge some of the best rates in the galaxy, and I promise you I'm perfect for the job! Here's my info. Well, what do you think? Would you like to add me to your fleet?"]])

credentials = _([[
Pilot name: %s
Ship: %s
Deposit: %s
Royalty: %.1f%% of mission earnings

Money: %s
Current total royalties: %.1f%% of mission earnings]])

pilot_action_text = _([[Would you like to do something with this pilot?

Pilot credentials:]])


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


function createPilotNPCs ()
   local ship_choices = {
      { ship = "Hyena", royalty = 0.1 },
      { ship = "Shark", royalty = 0.15 },
      { ship = "Vendetta", royalty = 0.2 },
      { ship = "Lancelot", royalty = 0.2 },
      { ship = "Ancestor", royalty = 0.25 },
      { ship = "Admonisher", royalty = 0.3 },
      { ship = "Phalanx", royalty = 0.3 },
      { ship = "Pacifier", royalty = 0.4 },
      { ship = "Vigilance", royalty = 0.4 },
   }
   local num_pilots = rnd.rnd(0, 5)
   local fac = faction.get("Mercenary")
   local name_func = pilot_name
   local portrait_arg = nil

   if planet.cur():faction() == faction.get("Pirate") then
      ship_choices = {
         { ship = "Hyena", royalty = 0.1 },
         { ship = "Pirate Shark", royalty = 0.15 },
         { ship = "Pirate Vendetta", royalty = 0.2 },
         { ship = "Pirate Ancestor", royalty = 0.25 },
         { ship = "Pirate Admonisher", royalty = 0.3 },
         { ship = "Pirate Phalanx", royalty = 0.3 },
      }
      fac = faction.get("Pirate")
      name_func = pirate_name
      portrait_arg = "Pirate"
   elseif planet.cur():faction() == faction.get("Thurion") then
      ship_choices = {
         { ship = "Thurion Ingenuity", royalty = 0.15 },
         { ship = "Thurion Scintillation", royalty = 0.25 },
         { ship = "Thurion Virtuosity", royalty = 0.3 },
         { ship = "Thurion Apprehension", royalty = 0.4 },
      }
      fac = faction.get("Thurion")
      portrait_arg = "Thurion"
   end

   if fac == nil or fac:playerStanding() < 0 then
      return
   end

   if num_pilots then
      for i=1, num_pilots do
         local newpilot = {}
         local shipchoice = ship_choices[rnd.rnd(1, #ship_choices)]
         local p = pilot.add(shipchoice.ship, fac)
         local n, deposit = p:ship():price()
         newpilot.outfits = {}
         newpilot.outfits["__save"] = true

         for j, o in ipairs(p:outfits()) do
            deposit = deposit + o:price()
            newpilot.outfits[#newpilot.outfits + 1] = o:nameRaw()
         end

         deposit = math.floor((deposit + 0.2*deposit*rnd.sigma()) / 2)
         if deposit <= player.credits() then
            newpilot.ship = shipchoice.ship
            newpilot.deposit = deposit
            newpilot.royalty = (
                  shipchoice.royalty + 0.1*shipchoice.royalty*rnd.sigma() )
            newpilot.name = name_func()
            newpilot.portrait = portrait.get(portrait_arg)
            newpilot.faction = fac:name()
            newpilot.approachtext = npctext[rnd.rnd(1, #npctext)]
            local id = evt.npcAdd(
                  "approachPilot", _("Pilot"), newpilot.portrait,
                  _("This pilot seems to be looking for work."), 9 )
            npcs[id] = newpilot
         end
      end
   end
end


function getTotalRoyalties ()
   local royalties = 0
   for i, edata in ipairs(escorts) do
      if edata.alive then
         royalties = royalties + edata.royalty
      end
   end
   return royalties
end


function land ()
   lastplanet = planet.cur()
   npcs = {}
   if standing_hook ~= nil then
      hook.rm(standing_hook)
      standing_hook = nil
   end
   if hail_hook ~= nil then
      hook.rm(hail_hook)
      hail_hook = nil
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

   -- No sense continuing is there is no bar on the planet.
   if not planet.cur():services()["bar"] then return end

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


function enter ()
   if standing_hook == nil then
      standing_hook = hook.standing("standing")
   end
   if hail_hook == nil then
      hail_hook = hook.hail("hail")
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

   for i, edata in ipairs(escorts) do
      if edata.alive then
         local f = faction.get(edata.faction)

         edata.pilot = pilot.add(edata.ship, f, spawnpoint, edata.name)
         edata.pilot:rmOutfit("all")
         edata.pilot:rmOutfit("cores")
         for j, o in ipairs(edata.outfits) do
            edata.pilot:addOutfit(o)
         end
         edata.pilot:fillAmmo()

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
            -- Limit this to 99 so we don't have the weridness of a
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
            edata.pilot:setLeader(player.pilot())
            edata.pilot:setNoClear(true)
            hook.pilot(edata.pilot, "death", "pilot_death", i)
         else
            edata.alive = false
         end
      end
   end
end


function pay( amount )
   for i, edata in ipairs(escorts) do
      if edata.alive and edata.royalty then
         player.pay(-amount * edata.royalty, true)
      end
   end
end


function standing ()
   for i, edata in ipairs(escorts) do
      if edata.alive and edata.faction ~= nil and edata.pilot ~= nil
            and edata.pilot:exists() then
         local f = faction.get(edata.faction)
         if f ~= nil and f:playerStanding() < 0 then
            edata.alive = false
            edata.pilot:setLeader(nil)
            edata.pilot:setNoClear(false)
            edata.pilot:hookClear()
         end
      end
   end
end


function hail( p )
   for i, edata in ipairs(escorts) do
      if edata.alive and edata.pilot == p then
         player.commClose()

         local approachtext = (
               pilot_action_text .. "\n\n" .. credentials:format(
                  edata.name, edata.ship, creditstring(edata.deposit),
                  edata.royalty * 100, creditstring(player.credits()),
                  getTotalRoyalties() * 100 ) )

         local n, s = tk.choice(
               "", approachtext, _("Fire pilot"), _("Do nothing") )
         if n == 1 and tk.yesno(
               "", string.format(
                  _("Are you sure you want to fire %s? This cannot be undone."),
                  edata.name ) ) then
            edata.alive = false
            edata.pilot:setLeader(nil)
            edata.pilot:setNoClear(false)
            edata.pilot:hookClear()
         end
      end
   end
end


function pilot_death( p, attacker, arg )
   escorts[arg].alive = false
end


function approachEscort( npc_id )
   local edata = npcs[npc_id]
   if edata == nil then
      evt.npcRm(npc_id)
      return
   end

   local approachtext = (
         pilot_action_text .. "\n\n" .. credentials:format(
            edata.name, edata.ship, creditstring(edata.deposit),
            edata.royalty * 100, creditstring(player.credits()),
            getTotalRoyalties() * 100 ) )

   local n, s = tk.choice("", approachtext, _("Fire pilot"), _("Do nothing"))
   if n == 1 then
      if tk.yesno(
            "", string.format(
               _("Are you sure you want to fire %s? This cannot be undone."),
               edata.name ) ) then
         evt.npcRm(npc_id)
         npcs[npc_id] = nil
         -- We just set alive to false for now and let them get cleaned
         -- up next time we land.
         edata.alive = false
      end
   end
end


function approachPilot( npc_id )
   local pdata = npcs[npc_id]
   if pdata == nil then
      evt.npcRm(npc_id)
      return
   end

   local cstr = credentials:format(
         pdata.name, pdata.ship, creditstring(pdata.deposit),
         pdata.royalty * 100, creditstring(player.credits()),
         getTotalRoyalties() * 100 )

   if tk.yesno("", pdata.approachtext .. "\n\n" .. cstr) then
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
   end
end

