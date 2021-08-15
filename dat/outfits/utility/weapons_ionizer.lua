threshold = 50

function init( p, po )
   mem.t = nil
   mem.forced_on = false
   mem.forced_off = false
   po:state( "off" )
end

function update( p, po, dt )
   -- Ignore if forced
   if mem.forced_on then return end

   local t = p:target()
   -- Target changed
   if t ~= mem.t then
      po:state( "off" )
      mem.t = t
      forced_off = false
      return
   end
   if mem.forced_off then return end
   if t == nil or t:health() > threshold then
      po:state( "off" )
      return
   end
   po:state( "on" )
end

function ontoggle( p, po, on )
   if on then
      po:state( "on" )
      mem.forced_on = true
   else
      po:state( "off" )
      if not mem.forced_on then
         mem.forced_off = true
      else
         mem.forced_off = true
      end
      mem.forced_on = false
   end
end
