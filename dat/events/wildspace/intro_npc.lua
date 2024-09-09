--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Way to Wild Space">
 <location>land</location>
 <unique />
 <chance>10</chance>
 <chapter>^0</chapter><!-- TODO bump to 2 when finished. -->
 <cond>
   local s = spob.cur()
   if not s:services()["bar"] then
      return false
   end
   if s:tags().restricted then
      return false
   end
   local sf = s:faction()
   local sft = sf:tags()
   if not (sft.generic or sft.pirate) then
      return false
   end

   local j = jump.get( "Jommel", "Syndania" )
   return not j:known()
 </cond>
</event>
--]]
local vn = require "vn"
local vni = require "vnimage"
local fmt = require "format"

local img, prt
local npcname = _("Suspicious Patron")
local price = 800e3
local jmp = jump.get( "Jommel", "Syndania" )

function create ()
   evt.finish(false)
   img, prt = vni.pirate()
   evt.npcAdd( "approach", npcname, prt, _("You get the impression that the suspicious patron is beckoning you to come to their table.") )
   hook.enter("done")
end

function approach ()
   local accepted = false

   vn.clear()
   vn.scene()
   local pir = vn.newCharacter( _(""), { image=img } )
   vn.transition()

   vn.na(_([[You approach the suspicious patron. They seem to reek of some sort of strong incense or herb.]]))
   -- deets is slang for details (aka intel)
   pir(_([["Yo, soul. Ya luk like ya could use some 'dventure, know whaddai mean? Ai've got mai hands on some iiinteresting data. You in fo' sum deets on a path to them so-called #bWild Space#0, soul?"]]))
   vn.menu{
      {_([["Wild Space?"]]), "cont01"},
      {_([["Not interested."]]), "decline"},
   }

   vn.label("decline")
   vn.na(_([[You decline. It is probably a scam anyway.]]))
   vn.done()

   vn.label("cont01")
   pir(_([[They lean forward and talk in a hushed voice.
"Ya see, soul, it's them lost systems. Eaten by thee Neb'la. Full of all them rich's and stuff left 'hind. One 'tch though, soul, many souls go in, none come out! Like one of dem one-way doors."]]))
   pir(fmt.f(_([["You be a strong soul, ya can pull out where other souls fail'd. I see et in ya eyes, soul. Wat ya say, {cost} and the deets are yurs, soul."]]),
      {cost=fmt.credits(price)}))
   vn.menu{
      {_([["Hook me up, soul."]]), "cont02_buy"},
      {_([["Too much."]]), "cont02_decline"},
   }

   vn.label("cont02_decline")
   -- TODO let the player get a discount if they have drugs or something

   vn.label("cont02_buy")

   vn.func( function ()
      if player.credits() >= price then
         player.pay( -price )
         accepted = true
         return
      else
         vn.jump("broke")
      end
   end )

   vn.label("broke")
   vn.done()

   vn.run()

   if not accepted then return false end

   jmp:setKnown(true)
end

function done ()
   evt.finish(false)
end
