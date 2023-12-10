local helper = {}

function helper.msgnospam( msg )
   local lasttick = mem.lasttick or 0
   local curtick = naev.ticks()
   if (mem.lastmsg~=msg) or (curtick-lasttick > 1) then
      player.msg( msg )
      mem.lastmsg = msg
      mem.lasttick = curtick
   end
end

return helper
