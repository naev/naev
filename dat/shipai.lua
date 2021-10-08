--[[
   Handles the Ship AI (tutorial-ish?) being triggered from the ifo menu
--]]
local fmt = require "format"
local tut = require "common.tutorial"
local vn  = require "vn"

function create ()
   vn.clear()
   vn.scene()
   local sai = vn.newCharacter( tut.vn_shipai() )
   vn.transition( tut.shipai.transition )
   vn.label("mainmenu")
   sai(fmt.f(_([["Hello {playername}! Is there anything you want to change or brushen up on?"]]),{playername=player.name()}))
   vn.menu( function ()
      local opts = {
         {_("Tutorial Options"), "opts"},
         {_("Close"), "close"},
      }
      return opts
   end )

   vn.label("opts")
   vn.menu( function ()
      local opts = {
         {_("Reset all tutorials"), "reset"},
         {_("Close"), "mainmenu"},
      }
      if tut.isDisabled() then
         table.insert( opts, 1, {_("Enable tutorials"), "enable"} )
      else
         table.insert( opts, 1, {_("Disable tutorials"), "disable"} )
      end
      return opts
   end )

   vn.label("enable")
   vn.func( function () var.pop("tut_disable") end )
   sai(_([["In-game tutorials are now enabled! If you want to revisit old tutorials, make sure to reset them."]]))
   vn.jump("mainmenu")

   vn.label("disable")
   vn.func( function () var.push("tut_disable", true) end )
   sai(_([["In-game tutorials are now disabled."]]))
   vn.jump("mainmenu")

   vn.label("reset")
   sai(_([["This will reset and enable all in-game tutorials, are you sure you want to do this?"]]))
   vn.menu{
      {_("Reset tutorials"), "resetyes"},
      {_("Nevermind"), "opts"},
   }
   vn.label("resetyes")
   vn.func( function () tut.reset() end )
   sai(_([["All in-game tutorials have been set and enabled! They will naturally pop up as you do different things in the game."]]))
   vn.jump("mainmenu")

   vn.label("close")
   vn.done( tut.shipai.transition )
   vn.run()
end

