--Escort a convoy of traders to a destination--

require "dat/scripts/nextjump.lua"
require "dat/scripts/cargo_common.lua"
require "dat/scripts/numstring.lua"

misn_title = _("Escort a %s convoy to %s in %s.")
misn_reward = _("%s credits")

convoysizestr = {}
convoysizestr[1] = _("tiny")
convoysizestr[2] = _("small") 
convoysizestr[3] = _("medium")
convoysizestr[4] = _("large")
convoysizestr[5] = _("huge")

title_p1 = _("A %s convoy of traders needs protection to %s in %s. You must stick with the convoy at all times, waiting to jump or land until the entire convoy has done so.")
   
-- Note: please leave the trailing space on the line below! Needed to make the newline show up.
title_p2 = _([[

Cargo: %s
Jumps: %d
Travel distance: %d
Piracy Risk: %s]])

piracyrisk = {}
piracyrisk[1] = _("None")
piracyrisk[2] = _("Low")
piracyrisk[3] = _("Medium")
piracyrisk[4] = _("High")

accept_title = _("Mission Accepted")

osd_title = _("Convey Escort")
osd_msg = _("Escort a convoy of traders to %s in the %s system")

slow = {}
slow[1] = _("Not enough fuel")
slow[2] = _([[The destination is %s jumps away, but you only have enough fuel for %s jumps. You cannot stop to refuel.

Accept the mission anyway?]])

landsuccesstitle = _("Success!")
landsuccesstext = _("You successfully escorted the trading convoy to the destination. There wasn't a single casualty, and you are rewarded the full amount.")

landcasualtytitle = _("Success with Casualities")
landcasualtytext = {}
landcasualtytext[1] = _("You've arrived with the trading convoy more or less intact. Your pay is docked slightly to provide compensation for the families of the lost crew members.")
landcasualtytext[2] = _("You arrive with what's left of the convoy. It's not much, but it's better than nothing. After a moment of silence of the lost crew members, you are paid a steeply discounted amount.")

wrongsystitle = _("You diverged!")
wrongsystext = _([[You have jumped to the wrong system! You are no longer part of the mission to escort the traders.]])

convoydeathtitle = _("The convoy has been destroyed!")
convoydeathtext = _([[All of the traders have been killed. You are a terrible escort.]])

landfailtitle = _("You abandoned your mission!")
landfailtext = _("You have landed, but you were supposed to escort the trading convoy. Your mission is a failure!")


convoynoruntitle = _("You have left your convoy behind!")
convoynoruntext = _([[You jumped before the rest of your convoy and the remaining ships scatter to find cover. You have abandoned your duties, and failed your mission.]])

convoynolandtitle = _("You landed before the convoy!")
convoynolandtext = _([[You landed at the planet before ensuring that the rest of your convoy was safe. You have abandoned your duties, and failed your mission.]])

traderdistress = _("Convoy ships under attack! Requesting immediate assistance!")


function create()
   --This mission does not make any system claims
   destplanet, destsys, numjumps, traveldist, cargo, avgrisk, tier = cargo_calculateRoute()
   
   if destplanet == nil then
      misn.finish(false)
   elseif numjumps == 0 then
      misn.finish(false) -- have to escort them at least one jump!
   elseif avgrisk * numjumps <= 25 then
      misn.finish(false) -- needs to be a little bit of piracy possible along route
   end
   
   if avgrisk == 0 then
      piracyrisk = piracyrisk[1]
   elseif avgrisk <= 25 then
      piracyrisk = piracyrisk[2]
   elseif avgrisk <= 100 then
      piracyrisk = piracyrisk[3]
   else
      piracyrisk = piracyrisk[4]
   end
    
   convoysize = rnd.rnd(1,5)
   
   -- Choose mission reward.
   -- Reward depends on type of cargo hauled. Hauling expensive commodities gives a better deal.
   if convoysize == 1 then
      jumpreward = 6*commodity.price(cargo)
      distreward = math.log(500*commodity.price(cargo))/100
   elseif convoysize == 2 then
      jumpreward = 7*commodity.price(cargo)
      distreward = math.log(700*commodity.price(cargo))/100
   elseif convoysize == 3 then
      jumpreward = 8*commodity.price(cargo)
      distreward = math.log(800*commodity.price(cargo))/100
   elseif convoysize == 4 then
      jumpreward = 9*commodity.price(cargo)
      distreward = math.log(900*commodity.price(cargo))/100
   elseif convoysize == 5 then
      jumpreward = 10*commodity.price(cargo)
      distreward = math.log(1000*commodity.price(cargo))/100
   end
   reward = 2.0 * (avgrisk * numjumps * jumpreward + traveldist * distreward) * (1. + 0.05*rnd.twosigma())
   
   misn.setTitle( misn_title:format( convoysizestr[convoysize], destplanet:name(), destsys:name() ) )
   misn.setDesc(title_p1:format(convoysizestr[convoysize], destplanet:name(), destsys:name()) .. title_p2:format(cargo, numjumps, traveldist, piracyrisk))
   misn.markerAdd(destsys, "computer")
   misn.setReward(misn_reward:format(numstring(reward)))
