require 'ai.escort'

-- Settings
mem.aggressive = true
mem.guardbrake    = 350
mem.guarddodist   = 3000 -- distance at which to start activities
mem.guardreturndist = 6000 -- distance at which to return
mem.enemyclose    = mem.guarddodist
mem.atk_board = true
mem.send_escort = true
mem.autoleader =  false

function create ()
   create_pre()

   local p = ai.pilot()
   local ps = p:ship()

   -- Default range stuff
   mem.guarddodist   = 1000 + 200 * ps:size()
   mem.guardreturndist = mem.guarddodist + 2000
   mem.enemyclose    = mem.guarddodist

   -- Finish up creation
   create_post()
end

-- is it close to me or my protectee?
local function gdist( t )
   return math.min(
      player.pilot():pos():dist( t:pos() ),
      ai.pilot():pos():dist( t:pos() )
   )
end

local _should_attack = should_attack
function should_attack( enemy, si )
   if _should_attack( enemy, si ) and gdist(enemy) < mem.guarddodist then
      return true
   end
   return false
end

-- luacheck: globals idle (AI Task functions passed by name)
function idle ()

   local pp = player.pilot()
   local me = ai.pilot()
   local subordinates = me:followers()

   local hostiles = pp:getHostiles(mem.guarddodist, me:pos(), true, false, false)
   local detected_hostile = nil
   local can_send = rnd.rnd(1, #subordinates * 2) == #subordinates

   for i, enemy in ipairs(hostiles) do
      if should_attack(enemy) then
         local dangerous = false
         if enemy:ship():size() >= 3 then
            dangerous = true
         end

         if dangerous then
            for j, under in ipairs(subordinates) do
               me:msg(under, "e_attack", enemy)
--               under:pushtask( "attack", enemy )
            end
            ai.pushtask( "attack", enemy )
            return
         else
            -- send a random escort
            if (can_send or mem.send_escort) and #subordinates > 0 then
               me:msg(subordinates[rnd.rnd(1, #subordinates)], "e_attack", enemy)
               mem.send_escort = false
            end
         end
      end
   end

   -- get distance
   local guarddist = ai.dist(pp:pos())
   if guarddist > mem.guardreturndist then
         ai.iface(pp)
      ai.accel(1)
      return
   end

   -- find something to board
   hostiles = pilot.getInrange(me:pos(), mem.guarddodist)
   for _i, target in ipairs(hostiles) do
      if target:hostile() then
         if mem.atk_board and ai.canboard(target) then
            ai.pushtask("board", target )
            return
         elseif (should_attack(target) or (mem.aggressive and not ai.canboard(target))) and rnd.rnd(0, 1) == 0 then
            -- a pot shot or two
            ai.settarget( target )
            ai.aim(target)
            ai.shoot()
         end
      end
   end

      -- find nearby enemy
   local enemy = detected_hostile
   if not enemy then
      enemy = ai.getenemy()
   end
   if enemy ~= nil then
      for k, v in ipairs(me:followers()) do
         me:msg ( v, "e_attack", enemy )
      end
      ai.pushtask("attack", enemy)
   end

   mem.send_escort = true

   if guarddist < mem.guardbrake then
      -- let him drift a bit
      if rnd.rnd() > (guarddist / mem.guardbrake) then
         -- brake
         ai.brake()
      else
         -- just don't accelerate
         ai.accel(0)
      end
      return
   elseif guarddist < mem.guarddodist then
      ai.face( pp, nil, true )
      ai.accel((guarddist / mem.guarddodist) * rnd.rnd()) -- fun variability
      return
   end

   ai.iface(pp)
   ai.accel()
end
