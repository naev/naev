function add( p )
   p:setInvisible(true)
end

function remove( p )
   if player.pilot()~=p then -- and p:exists() then
      p:rm()
   else
      p:setInvisible(false)
   end
end
