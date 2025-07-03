--[[
   Active hypergate
--]]
local fmt = require "format"
local lg = require "love.graphics"
local lf = require "love.filesystem"
local audio = require "love.audio"
local love_shaders = require "love_shaders"
local luatk = require "luatk"
local luatk_map = require "luatk.map"
local prng = require "prng"
local luaspfx = require "luaspfx"

local pixelcode = lf.read( "spob/lua/glsl/hypergate.frag" )
local jumpsfx = audio.newSource( 'snd/sounds/hypergate.ogg' )

local hypergate = {}

function hypergate.destinations_default ()
   local csys = system.cur()
   local destinations = {}
   for i,s in ipairs(system.getAll()) do
      if s ~= csys then
         for j,sp in ipairs(s:spobs()) do
            local t = sp:tags()
            if t.hypergate and t.active then
               table.insert( destinations, sp )
            end
         end
      end
   end
   return destinations
end

local function update_canvas ()
   local oldcanvas = lg.getCanvas()
   lg.setCanvas( mem.cvs )
   lg.clear( 0, 0, 0, 0 )
   lg.setColour( 1, 1, 1, 1 )

   -- Draw base hypergate
   mem.tex:draw( 0, 0 )

   -- Draw active overlay shader
   local oldshader = lg.getShader()
   lg.setShader( mem.shader )
   mem.mask:draw( 0, 0 )
   lg.setShader( oldshader )

   lg.setCanvas( oldcanvas )
end

function hypergate.setup( params )
   -- Hook functions to globals as necessary
   init     = hypergate.init
   load     = hypergate.load
   unload   = hypergate.unload
   update   = hypergate.update
   render   = hypergate.render
   can_land = hypergate.can_land
   land     = hypergate.land

   -- Set up parameters
   params = params or {}
   mem.destfunc   = params.destfunc or hypergate.destinations_default
   mem.basecol    = params.basecol or { 0.2, 0.8, 0.8 }
   mem.cost_flat  = params.cost_flat or 10e3
   mem.cost_mass  = params.cost_mass or 50
   mem.cost_mod   = params.cost_mod or 1
   mem.tex_path   = params.tex or "hypergate_neutral_activated.webp"
   mem.tex_mask_path = params.tex_mask or "hypergate_mask.webp"
end

function hypergate.init( spb )
   mem.spob = spb
end

function hypergate.load()
   if mem.tex==nil then
      -- Handle some options
      mem._cost_mod = mem.cost_mod
      if type(mem.cost_mod)=="table" then
         mem._cost_mod = 1
         local standing = mem.spob:reputation()
         for k,v in ipairs(mem.cost_mod) do
            if standing >= k then
               mem._cost_mod = v
               break
            end
         end
      end

      -- Set up texture stuff
      local prefix = "gfx/spob/space/"
      mem.tex  = lg.newImage( prefix..mem.tex_path )
      mem.mask = lg.newImage( prefix..mem.tex_mask_path )

      -- Position stuff
      mem.pos = mem.spob:pos()
      mem.tw, mem.th = mem.tex:getDimensions()
      mem.pos = mem.pos + vec2.new( -mem.tw/2, mem.th/2 )

      -- The canvas
      mem.cvs  = lg.newCanvas( mem.tw, mem.th, {dpiscale=1} )

      -- Set up shader
      local fragcode = string.format( pixelcode, mem.basecol[1], mem.basecol[2], mem.basecol[3] )
      mem.shader = lg.newShader( fragcode, love_shaders.vertexcode )
      mem.shader._dt = -1000 * rnd.rnd()
      mem.shader.update = function( self, dt )
         self._dt = self._dt + dt
         self:send( "u_time", self._dt )
      end

      update_canvas()
   end

   return mem.cvs.t.tex, mem.tw/2
end

function hypergate.unload ()
   mem.shader= nil
   mem.tex   = nil
   mem.mask  = nil
   mem.cvs   = nil
   --sfx   = nil
end

function hypergate.render ()
   update_canvas() -- We want to do this here or it gets slow in autonav
   local z = camera.getZoom()
   local x, y = gfx.screencoords( mem.pos, true ):get()
   z = 1/z
   mem.cvs:draw( x, y, 0, z, z )
end

function hypergate.update( dt )
   mem.shader:update( dt )
end

function hypergate.can_land ()
   return true, _("The hypergate is active.")
end

