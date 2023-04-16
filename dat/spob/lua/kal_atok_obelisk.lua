local vn = require 'vn'
local fmt = require "format"
local ccomm = require "common.comm"

function init( spb )
   mem.spob = spb
   mem.target = system.get("Kal Atok")
   mem.criteria = function ()
      return true, ""
   end
end

function can_land ()
   -- No psychic powers
   if not var.peek("sirius_psychic") then
      return false, _("The obelisk seems to be inert.")
   end

   return false, _("It seems like you may be able to establish psychic communication with the obelisk.")
end

function comm ()
   -- No psychic powers
   if not var.peek("sirius_psychic") then
      player.msg(_("The obelisk seems to be inert."), true)
      return true
   end
   local activated = false
   local canactivate, requirement = mem.criteria()

   vn.clear()
   vn.scene()
   --local spb = ccomm.newCharacterSpob( vn, mem.spob )
   ccomm.newCharacterSpob( vn, mem.spob )
   vn.transition()
   vn.na(fmt.f(_("You tune your psychic energy to the {spb}."),
      {spb=mem.spob}))

   vn.label("menu")
   vn.menu( function ()
      local opts = {
         { _("Close"), "leave" }
      }
      if canactivate then
         table.insert( opts, 1, {_("Activate"),"activate"} )
      else
         table.insert( opts, 1, "#r"..{_("Activate"),"cant_activate"}.."#0" )
      end
      return opts
   end )

   vn.label("cant_activate")
   vn.na(_("You muster your psychic powers to attempt to activate he obelisk, however, it seems like you do not have the powers to be able to activate this obelisk.").."\n"..requirement)
   vn.jump("menu")

   vn.label("activate")
   vn.na(_("You activate the obelisk."))
   vn.func( function () activated = true end )
   vn.done()

   vn.label("leave")
   vn.run()

   -- Player activated the obelisk
   if activated then
      var.push( "obelisk_target", mem.target:nameRaw() )
      naev.eventStart("Obelisk")
   end
   return true
end
