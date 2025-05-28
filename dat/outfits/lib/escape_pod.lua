local lib = {}

function lib.launch( p )
   -- TODO for some reason it doesn't like setting the ai first, gives an
   -- error that we can't really trace atm
   --local ep = pilot.add("Escape Pod", p:faction(), p:pos(), nil, {ai="escape_pod"} )
   local ep = pilot.add("Escape Pod", p:faction(), p:pos(), nil, {naked=true} )
   ep:changeAI("escape_pod")
   ep:setInvincible(true)
   ep:setInvisible(true)
   ep:setVel( p:vel() )
   ep:setDir( p:dir() + rnd.angle()*(p:ship():size()-1)/6 )
end

return lib
