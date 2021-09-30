local factions
function onload ()
   factions = outfit.get("Fake Transponder"):illegality()
end

local function fget( f )
   return var.peek("faket_"..f:nameRaw())
end
local function fset( f, v )
   return var.push("faket_"..f:nameRaw(), v)
end
local function fclear( f )
   var.pop("faket_"..f:nameRaw())
end

local function reset( p, po )
   for k,f in ipairs(factions) do
      local os = fget( f )
      local s  = f:playerStanding()
      -- See how to modify saved value
      if not os then
         -- If not set, just set
         fset( f, s )
      else
         -- Otherwise, we use negative hits until the player drops to 0 (in case of positive)
         if s < 0 and os > 0 then
            fset( f, math.min( os+s, 0 ) )
         end
      end
      -- Reset current standing
      f:setPlayerStanding( f:playerStandingDefault() )
   end
   po:state("on")
end

local function disable( p, po )
   for k,f in ipairs(factions) do
      f:setPlayerStanding( fget( f ) )
      fclear( f )
   end
   po:state("off")
end

function onadd( p, po )
   reset( p, po )
end

function onremove( p, po )
   disable( p, po )
end

function init( p, po )
   reset( p, po )
end

function onscanned( p, po, scanner )
   disable( p, po )
end
