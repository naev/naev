update_dt = 1

function init ()
   mem.dt = 0
end

function update( p )
   mem.dt = mem.dt + update_dt
   if mem.dt > 9 then
      p:effectAdd("Fade-Out")
      mem.dt = -math.huge
   end
end
