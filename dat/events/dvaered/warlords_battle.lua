--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Warlords battle">
  <trigger>enter</trigger>
  <chance>5</chance>
  <cond>system.cur():faction() == faction.get("Dvaered") and not player.evtActive ("Warlords battle")</cond>
  <flags>
  </flags>
  <notes>
   <tier>3</tier>
  </notes>
 </event>
 --]]
--  A battle between two Dvaered warlords. The player can join one of them and get a reward

require "fleethelper"
require "proximity"
require "numstring"
local formation = require "formation"


title = {}
text = {}

title[1] = _("A battle is about to begin")
text[1] = _([["Hey, you," the captain of the ship says. "You seem not to know what is going to happen here: a mighty warlord from %s is going to attack %s. You shouldn't stay there, unless you are a mercenary. Do you know how it works? If you attack a warlord's ship, and he loses the battle, the other warlord will reward you. But if he wins, you will be hunted down."]])

title[2] = _("Here comes your reward")
text[2] = _([["Hello captain," a Dvaered officer says, "You helped us in this battle. I am authorized to give you %s as a reward."]])

function create ()
   source_system = system.cur()
   jumphook = hook.jumpin("begin")
   landhook = hook.land("leave")
end

function begin ()
   thissystem = system.cur()

   -- thissystem and source_system must be adjacent (for those who use player.teleport)
   areAdj = false
   for _,s in ipairs( source_system:adjacentSystems() ) do
      if thissystem == s then areAdj = true end
   end

   if not evt.claim(thissystem) or not areAdj then
      evt.finish(false)
   end

   --choose 1 particular planet
   plan = thissystem:planets()
   cand = {}
   k = 1

   for i, j in ipairs(plan) do  --choose only Dvaered planets (and no stations)
      classofj = j:class()
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

   hook.timer(3000, "merchant")
   hook.timer(7000, "attack")
   hook.timer(12000, "defense")
   inForm       = true -- People are in formation
   batInProcess = true -- Battle is happening

   hook.rm(jumphook)
   hook.jumpout("leave")
end

