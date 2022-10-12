function add( p )
   p:setInvincible(true)
   p:setInvisible(true)
end

function remove( p )
   if player.pilot()~=p then
      p:rm()
   end
   p:setInvincible(false)
   p:setInvisible(false)
end
