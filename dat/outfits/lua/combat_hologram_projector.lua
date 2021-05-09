cooldown = 20
active = 15
dist = 3000 -- Distance to possible change target of hostiles
hologram = outfit.get("Combat Hologram Projector")

function turnon( p, po )
   -- Make sure pilot has a target
   local t = p:target()
   if t == nil then return false end

   -- Set outfit state
   mem.timer = active
   po:state("on")

   -- Create hologram at the same position
   -- TODO hologram-specific AI?
   local s = p:ship()
   local np = pilot.add( s:nameRaw(), p:faction(), p:pos(), p:name(), "escort" )
   mem.p = np
   np:setHealth( p:health() ) -- Copy health
   np:setNoDeath( true ) -- Dosen't die
   -- Copy outfits
   np:rmOutfit("all")
   for k,v in ipairs(p:outfits()) do
      -- We don't want holograms and fighter bays to create weird loops
      if v ~= hologram and v:typeBroad()~="Fighter Bay" then
         np:addOutfit( v, 1, true )
      end
   end
   -- No damage and low health
   np:intrinsicSet( {
      launch_damage  = -1000,
      fbay_damage    = -1000, -- shouldn't have fighter bays, but just incase
      fwd_damage     = -1000,
      tur_damage     = -1000,
      armour_mod     = -50,
      shield_mod     = -50,
   }, true ) -- overwrite all
   -- Don't let player attack their own hologram
   if mem.isp then
      np:setInvincPlayer(true)
   end
   -- Exact same position and direction as pilot
   np:setDir( p:dir() )
   np:setVel( p:vel() )
   np:control( true )
   local bt = s:baseType()
   if bt=="Yacht" or bt=="Luxury Yacht" or bt=="Cruise Ship" or bt=="Courier" or bt=="Freighter" or bt=="Bulk Carrier" then
      -- Run away for civilian ships
      np:runaway( t, true )
   else
      -- Attacks pilot's target for non civilian ships
      np:attack( t )
   end

   -- Modify randomly targetting of hostiles (probably don't have to go over all ships)
   for k,v in ipairs(p:getHostiles(dist)) do
      if v:target()==p and rnd.rnd() > 0.5 then
         v:setTarget(np)
      end
   end

   return true
end
function turnoff( po )
   -- Get rid of hologram
   if mem.p:exists() then
      mem.p:rm()
   end
   mem.p = nil

   -- Set outfit state
   mem.timer = cooldown
   po:state("cooldown")
end

function init( p, po )
   po:state("off")
   mem.p = nil
   mem.timer = 0
   mem.isp = player.pilot()==p -- is player?
end

function update( p, po, dt )
   mem.timer = mem.timer - dt
   if mem.p then
      if not mem.p:exists() or mem.timer < 0 or mem.p:health(true) < 5 then
         turnoff( po )
      end
   else
      if mem.timer < 0 then
         po:state("off")
      end
   end
end

-- This should trigger when the pilot is disabled or killed and destroy the
-- hologram if it is up
function ontoggle( p, po, on )
   if on then
      -- Not ready yet
      if mem.timer > 0 then return end

      return turnon( p, po )
   else
      if mem.p then
         turnoff( po )
         return true
      end
   end
   return false
end
