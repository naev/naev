local cmark = require "cmark"
local lf = require "love.filesystem"
local luatk = require 'luatk'
local md = require "luatk.markdown"
local fmt = require "format"

local naevpedia = {}

local function loaddoc( filename )
   local dat = lf.read( filename )
   if not dat then
      warn(fmt.f(_("File '{filename}' not found!"),{filename=filename}))
      return nil
   end
   return cmark.parse_string( dat, cmark.OPT_DEFAULT )
end

local naevpediadata = {
   index = loaddoc( 'index.md' ),
   mechanics = loaddoc( 'mechanics.md' ),
}

function naevpedia.open( name )
   name = name or "index"

   local history = {}
   local current = "index"

   local open_page
   local w, h = 640, 480
   local wdw = luatk.newWindow( nil, nil, w, h )
   luatk.newButton( wdw, -20, -20, 80, 30, _("Close"), luatk.close )
   local btnback = luatk.newButton( wdw, -20-80-20, -20, 80, 30, _("Back"), function ( self )
      local n = #history
      current = history[n]
      history[n] = nil
      open_page( naevpediadata[current] )
      if #history <= 0 then
         self:disable()
      end
   end )
   btnback:disable()
   local mrk
   function open_page( doc )
      if mrk then
         mrk:destroy()
      end
      mrk = md.newMarkdown( wdw, doc, 20, 20, w-40, h-60, {
         linkfunc = function ( target )
            local newdoc = naevpediadata[ target ]
            if not newdoc then
               -- do warning
               luatk.msg( _("404"), fmt.f(_("Unable to find link to '{target}'!"),{target=target}))
               return
            end
            table.insert( history, current )
            btnback:enable()
            current = target
            mrk:destroy()
            open_page( newdoc )
         end,
      } )
   end
   open_page( naevpediadata[name] )
   luatk.run()
end

return naevpedia
