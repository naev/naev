local duration = 10

function onstealth( _p, po, stealthed )
   if stealthed then
      po:state( "on" )
      mem.timer = duration
   end
end

function update( p, po, dt )
   if mem.timer then
      mem.timer = mem.timer - dt
      if mem.timer <= 0 then
         po:state( "off" )
      end
   end
   onstealth( p, po, p:flags("stealth") )
end
