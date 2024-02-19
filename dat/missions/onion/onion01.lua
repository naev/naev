--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Onion Society 01">
 <unique />
 <priority>0</priority>
 <chance>10</chance>
 <location>Computer</location>
 <cond>
   return require("misn_test").cargo()
 </cond>
 <chapter>1</chapter>
 <notes>
  <campaign>Onion Society</campaign>
 </notes>
</mission>
--]]
--[[
   Stub mission to trigger onion01
--]]
local fmt = require "format"
local vntk = require "vntk"
local strmess = require "strmess"

local destpnt, destsys = spob.getS("Gordon's Exchange")

-- Create the mission
function create()
   if not var.peek("testing") then return false end

   -- This will mess up strings quite badly
   local messup = function ( str )
      return strmess.messup( str, 0.2 )
   end

   local null = "#r".._("NULL").."#0"

   misn.markerAdd( system.get("Sol"), "computer" )
   local title = fmt.f(_("Shipment to {pnt} in {sys} ({tonnes}) #oWARNING: Invalid Formatting Detected#0"),
         {pnt=null, sys=null, tonnes=_("∞ t")} )
   misn.setTitle( messup( title ) )

   local desc = fmt.f( _("Small shipment of {amount} of {cargo} to {pnt} in the {sys} system."),
         {cargo=null, amount=_("Sol system"), pnt="#y".._("INVALID RECORD").."#0", sys=null} )
   desc = desc.."\n\n"..fmt.f(_("#nCargo:#0 {amount}"),{amount=null})
   desc = desc.."\n#r".._("ERROR: NoncurrentModificationExceptionException: Exception modified while excepting.").."#0"

   misn.setDesc( messup(desc) )
   misn.setReward(_("-∞ ¤"))
end

local accepted_tries = 0
function accept ()
   accepted_tries = accepted_tries + 1
   if accepted_tries == 1 then
      vntk.msg(_([[You try to accept the mission, but a bunch of errors pop out, and the mission computer ends up crashing and rebooting. That was weird.]]))
      local title = _("Shipment to Shipment to Shipment to Shipment to")
      misn.setTitle( strmess.messup( title, 0.1 ) )
      local desc = fmt.f(_([[ERROR: BufferOverrua80ho0ajqnc
hq;8eoa 8q0 h
08qj h
2

5 250arcqj0a8eSmall shipment of of of ofof of
to {pnt} in the {sys} system]]),
         {pnt=destpnt, sys=destsys} )
      desc = desc.."\n\n"..fmt.f(_("#nCargo:#0 {amount}"),{amount=_("Small Box")})
      misn.setDesc( strmess.messup( desc, 0.1 ) )
      misn.computerRefresh()
   end
end
