local love = require "love"
local intro = {}

function intro.init( _params )
   intro.data = {}
end

function intro.image( img )
   table.insert( intro.data, {
      type = "image",
      data = img,
   } )
end

function intro.text( txt )
   table.insert( intro.data, {
      type = "text",
      data = txt,
   } )
end

function intro.fadeout()
   table.insert( intro.data, {
      type = "fadeout",
   } )
end

function intro.run()
   local c = naev.cache()
   c._intro = intro.data
   love.exec( "scripts/intro" )
   c._intro = nil
end

return intro