end

function accept()
   if convoysize == 1 then
      convoyname = "Trader Convoy 3"
      alive = {true, true, true} -- Keep track of the traders. Update this when they die.
      alive["__save"] = true
   elseif convoysize == 2 then
      convoyname = "Trader Convoy 4"
      alive = {true, true, true, true}
      alive["__save"] = true
   elseif convoysize == 3 then
      convoyname = "Trader Convoy 5"
      alive = {true, true, true, true, true}
      alive["__save"] = true
   elseif convoysize == 4 then
      convoyname = "Trader Convoy 6"
      alive = {true, true, true, true, true, true}
      alive["__save"] = true
   elseif convoysize == 5 then
      convoyname = "Trader Convoy 8"
      alive = {true, true, true, true, true, true, true, true}
      alive["__save"] = true      
   end
 
   if player.jumps() < numjumps then
      if not tk.yesno( slow[1], slow[2]:format( numjumps, player.jumps())) then
         misn.finish()
      end
   end
    
   nextsys = getNextSystem(system.cur(), destsys) -- This variable holds the system the player is supposed to jump to NEXT.
   origin = planet.cur() -- The place where the AI ships spawn from.
   
   misnfail = false
   unsafe = false

   misn.accept()
   misn.osdCreate(osd_title, {osd_msg:format(destplanet:name(), destsys:name())})
   
   hook.takeoff("takeoff")
   hook.jumpin("jumpin")
   hook.jumpout("jumpout")
   hook.land("land")
end

function takeoff()
   spawnConvoy()
end

function jumpin()
   if system.cur() ~= nextsys then -- player jumped to somewhere other than the next system
      tk.msg(wrongsystitle, wrongsystext)
      misn.finish(false)
   elseif misnfail then -- Jumped ahead of traders. Convoy appears in next system,
      hook.timer(4000, "convoyContinues") -- waits 4 secs then has them follow.
   else -- Continue convoy
      spawnConvoy()
   end
end

function jumpout()
   if not convoyJump then
      tk.msg(convoynoruntitle, convoynoruntext)
      misnfail = true
   end
   origin = system.cur()
   nextsys = getNextSystem(system.cur(), destsys)
end