function hypergate.land( _s, p )
   -- Avoid double landing
   if p:shipvarPeek( "hypergate" ) then return end

   if player.pilot() == p then
      -- TODO check if hostile to disallow

      local target = hypergate.window()
      if target then
         var.push( "hypergate_target", target:nameRaw() )
         naev.cache().hypergate_colour = mem.basecol
         naev.eventStart("Hypergate")
      end
   else
      p:shipvarPush( "hypergate", true )
      p:effectAdd("Hypergate Enter")
      luaspfx.sfx( p:pos(), p:vel(), jumpsfx )
   end
end

function hypergate.window ()
   local w = 900
   local h = 600
   local wdw = luatk.newWindow( nil, nil, w, h )
   luatk.newText( wdw, 0, 10, w, 20, fmt.f(_("Hypergate ({sysname})"), {sysname=mem.spob:system()}), nil, "center", lg.newFont(14) )

   -- Load shaders
   local path = "scripts/luatk/glsl/"
   local function load_shader( filename )
      local src = lf.read( path..filename )
      return lg.newShader( src )
   end
   local shd_jumpgoto = load_shader( "jumplanegoto.frag" )
   shd_jumpgoto:send( "parami", 1 )
   shd_jumpgoto.dt = 0
   local shd_selectsys = load_shader( "selectsys.frag" )
   shd_selectsys.dt = 0
   local csys = system.cur()
   local cpos = csys:pos()

   local destinations = mem.destfunc()

   -- Get potential destinations from tags
   table.sort( destinations, function( a, b ) return a:nameRaw() < b:nameRaw() end )
   local destnames = {}
   for i,d in ipairs(destinations) do
      local ppf = player.pilot():faction()
      if d:known() then
         local s = d:system()
         local f = s:faction()
         local str = fmt.f(_("{sysname} ({factname})"), {sysname=s,factname=f})
         if f:areEnemies(ppf) then
            str = "#H"..str.."#0"
         elseif f:areAllies(ppf) then
            str = "#F"..str.."#0"
         end
         table.insert( destnames, str )
      else
         local rng = prng.new( csys:nameRaw()..d:nameRaw() )
         local hash = string.format( "%05X", rng:random(0, math.pow(16,5) ) )
         table.insert( destnames, fmt.f(_("Unknown Signature {hash}"),{hash=hash}) )
      end
   end

   local inv = vec2.new(1,-1)
   local targetknown = false
   local mapw, maph = w-330, h-60
   local jumpx, jumpy, jumpl, jumpa = 0, 0, 0, 0
   local targetx, targety = 0, 0
   local map = luatk_map.newMap( wdw, 20, 40, mapw, maph, {
      notinteractive = true,
      hidetarget = true,
      render = function ( m, bx, by )
         if not targetknown then
            lg.setColour( {0, 0, 0, 0.3} )
            lg.rectangle("fill", 0, 0, mapw, maph )
            -- Show big question mark or something
         else
            luatk.rerender() -- Animated, so we have to draw every frame
            local mx, my = m.pos:get()
            local s = m.scale
            local r = luatk_map.sys_radius * s
            local jumpw = math.max( 10, 2*r )
            lg.setColour( {0, 0.8, 1, 0.8} )

            local jx = bx + (jumpx-mx)*s + mapw*0.5
            local jy = by + (jumpy-my)*s + maph*0.5
            local shd = lg.getShader()
            lg.setShader( shd_jumpgoto )
            shd_jumpgoto:send( "paramf", r )
            shd_jumpgoto:send( "dimensions", {jumpl*s,jumpw} )
            love_shaders.img:draw( jx-jumpl*0.5*s, jy-jumpw*0.5, jumpa, jumpl*s, jumpw )

            lg.setColour( {1, 1, 1, 1} )
            shd_selectsys:send( "dimensions", {2*r, 2*r} )
            lg.setShader( shd_selectsys )
            love_shaders.img:draw( bx+(targetx-mx)*s + mapw*0.5 - 2*r, by+(targety-my)*s + maph*0.5 - 2*r, 0, 4*r, 4*r )
            lg.setShader( shd )
         end
      end,
   } )
   local function map_center( _sys, idx, hardset )
      local s = destinations[ idx ]:system()
      targetknown = s:known()
      if targetknown then
         local p = (cpos + s:pos())*0.5
         jumpx, jumpy = (p*inv):get()
         targetx, targety = (s:pos()*inv):get()
         jumpl, jumpa = ((s:pos()-cpos)*inv):polar()
         map:center( p, hardset )
      else
         jumpx, jumpy = 0, 0
         jumpl, jumpa = 0, 0
         map:center( cpos, hardset )
      end
   end
   map:setScale( 1/3 )
   map_center( nil, 1, true ) -- Center on first item in the list

   local pp = player.pilot()
   local ppf = pp:faction()
   local totalmass = pp:mass()
   for k,v in ipairs(pp:followers()) do
      totalmass = totalmass + v:mass()
   end
   local cost_mod = mem._cost_mod
   local totalcost = (mem.cost_flat + mem.cost_mass * totalmass) * cost_mod
   local hgsys = mem.spob:system()
   local hgfact = mem.spob:faction()
   local standing_value = mem.spob:reputation()
   local standing = hgfact:reputationText( standing_value )
   local cost_mod_str = tostring(cost_mod*100)
   if cost_mod < 1 then
      cost_mod_str = "#g"..cost_mod_str
   elseif cost_mod > 1 then
      cost_mod_str = "#r"..cost_mod_str
   end
   local standing_col = "#N"
   if hgfact:areAllies(ppf,hgsys) then
      standing_col = "#F"
   elseif hgfact:areEnemies(ppf,hgsys) then
      standing_col = "#H"
   end
   local txt = luatk.newText( wdw, w-260-20, 40, 260, 200, fmt.f(_(
[[#nCurrent System:#0 {cursys}
#nHypergate Faction:#0 {fact}
#nFaction Standing:#0 {standing} ({standing_value})
#nStanding Modifier:#0 {costmod}%#0
#nFleet Mass:#0 {totalmass}
#nUsage Cost:#0 {totalcost} ({flatcost} + {masscost} per tonne)

#nAvailable Jump Targets:#0]]), {
      cursys = csys,
      fact = hgfact,
      standing = standing_col..standing.."#0",
      standing_value = standing_col..tostring(math.floor(standing_value+0.5)).."#0",
      costmod = cost_mod_str,
      totalmass = fmt.tonnes(totalmass),
      totalcost = fmt.credits(totalcost),
      flatcost = fmt.credits(mem.cost_flat),
      masscost = fmt.credits(mem.cost_mass),
   }) )
   local txth = txt:height()
   local lst = luatk.newList( wdw, w-260-20, 40+txth+10, 260, h-40-20-40-20-txth-10, destnames, map_center )

   local target_gate
   local function btn_jump ()
      local _sel, idx = lst:get()
      local d = destinations[ idx ]
      local s = d:system()
      local extramsg = ""
      if d:known() and s:faction():areEnemies(ppf) then
         extramsg = "#r"..fmt.f(_("\nYou are enemies with the {fctname}!"),{fctname=s:faction()}).."#0"
      end
      if not d:known() then
         extramsg = "#r".._("\nThe destination is unknown!").."#0"
      end

      local sysname
      if s:known() then
         sysname = s:name()
      else
         sysname = _("Unknown")
      end

      luatk.yesno( fmt.f(_("Jump to {sysname}?"),{sysname=sysname}),
         fmt.f(_("Are you sure you want to jump to {sysname} for {credits}?{extramsg}"),{sysname=sysname,credits=fmt.credits(totalcost),extramsg=extramsg}), function ()
            if player.credits() > totalcost then
               player.pay(-totalcost)
               target_gate = d
               luatk.close()
            else
               luatk.msg(_("Insufficient Credits"),fmt.f(_("You have insufficient credits to use the hypergate. You are missing #r{difference}#0."),{difference=fmt.credits(totalcost-player.credits())}))
            end
         end, nil )
   end
   luatk.newButton( wdw, w-(120+20)*2, h-40-20, 120, 40, _("Jump!"), btn_jump )
   luatk.newButton( wdw, w-120-20, h-40-20, 120, 40, _("Close"), luatk.close )

   wdw:setUpdate( function ( dt )
      shd_jumpgoto.dt = shd_jumpgoto.dt + dt
      shd_jumpgoto:send( "dt", shd_jumpgoto.dt )
      shd_selectsys.dt = shd_selectsys.dt + dt
      shd_selectsys:send( "dt", shd_selectsys.dt )
   end )
   wdw:setAccept( btn_jump )
   wdw:setCancel( luatk.close )
   wdw:setKeypress( function ( key )
      if key=="down" then
         local _sel, idx = lst:get()
         lst:set( idx+1 )
         return true
      elseif key=="up"then
         local _sel, idx = lst:get()
         lst:set( idx-1 )
         return true
      end
      return false
   end )

   luatk.run()

   return target_gate
end

return hypergate
