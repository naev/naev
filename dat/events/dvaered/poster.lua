--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Dvaered Poster">
 <trigger>land</trigger>
 <cond>planet.cur():faction() == faction.get("Dvaered")</cond>
 <chance>50</chance>
</event>
--]]

--[[
-- Dvaered advertising posters.
--]]

local vn = require 'vn'
local graphics = require 'love.graphics'

-- List of avaliable posters and index of the corresponding message
imagesNmsg = { {"gfx/vn/posters/dvaered/dv_athena.png", 1},
               {"gfx/vn/posters/dvaered/dv_brunhilde.png", 1},
               {"gfx/vn/posters/dvaered/dv_goliath.png", 1},
               {"gfx/vn/posters/dvaered/dv_good_manners_ad.png", 2},
               {"gfx/vn/posters/dvaered/dv_minotaur.png", 1},
               {"gfx/vn/posters/dvaered/dv_oeudipe.png", 1} }

-- List of FLF flyers
FLFFlyers = { "gfx/vn/posters/flf/flf_spam_small.png",
              "gfx/vn/posters/flf/flf_spam2.png" }

messages = {}
messages[1] = _([[This is an advertisement for the Dvaered Army.]])
messages[2] = _("Does this poster make you want to buy this book?")

message_flf = _("The FLF has wrecked this Dvaered poster.")

              
function create()
   -- Create the NPC.
   evt.npcAdd("watchPoster", _("Poster on the wall"), "news.webp", _("There is a poster on the wall"), 15)

   local imageNmsg = imagesNmsg[ rnd.rnd(1,#imagesNmsg) ]
   myImg = imageNmsg[1]

   -- Did the FLF wreck this poster?
   flf = ( system.cur():presences()["FLF"] and rnd.rnd()<.3 )
   if flf then
      myMsg = message_flf
   else
      myMsg = messages[ imageNmsg[2] ]
   end

   -- End event on takeoff.
   hook.takeoff( "leave" )
end

function watchPoster()
   local canvas

   if flf then
      -- Create the image
      local img  = graphics.newImage( myImg )
      canvas = graphics.newCanvas(700, 900*1.5) -- It's all assuming the poster is 700x900
      graphics.setCanvas(canvas)
      graphics.draw( img, 0, 0, 0, 1, 1 )

      -- Add FLF stuff on it
      for i = 1, 20 do
         local spam = graphics.newImage( FLFFlyers[ rnd.rnd(1,#FLFFlyers) ] )
         graphics.draw( spam, rnd.rnd(-200,700), rnd.rnd(-200,900), rnd.rnd(-1,1), 1, 1 )
      end
      graphics.setCanvas()
   else -- Raw poster
      local img  = graphics.newImage( myImg )
      canvas = graphics.newCanvas(700, 900*1.5) -- It's all assuming the poster is 700x900
      graphics.setCanvas(canvas)
      graphics.draw( img, 0, 0, 0, 1, 1 )
      graphics.setCanvas()
   end

   -- Create the vn scene
   vn.clear()
   vn.scene()
   local npc = vn.newCharacter( _("Poster"), { image=canvas } )
   vn.transition()
   npc( myMsg )
   vn.run()
end

function leave()
   evt.finish()
end
