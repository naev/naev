--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Warlords Battle">
 <location>enter</location>
 <chance>5</chance>
 <cond>system.cur():faction() == faction.get("Dvaered") and not player.evtActive ("Warlords Battle")</cond>
 <notes>
  <tier>3</tier>
 </notes>
</event>
--]]
--  A battle between two Dvaered warlords. The player can join one of them and get a reward
require "proximity"
local fmt = require "format"
local formation = require "formation"

-- Non-persistent state
local source_system, source_planet, battleEnded, side
local attackers, attnum, attkilled, attdeath, defenders, defnum, defkilled, defdeath
local trader, attAttHook, defAttHook, hailhook, jumphook
local reward
local finvader, flocal

function create ()
   -- Doesn't pilot.clear so inclusive claim
   if not evt.claim( system.cur(), false, true ) then
      evt.finish( false )
   end
   source_system = system.cur()
   jumphook = hook.jumpin("begin")
   hook.land("leave")
end

function begin ()
   local thissystem = system.cur()

   -- thissystem and source_system must be adjacent (for those who use player.teleport)
   local areAdj = false
   for _,s in ipairs( source_system:adjacentSystems() ) do
      if thissystem == s then areAdj = true end
   end

   if not evt.claim(thissystem) or not areAdj then
      evt.finish(false)
   end

   --choose 1 particular planet
   local plan = thissystem:spobs()
   local cand = {}
   local k = 1

   for i, j in ipairs(plan) do  --choose only Dvaered planets (and no stations)
      local classofj = j:class()
      if j:faction() == faction.get("Dvaered") and classofj ~= "0" and classofj ~= "1" and classofj ~= "2" then
         cand[k] = j
         k = k+1
      end
   end

   --If no planet matches the specs...
   if #cand <= 0 then
      evt.finish(false)
   end

   source_planet = cand[rnd.rnd(1,#cand)]

   local fdv = faction.get("Dvaered")
   finvader = faction.dynAdd( fdv, "warlords_invaders", _("Warlords") )
   flocal = faction.dynAdd( fdv, "warlords_locals", _("Warlords") )
   faction.dynEnemy( finvader, flocal )

   hook.timer(3.0, "merchant")
   hook.timer(7.0, "attack")
   hook.timer(12.0, "defense")
   battleEnded = false

   hook.rm(jumphook)
   hook.jumpout("leave")
end

--Spawns a merchant ship that explains what happens
function merchant ()
   local merShips = {"Koala", "Mule", "Rhino", "Llama"}
   local mship = ship.get( merShips[rnd.rnd(1,#merShips)] )
   trader = pilot.add( mship, "Trader", source_system, fmt.f(_("Trader {ship}"), {ship=mship} ) )
   hook.timer(2.0, "hailme")
end

function hailme()
    trader:hailPlayer()
    hailhook = hook.pilot(trader, "hail", "hail")
end

function hail()
   hook.rm(hailhook)
   tk.msg(_("A battle is about to begin"), fmt.f(_([["Hey, you," the captain of the ship says. "You seem not to know what is going to happen here: a mighty warlord from {sys} is going to attack {pnt}. You shouldn't stay there, unless you are a mercenary. Do you know how it works? If you attack a warlord's ship, and he loses the battle, the other warlord will reward you. But if he wins, you will be hunted down."]]), {sys=source_system, pnt=source_planet}))
   player.commClose()
end

--Arranges a list of pilot with their mass
local function arrangeList(list)
   local newlist = {}

   for i, j in ipairs(list) do
      local rank = 1
      for k, l in ipairs(list) do
         if j:stats().mass < l:stats().mass then
            rank = rank + 1
         end
      end

      --Processing of the equality case
      local again = true
      while again do
         if not newlist[rank] then
            newlist[rank] = j
            again = false
            else
            rank = rank + 1
         end
      end
   end

   return newlist
end

--chooses the first non nil pilot in a list
local function chooseInList(list)
   for i, p in ipairs(list) do
      if p ~= nil and p:exists() then
         return p
      end
   end
end

-- Prepares the reward
local function prepareReward(massOfVictims)
   local warrior = chooseInList(side == "attacker" and attackers or defenders)
   if warrior == nil then return end -- Simultaneous destruction?
   reward = 20e3 + 60*massOfVictims
   reward = reward * (1 + rnd.sigma() / 3)
   warrior:hailPlayer()
   hailhook = hook.pilot(warrior, "hail", "hailagain")
end

function hailagain()
   hook.rm(hailhook)
   tk.msg(_("Here comes your reward"), fmt.f( _([["Hello captain," a Dvaered officer says, "You helped us in this battle. I am authorized to give you {credits} as a reward."]]), {credits=fmt.credits(reward)}) )
   player.pay(reward)
   player.commClose()
end

-- Returns leader of fleet
local function getLeader(list)
   local p = chooseInList(list)
   if p == nil or not p:exists() then
      return nil
   end
   local l = p:leader()
   if l==nil then
      return p
   else
      return l
   end
end

local function landFleet(list)
   for i, p in ipairs(list) do
      if p ~= nil and p:exists() then
         p:control()
         p:land(source_planet)
      end
   end
end

function attack ()
   attAttHook = {}
   local n = rnd.rnd(3,6)
   local name = _("Invader")

   attackers = {}
   for i = 1, n do
      attackers[2*i-1] = pilot.add( "Dvaered Vendetta", "Dvaered", source_system, name )
      attackers[2*i]   = pilot.add( "Dvaered Ancestor", "Dvaered", source_system, name )
   end

   attackers[2*n+1] = pilot.add( "Dvaered Phalanx", "Dvaered", source_system, name )
   attackers[2*n+2] = pilot.add( "Dvaered Phalanx", "Dvaered", source_system, name )
   attackers[2*n+3] = pilot.add( "Dvaered Vigilance", "Dvaered", source_system, name )
   attackers[2*n+4] = pilot.add("Rhino", "Dvaered", source_system, name) --some transport ships
   attackers[2*n+5] = pilot.add("Rhino", "Dvaered", source_system, name)
   attackers[2*n+6] = pilot.add("Dvaered Arsenal", "Dvaered", source_system, name)
   local goda       = pilot.add( "Dvaered Goddard", "Dvaered", source_system, name )
   attackers[2*n+7] = goda

   -- The transport ships tend to run away
   attackers[2*n+4]:memory().shield_run = 70
   attackers[2*n+5]:memory().shield_run = 70
   attackers[2*n+6]:memory().shield_run = 70

   attackers[2*n+4]:memory().shield_return = 99
   attackers[2*n+5]:memory().shield_return = 99
   attackers[2*n+6]:memory().shield_return = 99

   attackers = arrangeList(attackers)  --The heaviest ships will surround the leader
   local form = formation.random_key()

   for i, j in ipairs(attackers) do
      j:setFaction( finvader )
      local m = j:memory()
      m.formation = form
      m.aggressive = false

      if j ~= goda then
         j:setLeader(goda)
      end

      attAttHook[i] = hook.pilot(j, "attacked", "attackerAttacked")
      hook.pilot(j, "death", "attackerDeath")
      hook.pilot(j, "jump", "attackerDeath")
      hook.pilot(j, "land", "attackerDeath")
   end

   attnum = #attackers
   attdeath = 0
   attkilled = 0  --mass of the player's victims

   goda:control()
   goda:land(source_planet)
end

function defense ()
   defAttHook = {}
   local n = rnd.rnd(3,6)
   local name = _("Local Warlord's Force")

   defenders = {}
   for i = 1, n do
      defenders[2*i-1] = pilot.add( "Dvaered Vendetta", "Dvaered", source_planet, name )
      defenders[2*i]   = pilot.add( "Dvaered Ancestor", "Dvaered", source_planet, name )
   end

   defenders[2*n+1] = pilot.add( "Dvaered Phalanx", "Dvaered", source_planet, name )
   defenders[2*n+2] = pilot.add( "Dvaered Phalanx", "Dvaered", source_planet, name )
   defenders[2*n+3] = pilot.add( "Dvaered Vigilance", "Dvaered", source_planet, name )
   local godd       = pilot.add( "Dvaered Goddard", "Dvaered", source_planet, name )
   defenders[2*n+4] = godd

   defenders = arrangeList(defenders)  --The heaviest ships will surround the leader
   local form = formation.random_key()

   for i, j in ipairs(defenders) do
      j:setFaction( flocal )
      local m = j:memory()
      m.formation = form
      m.aggressive = false

      if j ~= godd then
         j:setLeader(godd)
      end

      defAttHook[i] = hook.pilot(j, "attacked", "defenderAttacked")
      hook.pilot(j, "death", "defenderDeath")
      hook.pilot(j, "jump", "defenderDeath")
      hook.pilot(j, "land", "defenderDeath")
   end

   defnum = #defenders
   defdeath = 0
   defkilled = 0 --mass of the player's victims

   if attdeath >= attnum then  -- Special case: first fleet slaughtered before second fleet spawned!
      landFleet(defenders)
      battleEnded = true
      if side == "defender" then
         prepareReward(attkilled)
      end
   else
      local alead = getLeader(attackers)
      local dlead = getLeader(defenders)
      alead:taskClear()
      dlead:control()
      alead:attack(dlead)
      dlead:attack(alead)
      hook.timer(0.5, "proximity", {anchor = alead, radius = 5000, funcname = "startBattleIfReady", focus = dlead})
   end
end

-- Both fleets are close enough: start the epic battle
function startBattleIfReady()
   if attackers ~= nil and defenders ~= nil then
      local alead = getLeader(attackers)
      local dlead = getLeader(defenders)
      for i, p in ipairs(attackers) do
         if  p:exists() then
            p:memory().aggressive = true
         end
      end
      if alead then alead:control(false) end
      for i, p in ipairs(defenders) do
         if  p:exists() then
            p:memory().aggressive = true
         end
      end
      if dlead then dlead:control(false) end
   end
end

function defenderAttacked(_victim, attacker)
   --The player chose his side
   if attacker and attacker:withPlayer() then
      for i, p in ipairs(defenders) do
         hook.rm(defAttHook[i])
         if p ~= nil and p:exists() then
            p:setHostile()
         end
      end
      if side == "defender" then
         side = nil
         elseif side == nil then
         side = "attacker"
      end
   end
   startBattleIfReady()
end

function attackerAttacked(_victim, attacker)
   --The player chose his side
   if attacker and attacker:withPlayer() then
      for i, p in ipairs(attackers) do
         hook.rm(attAttHook[i])
         if p ~= nil and p:exists() then
            p:setHostile()
         end
      end
      if side == "attacker" then
         side = nil
         elseif side == nil then
         side = "defender"
      end
   end
   startBattleIfReady()
end

function attackerDeath(victim, arg)
   attdeath = attdeath + 1

   -- Credit the player if applicable. (This hook is overloaded; non-nil "arg.leader" means it's a death and "arg" is the killer.)
   local pp = player.pilot()
   if arg and arg.leader and (arg == pp or arg:leader() == pp) then
      attkilled = attkilled + victim:stats().mass
   end

   if not battleEnded and attdeath >= attnum then  --all the enemies are dead
      battleEnded = true
      landFleet(defenders)
      if side == "defender" then
         prepareReward(attkilled)
      end
   end
end

function defenderDeath(victim, arg)
   defdeath = defdeath + 1

   -- Credit the player if applicable. (This hook is overloaded; non-nil "arg.leader" means it's a death and "arg" is the killer.)
   local pp = player.pilot()
   if arg and arg.leader and (arg == pp or arg:leader() == pp) then
      defkilled = defkilled + victim:stats().mass
   end

   if not battleEnded and defdeath >= defnum then  -- all the defenders died: the winner lands on his planet
      battleEnded = true
      landFleet(attackers)
      if side == "attacker" then
         prepareReward(defkilled)
      end
   end
end

function leave ()
    evt.finish()
end