function land()
   dead = 0
   for k,v in pairs(alive) do
      if v ~= true then
      dead = dead + 1
      end
   end
   if planet.cur() ~= destplanet then
      tk.msg(landfailtitle, landfailtext)
      misn.finish(false)
   elseif planet.cur() == destplanet and not convoyLand then
      tk.msg(convoynolandtitle, convoynolandtext)
      misn.finish(false)
   elseif planet.cur() == destplanet and convoyLand and convoysize == 1 then
      if dead == 0 then
         tk.msg(landsuccesstitle, landsuccesstext)
         player.pay(reward)
         misn.finish(true)
      elseif dead == 1 then
         tk.msg(landcasualtytitle, landcasualtytext[1])
         player.pay(reward/2)
         misn.finish(true)
      elseif dead == 2 then
         tk.msg(landcasualtytitle, landcasualtytext[2])
         player.pay(reward/4)
         misn.finish(true)
      end
   elseif planet.cur() == destplanet and convoyLand == true and convoysize == 2 then
      if dead == 0 then
         tk.msg(landsuccesstitle, landsuccesstext)
         player.pay(reward)
         misn.finish(true)
      elseif dead > 0 and dead < 3 then
         tk.msg(landcasualtytitle, landcasualtytext[1])
         player.pay(reward/2)
         misn.finish(true)
      elseif dead == 3 then
         tk.msg(landcasualtytitle, landcasualtytext[2])
         player.pay(reward/4)
         misn.finish(true)
      end
   elseif planet.cur() == destplanet and convoyLand == true and convoysize == 3 then
      if dead == 0 then
         tk.msg(landsuccesstitle, landsuccesstext)
         player.pay(reward)
         misn.finish(true)
      elseif dead > 0 and dead < 3 then
         tk.msg(landcasualtytitle, landcasualtytext[1])
         player.pay(reward/2)
         misn.finish(true)
      elseif dead >= 3 then
         tk.msg(landcasualtytitle, landcasualtytext[2])
         player.pay(reward/4)
         misn.finish(true)
      end
   elseif planet.cur() == destplanet and convoyLand == true and convoysize == 4 then
      if dead == 0 then
         tk.msg(landsuccesstitle, landsuccesstext)
         player.pay(reward)
         misn.finish(true)
      elseif dead <= 3 then
         tk.msg(landcasualtytitle, landcasualtytext[1])
         player.pay(reward/2)
         misn.finish(true)
      elseif dead > 3 then
         tk.msg(landcasualtytitle, landcasualtytext[2])
         player.pay(reward/4)
         misn.finish(true)
      end
   elseif planet.cur() == destplanet and convoyLand == true and convoysize == 5 then
      if dead == 0 then
         tk.msg(landsuccesstitle, landsuccesstext)
         player.pay(reward)
         misn.finish(true)
      elseif dead <= 4 then
         tk.msg(landcasualtytitle, landcasualtytext[1])
         player.pay(reward/2)
         misn.finish(true)
      elseif dead > 4 then
         tk.msg(landcasualtytitle, landcasualtytext[2])
         player.pay(reward/4)
         misn.finish(true)
      end
   end
end

function traderDeath()
   if convoysize == 1 then
      if alive[3] then alive[3] = false
      elseif alive[2] then alive[2] = false
      else -- all convoy dead
         tk.msg(convoydeathtitle, convoydeathtext)
         misn.finish(false)
      end
   elseif convoysize == 2 then
      if alive[4] then alive[4] = false
      elseif alive[3] then alive[3] = false
      elseif alive[2] then alive[2] = false
      else -- all convoy dead
         tk.msg(convoydeathtitle, convoydeathtext)
         misn.finish(false)
      end
   elseif convoysize == 3 then
      if alive[5] then alive[5] = false
      elseif alive[4] then alive[4] = false
      elseif alive[3] then alive[3] = false
      elseif alive[2] then alive[2] = false
      elseif alive[1] then alive[1] = false
      else -- all convoy dead
         tk.msg(convoydeathtitle, convoydeathtext)
         misn.finish(false)
      end
   elseif convoysize == 4 then
      if alive[6] then alive[6] = false
      elseif alive[5] then alive[5] = false
      elseif alive[4] then alive[4] = false
      elseif alive[3] then alive[3] = false
      elseif alive[2] then alive[2] = false
      else -- all convoy dead
         tk.msg(convoydeathtitle, convoydeathtext)
         misn.finish(false)
      end
   elseif convoysize == 5 then
      if alive[8] then alive[8] = false
      elseif alive[7] then alive[7] = false
      elseif alive[6] then alive[6] = false
      elseif alive[5] then alive[5] = false
      elseif alive[4] then alive[4] = false
      elseif alive[3] then alive[3] = false
      elseif alive[2] then alive[2] = false
      else -- all convoy dead
         tk.msg(convoydeathtitle, convoydeathtext)
         misn.finish(false)
      end
   end
end

-- Handle the jumps of convoy.
function traderJump()
   convoyJump = true
   local broadcast = false
   for i, j in pairs(convoy) do
      if j:exists() and broadcast == false then
         player.msg(string.format("%s has jumped to %s.", j:name(),getNextSystem(system.cur(), destsys):name()))
         broadcast = true
      end
   end
