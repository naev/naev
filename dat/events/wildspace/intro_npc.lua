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
   local st = s:tags()
   if st.restricted then
      return false
   end
   local sf = s:faction()
   local sft = sf:tags()
   if not (sft.generic or sft.pirate) then
      return false
   end

   -- Less chance on non-criminal systems
   if rnd.rnd() &lt; 0.8 and not st.criminal then
      return false
   end

   -- Must not know the jump already
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
local jmpsys = system.get("Jommel")
local jmp = jump.get( jmpsys, "Syndania" )

function create ()
   img, prt = vni.pirate()
   evt.npcAdd( "approach", npcname, prt, _("You get the impression that the suspicious patron is beckoning you to come to their table.") )
   hook.enter("done")
end

local approached = false
function approach ()
   local accepted = false

   vn.clear()
   vn.scene()
   local pir = vn.newCharacter( _(""), { image=img } )
   vn.transition()

   vn.na(_([[You approach the suspicious patron. They seem to reek of some sort of strong incense or herb.]]))
   if approached then
      pir(fmt.f(_([["Yo, soul. Ya changed ya mind about them #bWild Space#0 deets for {cost}?"]]),
         {cost=fmt.credits(price)}))
   else
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
      vn.func( function () approached = true end )
   end
   vn.menu{
      {_([["Hook me up, soul."]]), "cont02_buy"},
      {_([["Too much."]]), "cont02_decline"},
   }

   vn.label("cont02_decline")
   -- TODO let the player get a discount if they have drugs or something
   pir(_([["If ya change ya mind, I'll be a chillin' 'ere, soul."]]))
   vn.done()

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
   pir(_([["Ya comin' back alive, eh, soul?"]]))
   vn.sfxEerie()
   if jmpsys:known() then
      vn.na(fmt.f(_([[You have obtained information about a jump from {sys} to #bWild Space#0.]]),
         {sys=jmpsys}))
   else
      vn.na(fmt.f(_([[You have lost {cost}.
You have obtained information about a jump to #bWild Space#0.]]),
      {cost=fmt.credits(price)}))
   end
   vn.done()

   vn.label("broke")
   vn.na(_([[You can not afford the information about #bWild Space#0.]]))
   vn.done()

   vn.run()

   if not accepted then return false end

   jmp:setKnown(true)
   evt.finish(true)
end

function done ()
   evt.finish(false)
end
