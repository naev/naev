function add( p )
   p:setInvincible(true)
   p:setInvisible(true)
end

function remove( p )
   if player.pilot()~=p then -- and p:exists() then
      p:rm()
   else
      p:setInvincible(false)
      p:setInvisible(false)
   end
end
