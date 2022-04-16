function add( p )
   p:control()
   p:setInvincible(true)
   p:setInvisible(true)
end

function remove( p )
   if p ~= player.pilot() then
      p:rm() -- Remove the pilot
   else
      p:control(false)
      p:setInvincible(false)
      p:setInvisible(false)
      p:shipvarPop( "wormhole" )
   end
end
