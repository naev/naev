local cmark = require "cmark"
local lf = require "love.filesystem"
local luatk = require 'luatk'
local md = require "luatk.markdown"
local fmt = require "format"

local naevpedia = {}

-- See if we have to load the naevpedia
local nc = naev.cache()
if true then --if not nc._naevpedia then
   local mds = {}
   local function find_md( dir )
      for k,v in ipairs(lf.getDirectoryItems('naevpedia/'..dir)) do
         local f = v
         if dir ~= "" then
            f = dir.."/"..f
         end
         local i = lf.getInfo( 'naevpedia/'..f )
         if i then
            if i.type == "file" then
               local suffix = string.sub(f, -3)
               if suffix=='.md' then
                  mds[ string.sub( f, 1, -4 ) ] = true
               end
            elseif i.type == "directory" then
               find_md( f )
            end
         end
      end
   end
   find_md( '' )
   nc._naevpedia = mds
end

-- Parse document
local function loaddoc( filename )
   local dat = lf.read( 'naevpedia/'..filename..'.md' )
   if not dat then
      warn(fmt.f(_("File '{filename}' not found!"),{filename=filename}))
      return nil
   end
   return cmark.parse_string( dat, cmark.OPT_DEFAULT )
end

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
      open_page( current )
      if #history <= 0 then
         self:disable()
      end
   end )
   btnback:disable()
   local mrk
   function open_page( filename )
      if mrk then
         mrk:destroy()
      end

      -- Load the document
      local doc = loaddoc( filename )

      -- Create widget
      mrk = md.newMarkdown( wdw, doc, 20, 20, w-40, h-60, {
         linkfunc = function ( target )
            local newdoc = target
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

      -- Clean up the document
      cmark.node_free( doc )
   end
   open_page( name )
   luatk.run()
end

return naevpedia
