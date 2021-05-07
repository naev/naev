-- Global constant variables for the outfit
cooldown = 8 -- cooldown period in seconds
ontime = 3 -- powered on time in seconds (it gets modulated by time_mod)

-- Init function run on creation
function init( p, po )
   mem.timer = 0
   mem.active = false
   po:state( "off" )
end

function update( p, po, dt )
   mem.timer = mem.timer - dt
   -- If active, we run until end
   if mem.active then
      if mem.timer <= 0 then
         mem.timer = cooldown
         mem.active = false
         po:state( "cooldown" )
         return
      end
   else
      if mem.timer <= 0 then
         po:state( "off" )
      end
   end
end

function onhit( p, po, armour, shield )
   if not mem.active and armour > 0 then
      -- Don't run while cooling off
      if mem.timer > 0 then return end
      mem.timer = ontime
      mem.active = true
      po:state( "on" )
   end
end
