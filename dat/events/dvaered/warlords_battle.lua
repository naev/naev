--  A battle between two dvaered warlords. The player can join one of them and get a reward

include "fleethelper.lua"
include "fleet_form.lua"
include "proximity.lua"
include "numstring.lua"

lang = naev.lang()
if lang == "es" then
   else -- default english

   title = {}
   text = {}

   title[1] = "A battle is about to begin"
   text[1] = [["Hey, you," the captain of the ship says. "You seem not to know what is going to happen here: a mighty warlord from %s is going to attack %s. You shouldn't stay there, unless you are a mercenary. Do you know how it works? If you attack a warlord's ship, and he loses the battle, the other warlord will reward you. But if he wins, you will be hunted down."]]

   title[2] = "Here comes your reward"
   text[2] = [["Hello captain," a Dvaered officer says, "You helped us in this battle. I am authorized to give you %s credits as a reward."]]

end

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

   for i, j in ipairs(plan) do  --choose only dvaered planets (and no stations)
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

   hook.rm(jumphook)
   hook.jumpout("leave")

end

--Spawns a merchant ship that explains what happends
function merchant ()
   merShips = {"Trader Koala", "Trader Mule", "Trader Rhino", "Trader Llama"}
   mship = merShips[rnd.rnd(1,#merShips)]
   trader = pilot.add(mship, nil, source_system)[1]
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
   tk.msg(title[2], text[2]:format(numstring(reward)))
   player.pay(reward)
end

function attack ()

   attAttHook = {}
   local n = rnd.rnd(3,6)

   attackers = addShips({"Dvaered Vendetta", "Dvaered Ancestor"}, nil, source_system, n)
   attackers[2*n+1] = pilot.add("Dvaered Phalanx", nil, source_system)[1]
   attackers[2*n+2] = pilot.add("Dvaered Phalanx", nil, source_system)[1]
   attackers[2*n+3] = pilot.add("Dvaered Vigilance", nil, source_system)[1]
   attackers[2*n+4] = pilot.addRaw("Rhino", "dvaered", source_system, "Thugs")[1] --some transport ships
   attackers[2*n+5] = pilot.addRaw("Rhino", "dvaered",source_system, "Thugs")[1]
   attackers[2*n+6] = pilot.addRaw("Rhino", "dvaered",source_system, "Thugs")[1]
   attackers[2*n+7] = pilot.addRaw("Rhino", "dvaered",source_system, "Thugs")[1]
   attackers[2*n+8] = pilot.add("Dvaered Goddard", nil, source_system)[1]

   -- The transport ships tend to run away
   attackers[2*n+4]:memory("shield_run", 70)
   attackers[2*n+5]:memory("shield_run", 70)
   attackers[2*n+6]:memory("shield_run", 70)
   attackers[2*n+7]:memory("shield_run", 70)

   attackers[2*n+4]:memory("shield_return", 99)
   attackers[2*n+5]:memory("shield_return", 99)
   attackers[2*n+6]:memory("shield_return", 99)
   attackers[2*n+7]:memory("shield_return", 99)

   attackers = arrangeList(attackers)  --The heaviest ships will surround the leader

   for i, j in ipairs(attackers) do
      j:rename("Invader")
      j:setFaction("Thugs")  --I use Thugs and Associates because they won't interact with anybody

      attAttHook[i] = hook.pilot(j, "attacked", "attackerAttacked")
      hook.pilot(j, "death", "attackerDeath")
      hook.pilot(j, "jump", "attackerDeath")
      hook.pilot(j, "land", "attackerDeath")
   end

   formations = {"echelon left", "cross", "echelon right", "wedge", "wall", "vee", "column", "circle", "fishbone", "chevron"}
   form = formations[rnd.rnd(9)+1]
   attnum = table.getn(attackers)
   attdeath = 0
   attkilled = 0  --mass of the player's victims

   atFleet = Forma:new(attackers, form, 3000)
   atFleet:setTask("land",source_planet)

end

function defense ()

   defAttHook = {}
   local n = rnd.rnd(3,6)

   defenders = addShips({"Dvaered Vendetta", "Dvaered Ancestor"}, nil, source_planet, n)
   defenders[2*n+1] = pilot.add("Dvaered Phalanx", nil, source_planet)[1]
   defenders[2*n+2] = pilot.add("Dvaered Phalanx", nil, source_planet)[1]
   defenders[2*n+3] = pilot.add("Dvaered Vigilance", nil, source_planet)[1]
   defenders[2*n+4] = pilot.add("Dvaered Goddard", nil, source_planet)[1]

   defenders = arrangeList(defenders)  --The heaviest ships will surround the leader

   for i, j in ipairs(defenders) do
      j:rename("Local Warlord's Force")
      j:setFaction("Associates")

      defAttHook[i] = hook.pilot(j, "attacked", "defenderAttacked")
      hook.pilot(j, "death", "defenderDeath")
      hook.pilot(j, "jump", "defenderDeath")
      hook.pilot(j, "land", "defenderDeath")
   end

   defnum = table.getn(defenders)
   defdeath = 0
   defkilled = 0 --mass of the player's victims
   formations = {"echelon left", "cross", "echelon right", "wedge", "wall", "vee", "column", "circle", "fishbone", "chevron", "buffer"}
   form = formations[rnd.rnd(9)+1]

   deFleet = Forma:new(defenders, form, 3000)
   deFleet:setTask("follow", chooseInList(attackers))

end

function defenderAttacked(victim, attacker)
   --The player choosed his side
   if attacker == player.pilot() then
      for i, j in ipairs(defenders) do
         hook.rm(defAttHook[i])
         j:setHostile()
      end
      if side == "defender" then
         side = nil
         elseif side == nil then
         side = "attacker"
      end
   end
end

function attackerAttacked(victim, attacker)
   --The player choosed his side
   if attacker == player.pilot() then
      for i, j in ipairs(attackers) do
         hook.rm(attAttHook[i])
         j:setHostile()
      end
      if side == "attacker" then
         side = nil
         elseif side == nil then
         side = "defender"
      end
   end
end

function attackerDeath(victim, attacker)
   attdeath = attdeath + 1

   if attacker == player.pilot() then
      attkilled = attkilled + victim:stats().mass
   end

   if attdeath < attnum then
      deFleet:setTask("follow", chooseInList(attackers))

   else  --all the enemes are dead
      deFleet:setTask("land", source_planet)

      --Time to get rewarded
      if side == "defender" then
         warrior = chooseInList(defenders)
         computeReward(true, attkilled)
         hook.timer(1000, "hailmeagain")
      end
   end
end

function defenderDeath(victim, attacker)
   defdeath = defdeath + 1

   if attacker == player.pilot() then
      defkilled = defkilled + victim:stats().mass
   end

   if defdeath >= defnum then   --all the defenders died : the winner lands on his planet

      --Time to get rewarded
      if side == "attacker" then
         warrior = chooseInList(attackers)
         computeReward(true, defkilled)
         hook.timer(1000, "hailmeagain")
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

--chooses the first non nil pilot in a list
function chooseInList(list)
   for i, j in ipairs(list) do
      if j:exists() then
         return j
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
