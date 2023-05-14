local poi = require "common.poi"
local der = require 'common.derelict'
local vn = require 'vn'
local fmt = require 'format'

return {
   name = "Point of Interest",
   repeatable = true,
   nolimit = true,
   cond = function ()
      return not player.misnActive( "Point of Interest - Intro" ) and (player.misnActive("Point of Interest") or 0) < 3
   end,
   func = function ()
      local poiintro = "Point of Interest - Intro"

      -- Tries to do the intro the first time
      if not player.misnDone( poiintro ) then
         if not naev.missionStart( poiintro ) then
            -- Failed to start
            return false
         end
         return true
      end

      local poidata = poi.generate()
      if not poidata then -- Failed to generate
         return false
      end

      local accept = false
      vn.clear()
      vn.scene()
      vn.sfx( der.sfx.board )
      vn.music( der.sfx.ambient )
      vn.transition()
      if poidata.sys:known() then
         vn.na(fmt.f(_([[While the derelict itself has been picked clean. You manage to find some interesting data remaining in the navigation log. It looks like you may be able to follow the lead to something of interest in the {sys} system. Do you wish to download the data?]]),
            {sys="#b"..poidata.sys:name().."#0"}))
      else
         vn.na(_([[While the derelict itself has been picked clean. You manage to find some interesting data remaining in the navigation log. It looks like you may be able to follow the lead to something of interest in what appears to be a nearby system. Do you wish to download the data?]]))
      end

      vn.menu{
         {_("Download the data"), "accept"},
         {_("Leave."), "leave"},
      }

      vn.label("accept")
      vn.func( function () accept = true end )
      vn.na(_([[You download the data and mark the target system on your navigation console. With nothing else to do on the derelict, you leave it behind, and return to your ship.]]))
      vn.jump("done")

      vn.label("leave")
      vn.na(_([[You leave the information alone and leave the derelict.]]))

      vn.label("done")
      vn.sfx( der.sfx.unboard )
      vn.run()
      player.unboard()

      if accept then
         poi.setup( poidata )
         naev.missionStart("Point of Interest")
         der.addMiscLog(fmt.f(_([[You found information on a point of interest aboard a derelict in the {sys} system.]]),{sys=system.cur()}))
      else
         der.addMiscLog(_([[You found information about a point of interest aboard a derelict, but decided not to download it.]]))
      end

      return true -- Success
   end,
}
