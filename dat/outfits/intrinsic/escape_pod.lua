function init ()
   mem.launched = false
end

function ondeath( p )
   if not mem.launched then
      -- TODO for some reason it doesn't like setting the ai first, gives an
      -- error that we can't really trace atm
      --local ep = pilot.add("Escape Pod", p:faction(), p:pos(), nil, {ai="escape_pod"} )
      local ep = pilot.add("Escape Pod", p:faction(), p:pos(), nil, {naked=true} )
      ep:changeAI("escape_pod")
      ep:setInvincible(true)
      ep:setInvisible(true)
      ep:setVel( p:vel() )
      if p:ship():size() < 3 then
         ep:setDir( p:dir() )
      else
         ep:setDir( rnd.angle() )
      end
      mem.launched = true
   end
end
