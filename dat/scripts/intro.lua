local love = require "love"
local lf = require "love.filesystem"
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

local function lineiter( s )
   if s:sub(-1)~="\n" then s=s.."\n" end
   return s:gmatch("(.-)\n")
end

function intro.run( filename )
   -- Load file name if specified
   if filename then
      local data = lf.read( "AUTHORS" )
      if data==nil then
         return error(string.format("intro: unable to read '%s'", "AUTHORS"))
      end
      intro.init()
      for l in lineiter(data) do
         intro.text(_(l))
      end
      intro.run()
   end

   -- Set up and run Love
   local c = naev.cache()
   c._intro = intro.data
   love.exec( "scripts/intro" )
   c._intro = nil
end

return intro
