--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Dvaered Poster">
 <location>land</location>
 <cond>spob.cur():faction() == faction.get("Dvaered")</cond>
 <chance>50</chance>
</event>
--]]

--[[
-- Dvaered advertising posters.
--]]

local vn = require 'vn'
local graphics = require 'love.graphics'


-- List of avaliable posters and index of the corresponding message
local imagesNmsg = {
   {"gfx/vn/posters/dvaered/dv_athena.png", 1},
   {"gfx/vn/posters/dvaered/dv_brunhilde.png", 1},
   {"gfx/vn/posters/dvaered/dv_goliath.png", 1},
   {"gfx/vn/posters/dvaered/dv_good_manners_ad.png", 2},
   {"gfx/vn/posters/dvaered/dv_minotaur.png", 1},
   {"gfx/vn/posters/dvaered/dv_oeudipe.png", 1} }

-- List of FLF flyers
local FLFFlyers = {
   "gfx/vn/posters/flf/flf_spam_small.png",
   "gfx/vn/posters/flf/flf_spam2.png" }

local messages = {}
messages[1] = _([[This is an advertisement for the Dvaered Army.]])
messages[2] = _("Does this poster make you want to buy this book?")

local myImg, flf, myMsg, myCanvas

function create()
   local imageNmsg = imagesNmsg[ rnd.rnd(1,#imagesNmsg) ]
   myImg = imageNmsg[1]

   -- Did the FLF wreck this poster?
   flf = ( system.cur():presences()["FLF"] and rnd.rnd()<.3 )
   if flf then
      myMsg = _("The FLF has wrecked this Dvaered poster.")
   else
      myMsg = messages[ imageNmsg[2] ]
   end

   -- Create the NPC.
   evt.npcAdd("watchPoster", _("Poster"), myImg, _("There is a poster on the wall."), 15)

   -- End event on takeoff.
   hook.takeoff( "leave" )
end

function watchPoster()
   if not myCanvas then
      myCanvas = graphics.newCanvas(700, 900) -- It's all assuming the poster is 700x900
      local img    = graphics.newImage( myImg )

      -- Create the image
      local oldcanvas = graphics.getCanvas()
      graphics.setCanvas(myCanvas)
      graphics.clear( 0, 0, 0, 0 )
      graphics.setBlendMode( "alpha", "premultiplied" )
      graphics.draw( img, 0, 0, 0, 1, 1 )

      -- Add FLF stuff on it
      if flf then
         for i = 1, 20 do
            local spam = graphics.newImage( FLFFlyers[ rnd.rnd(1,#FLFFlyers) ] )
            graphics.draw( spam, rnd.rnd(-200,700), rnd.rnd(-200,900), rnd.rnd(-1,1), 1, 1 )
         end
      end

      -- Finish
      graphics.setBlendMode( "alpha" )
      graphics.setCanvas(oldcanvas)
   end

   -- Create the vn scene
   vn.clear()
   vn.scene()
   local npc = vn.newCharacter( _("Poster"), { image=myCanvas, isportrait=true } )
   vn.transition()
   npc( myMsg )
   vn.run()
end

function leave()
   evt.finish()
end