--Spawns a merchant ship that explains what happens
function merchant ()
   merShips = {"Koala", "Mule", "Rhino", "Llama"}
   mship = merShips[rnd.rnd(1,#merShips)]
   trader = pilot.add( mship, "Trader", source_system, _("Trader %s"):format( _(mship) ) )
   hook.timer(2000, "hailme")
end

function hailme()
    trader:hailPlayer()
    hailhook = hook.pilot(trader, "hail", "hail")
end

function hail()
   hook.rm(hailhook)
   tk.msg(title[1], text[1]:format(source_system:name(), source_planet:name()))
end

function hailmeagain()
    warrior:hailPlayer()
    hailhook = hook.pilot(warrior, "hail", "hailagain")
end

function hailagain()
   hook.rm(hailhook)
   tk.msg(title[2], text[2]:format(creditstring(reward)))
   player.pay(reward)
end

function attack ()
   attAttHook = {}
   local n = rnd.rnd(3,6)

   attackers = addShips(n, {"Dvaered Vendetta", "Dvaered Ancestor"}, "Dvaered", source_system) -- Give them Dvaered equipment
   attackers[2*n+1] = pilot.add( "Dvaered Phalanx", "Dvaered", source_system )
   attackers[2*n+2] = pilot.add( "Dvaered Phalanx", "Dvaered", source_system )
   attackers[2*n+3] = pilot.add( "Dvaered Vigilance", "Dvaered", source_system )
   attackers[2*n+4] = pilot.add("Rhino", "Dvaered", source_system) --some transport ships
   attackers[2*n+5] = pilot.add("Rhino", "Dvaered", source_system)
   attackers[2*n+6] = pilot.add("Rhino", "Dvaered", source_system)
   attackers[2*n+7] = pilot.add("Rhino", "Dvaered", source_system)
   goda             = pilot.add( "Dvaered Goddard", "Dvaered", source_system )
   attackers[2*n+8] = goda

   -- The transport ships tend to run away
   attackers[2*n+4]:memory().shield_run = 70
   attackers[2*n+5]:memory().shield_run = 70
   attackers[2*n+6]:memory().shield_run = 70
   attackers[2*n+7]:memory().shield_run = 70

   attackers[2*n+4]:memory().shield_return = 99
   attackers[2*n+5]:memory().shield_return = 99
   attackers[2*n+6]:memory().shield_return = 99
   attackers[2*n+7]:memory().shield_return = 99

   attackers = arrangeList(attackers)  --The heaviest ships will surround the leader
   form = formation.random_key()

   -- I use Thugs and Associates based factions because they won't interact with anybody
   f1 = faction.dynAdd( "Thugs", "Invaders", _("Warlords") )

   for i, j in ipairs(attackers) do
      j:rename("Invader")
      j:setFaction("Invaders")
      j:memory().formation = form
      j:memory().aggressive = false

      if j ~= goda then
         j:setLeader(goda)
      end

      attAttHook[i] = hook.pilot(j, "attacked", "attackerAttacked")
      hook.pilot(j, "death", "attackerDeath")
      hook.pilot(j, "jump", "attackerDeath")
      hook.pilot(j, "land", "attackerDeath")
   end

   attnum = table.getn(attackers)
   attdeath = 0
   attkilled = 0  --mass of the player's victims

   local lead = getLeader(attackers)
   lead:control()
   lead:land(source_planet)
end

function defense ()
   defAttHook = {}
   local n = rnd.rnd(3,6)

   defenders = addShips(n, {"Dvaered Vendetta", "Dvaered Ancestor"}, "Dvaered", source_planet)
   defenders[2*n+1] = pilot.add( "Dvaered Phalanx", "Dvaered", source_planet )
   defenders[2*n+2] = pilot.add( "Dvaered Phalanx", "Dvaered", source_planet )
   defenders[2*n+3] = pilot.add( "Dvaered Vigilance", "Dvaered", source_planet )
   godd             = pilot.add( "Dvaered Goddard", "Dvaered", source_planet )
   defenders[2*n+4] = godd

   defenders = arrangeList(defenders)  --The heaviest ships will surround the leader
   form = formation.random_key()

   f2 = faction.dynAdd( "Associates", "Locals", _("Warlords") )
   faction.dynEnemy (f1, f2)
   faction.dynEnemy (f2, f1)

   for i, j in ipairs(defenders) do
      j:rename("Local Warlord's Force")
      j:setFaction("Locals")
      j:memory().formation = form
      j:memory().aggressive = false

      if j ~= godd then
         j:setLeader(godd)
      end

      defAttHook[i] = hook.pilot(j, "attacked", "defenderAttacked")
      hook.pilot(j, "death", "defenderDeath")
      hook.pilot(j, "jump", "defenderDeath")
      hook.pilot(j, "land", "defenderDeath")
   end

   defnum = table.getn(defenders)
   defdeath = 0
   defkilled = 0 --mass of the player's victims

   local alead = getLeader(attackers)
   local dlead = getLeader(defenders)
   alead:taskClear()
   dlead:control()
   alead:attack(dlead)
   dlead:attack(alead)
   hook.timer(500, "proximity", {anchor = alead, radius = 5000, funcname = "startBattle", focus = dlead})
end

-- Both fleets are close enough: start the epic battle
function startBattle()
   if inForm then
      for i, p in ipairs(attackers) do
         p:memory().aggressive = true
      end
      getLeader(attackers):control(false)
      for i, p in ipairs(defenders) do
         p:memory().aggressive = true
      end
      getLeader(defenders):control(false)
      inForm = false
   end
end

function defenderAttacked(victim, attacker)
   --The player chose his side
   if attacker == player.pilot() or attacker:leader() == player.pilot() then
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
   if inForm then
      startBattle()
   end
end

function attackerAttacked(victim, attacker)
   --The player chose his side
   if attacker == player.pilot() or attacker:leader() == player.pilot() then
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
   if inForm then
      startBattle()
   end
end

function attackerDeath(victim, attacker)
   if batInProcess then
      attdeath = attdeath + 1

      if attacker == player.pilot() or attacker:leader() == player.pilot() then
         attkilled = attkilled + victim:stats().mass
      end

      if attdeath >= attnum then  --all the enemies are dead
         local lead = getLeader(defenders)
         lead:control()
         lead:land(source_planet)
         batInProcess = false -- Battle ended

         --Time to get rewarded
         if side == "defender" then
            warrior = chooseInList(defenders)
            computeReward(true, attkilled)
            hook.timer(1000, "hailmeagain")
         end
      end
   end
end

function defenderDeath(victim, attacker)
   if batInProcess then
      defdeath = defdeath + 1

      if attacker == player.pilot() or attacker:leader() == player.pilot() then
         defkilled = defkilled + victim:stats().mass
      end

      if defdeath >= defnum then  -- all the defenders died : the winner lands on his planet
         local lead = getLeader(attackers)
         lead:control()
         lead:land(source_planet)
         batInProcess = false -- Battle ended

         --Time to get rewarded
         if side == "attacker" then
            warrior = chooseInList(attackers)
            computeReward(true, defkilled)
            hook.timer(1000, "hailmeagain")
         end
      end
   end
end

--Computes the reward
function computeReward(attack, massOfVictims)
   if attack == true then
      baserew = 20000
   end
   baserew = baserew + 60*massOfVictims

   reward = baserew + rnd.sigma() * (baserew/3)
end

-- Returns leader of fleet
function getLeader(list)
   local p = chooseInList(list)
   if p:leader() == nil or not p:leader():exists() then
      return p
   else
      return p:leader()
   end
end

--chooses the first non nil pilot in a list
function chooseInList(list)
   for i, p in ipairs(list) do
      if p ~= nil and p:exists() then
         return p
      end
   end
end

--Arranges a list of pilot with their mass
function arrangeList(list)
   newlist = {}

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

function leave ()
    evt.finish()
end
