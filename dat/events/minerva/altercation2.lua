--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Minerva Station Altercation 2">
 <trigger>none</trigger>
 <chance>0</chance>
 <flags>
  <unique />
 </flags>
 <notes>
  <campaign>Minerva</campaign>
  <done_misn name="Maikki's Father 2">Random event</done_misn>
 </notes>
</event>
--]]

--[[
-- Bad people try to do bad things to cyborg chicken
--
-- Triggered from station.lua
--]]
local portrait = require "portrait"
local vn = require 'vn'
local window = require 'love.window'
local minerva = require "minerva"

thug1_name = _("Thug")
thug1_image = "scavenger1.png"

function create ()
   vn.clear()
   vn.scene()
   vn.fadein(1)
   local t1 = vn.newCharacter( thug1_name, { image=thug1_image } )
   local cc = vn.newCharacter( minerva.vn_cyborg_chicken() )
   t1.manualpos = true
   cc.manualpos = true
   local lw, lh = window.getDesktopDimensions()
   t1.offset = 0.25*lw
   cc.offset = 0.75*lw
   vn.na(_("You once again hear a large commotion in one of the wings of Minerva station. As you get closer you see what seems to be a group of people chasing something small... Wait, is that a chicken?"))
   local function runaround ( left, right )
      local lw, lh = window.getDesktopDimensions()
      left  = left or rnd.rnd( 0.25*lw, 0.4*lw )
      right = right or rnd.rnd( 0.6*lw, 0.75*lw )
      local function runinit ()
         local t1pos = t1.offset
         local ccpos = cc.offset
         local t1newpos, ccnewpos
         if t1pos < 0.5*lw then
            t1newpos = right
            ccnewpos = left
         else
            t1newpos = left
            ccnewpos = right
         end
         return { t1pos, ccpos, t1newpos, ccnewpos }
      end
      return vn.animation( 1, function (alpha, params)
         local t1pos, ccpos, t1newpos, ccnewpos = table.unpack(params)
         t1.offset = t1newpos*alpha + t1pos*(1-alpha)
         cc.offset = ccnewpos*alpha + ccpos*(1-alpha)
      end, nil, "ease-in-out", runinit )
   end
   t1(_([["Get back here you thief! Give me back my tokens! There is no way "]]))
   runaround()
   cc(_([["Puk Puk Pukaaak! Quack!"]]))
   runaround()
   runaround()
   t1(_([["Filthy little animal! I'll beat you into a pulp!"]]))
   runaround()
   runaround()
   cc(_([["Cluck Quack Clukuk Pukaaaaaaaaak!"]]))
   runaround()
   vn.na(_("Cyborg Chicken runs itself into a corner and it doesn't seem like station security will make it in time. What do you do?"))
   vn.menu( {
      {_("Try to intervene"),"intervene"},
      {_("Wait"),"wait"},
   } )

   vn.label("intervene")
   vn.na(_("You approach the chicken and thug and try to mediate."))
   cc(_("As the thug is distracted by you, Cyborg Chicken suddenly sprints and runs past you. Did it wink at you?"))
   vn.jump("chickenleft")

   vn.label("wait")
   t1(_([["Not such a tough chicken anymore, huh? Going to beat you so bad we'll have chicken nuggets for lunch!"]]))
   cc(_([[Cyborg Chicken blinks twice then sprints through the thugs legs and runs past you. The thug tries to chase but ends up bumping into you.]]))

   vn.label("chickenleft")
   local function chickenleft ()
      local lw, lh = window.getDesktopDimensions()
      local function runinit ()
         return cc.offset
      end
      vn.animation( 1.5, function (alpha, ccpos)
         local ccnewpos = 1.5 * lw
         cc.offset = ccnewpos*alpha + ccpos*(1-alpha)
      end, nil, "ease-out", runinit )
      vn.disappear( cc )
   end
   chickenleft()
   t1(_([["You let the chicken get away punk! How are you going to make up for this!"]]))
   vn.na(_("You buy your time and eventually the station security comes escorting the thug away. As you go back to what you were doing, you see cyborg chicken in the distance who seems to wink at you. What was that all about?"))
   vn.fadeout()
   vn.run()

   evt.finish(true)
end


