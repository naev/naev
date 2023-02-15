--[[
<?xml version='1.0' encoding='utf8'?>
<event name="POI - Event">
 <location>enter</location>
 <chance>5</chance>
 <cond>
   -- Must have done intro mission
   if player.misnActive("Point of Interest - Intro") or not player.misnDone("Point of Interest - Intro") then
      return false
   end

   -- Must not have POI mission active
   if player.misnActive("Point of Interest") then
      return false
   end

   -- Must have scanner equipped
   local has_scanner = false
   for k,v in ipairs(player.pilot():outfitsList()) do
      if v:tags().poi_scan then
         has_scanner = true
         break
      end
   end
   return has_scanner
 </cond>
</event>
--]]
--[[
   Small event that allows the player to do a POI if they don't leave the system.
   Only triggers with a poi scanner equipped.
--]]
local poi = require "common.poi"

function create ()
   local scur = system.cur()
   if not poi.test_sys( scur ) then
      evt.finish(false)
      return
   end

   -- We do a soft claim on the final system
   if not evt.claim( {scur}, nil, true ) then
      evt.finish(false)
      return
   end

   mem.poi = {
      risk   = 2, -- not used atm but TODO something better
      locked = true,
      sys    = scur,
      found  = "found",
   }
   mem.sys = scur
   mem.locked = true
   mem.reward = { type="data" }

   hook.timer( 5+10*rnd.rnd(), "delay" )

   hook.land("cleanup")
   hook.jumpout("cleanup")
end

function delay ()
   player.msg(_("Your sensors detect a point of interest in the system!"),true)
   player.autonavReset( 1 )
   poi.hook_enter()
end

-- luacheck: globals found (passed to POI framework for hooks)
function found ()
   player.msg(_("You have found something!"),true)

   -- TODO something more interesting
   local shiptype = mem.reward.ship or "Mule"
   local p = pilot.add( shiptype, "Derelict", mem.goal, _("Pristine Derelict"), {naked=true} )
   p:disable()
   p:setInvincible()
   p:setHilight()
   p:effectAdd( "Fade-In" )
   hook.pilot( p, "board", "board" )
end

function board( p )
   local failed = poi.board( p )

   -- Clean up stuff
   poi.cleanup( failed )
   p:setHilight(false)
   player.unboard()
   evt.finish( not failed )
end

function cleanup ()
   evt.finish(false)
end
