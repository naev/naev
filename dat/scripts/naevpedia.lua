local cmark = require "cmark"
local lf = require "love.filesystem"
local luatk = require 'luatk'
local md = require "luatk.markdown"
local fmt = require "format"

local naevpedia = {}

local nc = naev.cache()
function naevpedia.load()
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
-- See if we have to load the naevpedia, since we want to cache it for speed
if not nc._naevpedia then
   naevpedia.load()
end

--[[--
Processes the Lua in the markdown file as nanoc does.

<%= print('foo') %> statements get printed in the text, while <% if foo then %> get processed otherwise.
--]]
local function dolua( s )
   local luastr = [[local out = ""
local pr = _G.print
local pro = function( str )
   out = out..str
end
]]
   local function embed_str( str )
      for i=1,20 do
         local sep = ""
         for j=1,i do
            sep = sep.."="
         end
         if (not string.find(str,"["..sep.."[",1,true)) and (not string.find(str,"]"..sep.."]",1,true)) then
            luastr = luastr.."out = out..["..sep.."["..str.."]"..sep.."]\n"
            break
         end
      end
   end

   local ms, me = string.find( s, "<%", 1, true )
   local be = 1
   while ms do
      local bs
      local display = false
      embed_str( string.sub( s, be+1, ms-1 ) )
      bs, be = string.find( s, "%>", me, true )
      if string.sub( s, me+1, me+1 )=="=" then
         me = me+1
         display = true
         luastr = luastr.."_G.print = pro\n"
      else
         luastr = luastr.."_G.print = pr\n"
      end
      local ss = string.sub( s, me+1, bs-1 )
      luastr = luastr..ss.."\n"
      if display then
         luastr = luastr.."out = out..'\\n'\n"
      end
      ms, me = string.find( s, "<%", me, true )
   end
   embed_str( string.sub( s, be+1 ) )
   luastr = luastr.."return out"
   local pr = _G.print
   local c = loadstring(luastr)
   --setfenv( c, { _G={}, print=pr } )
   local success,result_or_err = pcall( c )
   _G.print = pr
   if not success then
      warn( result_or_err )
      return "#r"..result_or_err.."#0"
   end
   return success, result_or_err
end

--[[--
Parse document
--]]
local function loaddoc( filename )
   local rawdat = lf.read( 'naevpedia/'..filename..'.md' )
   if not rawdat then
      warn(fmt.f(_("File '{filename}' not found!"),{filename=filename}))
      return false, fmt.f("#r".._("404\nfile '{filename}' not found"), {filename=filename})
   end

   -- Preprocess Lua
   local success, dat = dolua( rawdat )
   if not success then
      return success, dat
   end
   return success, cmark.parse_string( dat, cmark.OPT_DEFAULT )
end

function naevpedia.open( name )
   name = name or "index"

   local history = {}
   local current = "index"

   local open_page
   local w, h = 640, 480
   local wdw = luatk.newWindow( nil, nil, w, h )
   luatk.newText( wdw, 0, 10, w, 20, _("Naevpedia"), nil, "center" )
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
      local success, doc = loaddoc( filename )

      -- Create widget
      if not success then
         -- Failed, so just display the error
         mrk = luatk.newText( wdw, 20, 40, w-40, h-110, doc )
      else
         -- Success so we try to load the markdown
         mrk = md.newMarkdown( wdw, doc, 20, 40, w-40, h-110, {
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
   end
   open_page( name )
   luatk.run()
end

return naevpedia