end

--Handle landing of convoy
function traderLand()
   convoyLand = true
   local broadcast = false
   for i, j in pairs(convoy) do
      if j:exists() and broadcast == false then
         player.msg(string.format("%s has landed on %s.", j:name(),destplanet:name()))
         broadcast = true
      end
   end
end


-- Handle the convoy getting attacked.
function traderAttacked( p, attacker )
   unsafe = true
   p:control( false )
   p:hookClear()
   p:setNoJump( true )
   p:setNoLand( true )

   if not shuttingup then
      shuttingup = true
      p:comm( player.pilot(), traderdistress )
      hook.timer( 5000, "traderShutup" ) -- Shuts him up for at least 5s.
   end
end

function traderShutup()
    shuttingup = false
end

function timer_traderSafe()
   hook.timer( 2000, "timer_traderSafe" )

   if unsafe then
      unsafe = false
      for i, j in ipairs( convoy ) do
         continueToDest( j )
      end
   end
end

function convoyContinues()
   convoy = pilot.add(convoyname, nil, origin) 
   for i, j in ipairs(convoy) do
      if not alive[i] then j:rm() end -- Dead traders stay dead.
      if j:exists() then
         j:changeAI( "trader" )
         j:control(false)
      end
   end 
   misn.finish(false)
end

function spawnConvoy ()
   convoyLand = false
   convoyJump = false

   --Make it interesting
   if convoysize == 1 then
      ambush = pilot.add("Trader Ambush 1", "baddie_norun", vec2.new(0, 0))
   elseif convoysize == 2 then
      ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(1,2)), "baddie_norun", vec2.new(0, 0))
   elseif convoysize == 3 then
      ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(2,3)), "baddie_norun", vec2.new(0, 0))
   elseif convoysize == 4 then
      ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(2,4)), "baddie_norun", vec2.new(0, 0))
   else
      ambush = pilot.add(string.format("Trader Ambush %i", rnd.rnd(3,5)), "baddie_norun", vec2.new(0, 0))
   end

   --Spawn the convoy
   convoy = pilot.add(convoyname, nil, origin)
   for i, j in ipairs(convoy) do
      if not alive[i] then j:rm() end -- Dead traders stay dead.
         if j:exists() then
            j:rmOutfit( "cores" )
            local class = j:ship():class()
            if class == "Yacht" or class == "Luxury Yacht" or class == "Scout"
                  or class == "Courier" or class == "Fighter" or class == "Bomber"
                  or class == "Drone" or class == "Heavy Drone" then
               j:addOutfit( "Unicorp PT-200 Core System" )
               j:addOutfit( "Melendez Ox XL Engine" )
               j:addOutfit( "S&K Small Cargo Hull" )
            elseif class == "Freighter" or class == "Armoured Transport"
                  or class == "Corvette" or class == "Destroyer" then
               j:addOutfit( "Unicorp PT-600 Core System" )
               j:addOutfit( "Melendez Buffalo XL Engine" )
               j:addOutfit( "S&K Medium Cargo Hull" )
            elseif class == "Cruiser" or class == "Carrier" then
               j:addOutfit( "Unicorp PT-600 Core System" )
               j:addOutfit( "Melendez Mammoth XL Engine" )
               j:addOutfit( "S&K Large Cargo Hull" )
            end

            j:setHealth( 100, 100 )
            j:setEnergy( 100 )
            j:setTemp( 0 )
            j:setFuel( true )
            j:cargoAdd( cargo, j:cargoFree() )

            j:control()
            j:setHilight(true)
            j:setInvincPlayer()
            continueToDest( j )

            hook.pilot(j, "death", "traderDeath")
            hook.pilot(j, "attacked", "traderAttacked", j)
      end
   end

   hook.timer( 1000, "timer_traderSafe" )
end

function continueToDest( p )
   if p ~= nil and p:exists() then
      p:control( true )
      p:setNoJump( false )
      p:setNoLand( false )

      if system.cur() == destsys then
         p:land( destplanet )
         hook.pilot( p, "land", "traderLand" )
      else
         p:hyperspace( getNextSystem( system.cur(), destsys ), true )
         hook.pilot( p, "jump", "traderJump" )
      end
   end
end
