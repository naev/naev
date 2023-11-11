local vn = require 'vn'
local fmt = require "format"
local ccomm = require "common.comm"
local lg = require "love.graphics"
local li = require "love.image"

local luaspob = {}

local f_empire = faction.get("Empire")
local f_dvaered = faction.get("Dvaered")
local f_zalek = faction.get("Za'lek")
local f_sirius = faction.get("Sirius")
local f_soromid = faction.get("Soromid")
local f_proteron = faction.get("Proteron")

function luaspob.setup( init_params )
   mem.params = init_params or {} -- Store parameters
   init     = luaspob.init
   load     = luaspob.load
   unload   = luaspob.unload
   can_land = luaspob.can_land
   comm     = luaspob.comm
   population = luaspob.population
   barbg    = luaspob.bg_generic
end

local bg_mapping = {
   ["0"] = luaspob.bg_station,
   ["1"] = luaspob.bg_station,
   ["2"] = luaspob.bg_station,
   ["3"] = luaspob.bg_station,
   ["4"] = luaspob.bg_station,
   ["A"] = luaspob.bg_lava,
   ["B"] = luaspob.bg_inert,
   ["C"] = luaspob.bg_inert,
   ["D"] = luaspob.bg_inert,
   ["E"] = luaspob.bg_generic,
   ["F"] = luaspob.bg_generic,
   ["G"] = luaspob.bg_generic,
   ["H"] = luaspob.bg_desert,
   ["I"] = luaspob.bg_generic,
   ["J"] = luaspob.bg_generic,
   ["K"] = luaspob.bg_generic,
   ["L"] = luaspob.bg_generic,
   ["M"] = luaspob.bg_mclass,
   ["N"] = luaspob.bg_generic,
   ["O"] = luaspob.bg_tundra,
   ["P"] = luaspob.bg_tundra,
   ["Q"] = luaspob.bg_generic,
   ["R"] = luaspob.bg_generic,
   ["S"] = luaspob.bg_generic,
   ["T"] = luaspob.bg_generic,
   ["X"] = luaspob.bg_generic,
   ["Y"] = luaspob.bg_generic,
}
function luaspob.init( spb )
   mem.spob = spb
   mem.std_land = mem.params.std_land or 0 -- Needed for can_land
   barbg = bg_mapping[ spb:class() ]
   if not barbg then
      barbg = luaspob.bg_generic
   end
end

local function msg_bribed_def ()
   local msg_def = {
      _([["Make it quick."]]),
      _([["Don't let anyone see you."]]),
      _([["Be quiet about this."]]),
   }
   local fct = mem.spob:faction()
   if fct == f_zalek then
      return {
         _([["PRIVILEGE OVERRIDE. LANDING AUTHORIZED."]]),
         _([["DATABASE ERROR. AUTHORIZATION GRANTED."]]),
         _([["LANDING DEN...  ...AUTHORIZED."]]),
      }
   end
   return msg_def
end

local function msg_denied_def ()
   local msg_def = {
      _([["Landing request denied."]]),
      _([["Landing not authorized."]]),
      _([["Landing denied."]]),
   }
   local fct = mem.spob:faction()
   if fct == f_empire then
      return {
         _([["Paperwork not in order. Landing denied."]]),
         _([["Credentials invalid. Landing request denied."]]),
      }
   elseif fct == f_zalek then
      return {
         _([["ACCESS DENIED."]]),
         _([["CONNECTION REFUSED."]]), -- ssh error
         fmt.f(_([["INVALID USER {player}."]]),{player=string.upper(player.name() or "")}),
         _([["CONNECTION CLOSED."]]),
         _([["INSUFFICIENT PRIVILEGES."]]),
         _([["LANDING PERMISSIONS NOT FOUND IN DATABASE."]]),
      }
   end
   return msg_def
end

local function msg_granted_def ()
   local msg_def = {
      _([["Permission to land granted."]]),
      _([["You are clear to land."]]),
      _([["Proceed to land."]]),
      _([["Landing authorized."]]),
   }

   local fct = mem.spob:faction()
   if fct == f_empire then
      return {
         _([["Paperwork approved. Landing authorized."]]),
         _([["Landing papers in order. Proceed to land."]]),
         _([["Landing authorization accredited."]]),
      }
   elseif fct == f_zalek then
      return {
         _([["CONTROL DRONE AUTHORIZING LANDING."]]),
         _([["LANDING ALGORITHM INITIALIZED. PROCEED TO LANDING PAD."]]),
         _([["INITIALIZING LANDING PROCEDURE. AUTHORIZATION GRANTED."]]),
         _([["DOCKING SEQUENCE TRANSMITTED."]]),
      }
   elseif fct == f_soromid then
      return {
         _([["Welcome Wanderer. Proceed to land."]]),
         _([["Landing sanctioned, continue to ship nests."]]),
         _([["Greetings Traveller. Land at free ship nest."]]),
      }
   elseif fct == f_sirius then
      return {
         _([["Landing authorized. May Sirichana guide you."]]),
         fmt.f(_([["You are clear to land. Proceed to landing chamber {num}."]]), {num=rnd.rnd(1,100)}),
         _([["Landing accepted. Welcome to Sirichana's domain."]]),
      }
   elseif fct == f_dvaered then
      return {
         _([["Hostilities not detected. Touch down now."]]),
         _([["Green for landing. No dawdling."]]),
         _([["Permission granted. Make it swift."]]),
         _([["Landing authorized. Make sure to power down weapons."]]),
      }
   elseif fct == f_proteron then
      return {
         _([["You loyalty to the Circle is noted. Commence landing when ready."]]), -- The landing request implicitly has a declaration of belief in the Circle
         fmt.f(_([["Landing ID X{num1}-{num2}, you are cleared to land. Follow the rules."]]), {num1=rnd.rnd(0, 9), num2=rnd.rnd(0, 99)}), --ID of an individual land request
         _([["You may land. Act appropriately while landed."]]), -- A true Proteron would not need to be told, but you are an outsider.
         _([["We have received your loyalty oath, pilot. You may land."]]), -- Colonel Cathcart smiles down on us
      }
   end
   return msg_def
end

local function msg_cantbribe_def ()
   local msg_def = {
      _([["We do not accept bribes."]]),
   }
   local fct = mem.spob:faction()
   if fct == f_zalek then
      return {
         _([["LANDING OVERRIDE PROGRAM NOT FOUND."]]),
         _([["USER NOT IN SUDOERS."]]),
      }
   end
   return msg_def
end

local function msg_trybribe_def ()
   local msg_def = {
      _([["I'll let you land for the modest price of {credits}."

Pay {credits}?]]),
      _([["Some {credits} would make me reconsider letting you land."

Pay {credits}?]]),
   }
   local fct = mem.spob:faction()
   if fct == f_zalek then
      return {
         _([["{credits} REQUIRED FOR LANDING ACCESS."

Pay {credits}?]]),
         _([["LANDING OVERRIDE PROVIDED FOR {credits}."

Pay {credits}?]]),
      }
   end
   return msg_def
end

local function msg_dangerous_def ()
   local msg_def = {
      _([["I'm not dealing with dangerous criminals like you!"]]),
   }
   local fct = mem.spob:faction()
   if fct == f_zalek then
      return {
         _([["DANGER DETECTED. DISABLING LANDING PROTOCOL."]]),
      }
   end
   return msg_def
end

function luaspob.load ()
   -- Basic stuff
   local fct = mem.spob:faction()
   mem.bribed = false

   mem.bribe_cost_function = mem.params.bribe_cost or function ()
      local std = fct:playerStanding()
      return (mem.std_land-std) * 1e3 * player.pilot():ship():size() + 5e3
   end

   mem.std_land = mem.params.std_land or 0
   mem.std_bribe = mem.params.std_bribe or -30
   mem.std_dangerous = mem.params.std_dangerous or -30

   mem.msg_bribed = mem.params.msg_bribed or msg_bribed_def()
   mem.msg_denied = mem.params.msg_denied or msg_denied_def()
   mem.msg_notyet = mem.params.msg_notyet or mem.msg_denied
   mem.msg_granted = mem.params.msg_granted or msg_granted_def()
   mem.msg_cantbribe = mem.params.msg_cantbribe or msg_cantbribe_def()
   mem.msg_trybribe = mem.params.msg_trybribe or msg_trybribe_def()
   mem.msg_dangerous = mem.params.msg_dangerous or msg_dangerous_def()

   -- Randomly choose
   local function choose( tbl )
      local msg = tbl[ rnd.rnd(1,#tbl) ]
      if type(tbl)=='function' then
         msg = msg()
      end
      return msg
   end
   mem.msg_bribed     = choose( mem.msg_bribed )
   mem.msg_denied     = choose( mem.msg_denied )
   mem.msg_notyet     = choose( mem.msg_notyet )
   mem.msg_granted    = choose( mem.msg_granted )
   mem.msg_cantbribe  = choose( mem.msg_cantbribe )
   mem.msg_trybribe   = choose( mem.msg_trybribe )
   mem.msg_dangerous  = choose( mem.msg_dangerous )
end

function luaspob.unload ()
   mem.bribed = false
end

function luaspob.can_land ()
   local s = mem.spob:services()
   if not s.land then
      return false,nil -- Use default landing message
   end
   if mem.bribed or mem.spob:getLandAllow() then
      return true, mem.msg_granted
   end
   local fct = mem.spob:faction()
   if not fct then
      return true,nil -- Use default landing message
   end
   local std = fct:playerStanding()
   if mem.spob:getLandDeny() or std < 0 then
      return false, mem.msg_denied
   end
   if std < mem.std_land then
      return false, mem.msg_notyet
   end
   return true, mem.msg_granted
end

function luaspob.comm ()
   local s = mem.spob:services()
   if not s.inhabited then
      return false
   end

   local fct = mem.spob:faction()
   vn.clear()
   vn.scene()
   local spb = ccomm.newCharacterSpob( vn, mem.spob, {
      bribed = mem.bribed,
   } )
   vn.transition()
   vn.na(fmt.f(_("You establish a communication channel with the authorities at {spb}."),
      {spb=mem.spob}))

   vn.label("menu")
   vn.menu( function ()
      local opts = {
         { _("Close"), "leave" }
      }
      local std = fct:playerStanding()
      if std < mem.std_land and not mem.bribed then
         table.insert( opts, 1, { _("Bribe"), "bribe" } )
      end
      return opts
   end )

   local bribe_cost
   vn.label("bribe")
   vn.func( function ()
      local std = fct:playerStanding()
      if std < mem.std_dangerous then
         vn.jump("dangerous")
         return
      end
      if std < mem.std_bribe then
         vn.jump("nobribe")
         return
      end
      bribe_cost = mem.bribe_cost_function( mem.spob )
   end )
   spb( function ()
      return fmt.f( mem.msg_trybribe, {credits=fmt.credits( bribe_cost )} )
   end )
   vn.menu( function ()
      return {
         { fmt.f(_("Pay {credits}"),{credits=fmt.credits( bribe_cost )}), "bribe_yes" },
         { _("Refuse"), "bribe_no" },
      }
   end )

   vn.label("bribe_yes")
   vn.func( function ()
      if bribe_cost > player.credits() then
         vn.jump("player_broke")
         return
      end
      player.pay( -bribe_cost )
      mem.bribed = true
      mem.params.bribed = true
      ccomm.nameboxUpdateSpob( mem.spob, mem.params )
   end )
   spb( mem.msg_bribed )
   vn.jump("menu")

   vn.label("player_broke")
   vn.na( function ()
      local cstr = fmt.credits( player.credits() )
      local cdif = fmt.credits( bribe_cost - player.credits() )
      return fmt.f(_("You only have {credits} credits. You need #r{cdif}#0 more to be able to afford the bribe!"), {credits=cstr, cdif=cdif} )
   end )
   vn.jump("menu")

   vn.label("bribe_no")
   vn.na(_("You refuse to pay the bribe."))
   vn.jump("menu")

   vn.label("nobribe")
   spb( mem.msg_cantbribe )
   vn.jump("menu")

   vn.label("dangerous")
   spb( mem.msg_dangerous )
   vn.jump("menu")

   vn.label("leave")
   vn.run()

   mem.spob:canLand() -- forcess a refresh of condition
   return true
end

function luaspob.population ()
   return fmt.f(_("roughly {amt}"),{amt=fmt.humanize( mem.spob:population() )})
end

local idata = li.newImageData( 1, 1 )
idata:setPixel( 0, 0, 0.5, 0.5, 0.5, 1 )
local img = lg.newImage( idata )
local pixellight = [[
vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec2 p = uv*2.0-1.0;
   float d = length(p)-1.0;
   colour.a *= smoothstep( 0.0, 0.7, -d );
   return colour;
}
]]
local lightshader
local function light( x, y, r )
   if not lightshader then
      lightshader = lg.newShader( pixellight )
   end

   lg.setShader(lightshader)
   lg.draw( img, x, y, 0, r, r )
   lg.setShader()
end

local pixelrectlight = [[
#include "lib/sdf.glsl"
vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec2 p = uv*2.0-1.0;
   float d = sdBox( p, vec2(0.5) )-0.5;
   colour.a *= smoothstep( 0.0, 0.7, -d );
   return colour;
}
]]
local rectlightshader
local function rectlight( x, y, w, h, r )
   if not rectlightshader then
      rectlightshader = lg.newShader( pixelrectlight )
   end

   lg.setShader(rectlightshader)
   lg.draw( img, x, y, r, w, h )
   lg.setShader()
end

local pixelgrad = [[
uniform vec4 u_col;
uniform vec2 u_vec;
vec4 effect( vec4 colour, Image tex, vec2 uv, vec2 px )
{
   vec2 p = uv*2.0-1.0;
   return mix( colour, u_col, smoothstep(-1.0, 1.0, dot(p,u_vec)) );
}
]]
local gradshader
local function gradient( x, y, r, g, b, a )
   if not gradshader then
      gradshader = lg.newShader( pixelgrad )
   end

   local w, h = lg.getDimensions()
   gradshader:send( "u_col", {r,g,b,a})
   gradshader:send( "u_vec", {x,y} )
   lg.setShader(gradshader)
   lg.draw( img, 0, 0, 0, w, h )
   lg.setShader()
end

local function calpha( c, a )
   return {c[1], c[2], c[3], a}
end

local function bg_generator( params )
   params = params or {}
   local cvs = lg.newCanvas( 400, 300 )
   local w, h = cvs:getDimensions()

   local colbg    = params.colbg    or {0.2, 0.15, 0.1, 1}
   local colfeat  = params.colfeat  or {0.4, 0.15, 0.1, 1}
   local collight = params.collight or {1.0, 0.9, 0.5, 1}
   local featrnd  = params.featrnd  or {0.2, 0.2, 0.2}
   local featalpha = params.featalpha or 0.2
   local featrandonmess = params.featrandonmess or 0.1
   local featscale = params.featscale or 1
   local nfeats   = 20
   local nlights  = params.nlights  or 7
   local lightbrightness = params.lightbrightness or 0.4
   local lightrandomness = params.lightrandomness or 0.3

   -- Do some background
   lg.setCanvas( cvs )
   lg.setColor( colbg )
   gradient( -0.1+0.2*rnd.rnd(), 0.5+0.2*rnd.rnd(), 0, 0, 0, 1 );
   for i=1,nfeats do
      local c = calpha( colfeat, featalpha+featrandonmess*rnd.rnd() )
      c[1] = c[1] + rnd.rnd()*featrnd[1]
      c[2] = c[2] + rnd.rnd()*featrnd[2]
      c[3] = c[3] + rnd.rnd()*featrnd[3]
      lg.setColor( c )
      if rnd.rnd() < 0.6 then
         local r = rnd.rnd()*(w+h)*0.4 * featscale
         light( rnd.rnd()*w-r*0.5, rnd.rnd()*h-r*0.5+0.3*h, r )
      else
         local bw = (0.2+0.4*rnd.rnd())*w * featscale
         local bh = (0.1+0.3*rnd.rnd())*h * featscale
         rectlight( rnd.rnd()*w-bw*0.5, rnd.rnd()*h-bh*0.5+0.3*h, bw, bh, (rnd.rnd()*2.0-1.0)*math.rad(15) )
      end
   end

   -- Do some lights
   for i=1,nlights do
      lg.setColor( calpha( collight, lightbrightness+lightrandomness*rnd.rnd() ) )
      local r = (0.1+0.15*rnd.rnd())*(w+h)*0.5
      light( rnd.rnd()*w-r*0.5, (0.1+0.5*rnd.rnd())*h-r*0.5, r )
   end

   lg.setCanvas()
   return cvs.t.tex
end

function luaspob.bg_inert ()
   return bg_generator{
      colbg    = { 0.5, 0.5, 0.5, 1 },
      colfeat  = { 0.1, 0.1, 0.1, 1 },
      collight = { 0.9, 0.9, 0.9, 1 },
      featrnd  = { 0.1, 0.1, 0.1 },
      nlights = rnd.rnd(6,7),
   }
end

function luaspob.bg_desert ()
   return bg_generator{
      colbg    = { 0.5, 0.4, 0.1, 1 },
      colfeat  = { 0.6, 0.5, 0.2, 1 },
      collight = { 1.0, 1.0, 0.9, 1 },
      featrnd  = { 0.2, 0.2, 0.1 },
      nlights  = rnd.rnd(4,6),
   }
end

function luaspob.bg_lava ()
   return bg_generator{
      colbg    = { 0.7, 0.4, 0.4, 1 },
      colfeat  = { 0.6, 0.2, 0.2, 1 },
      collight = { 1.0, 0.9, 0.9, 1 },
      featrnd  = { 0.4, 0.2, 0.1 },
      featalpha = 0.4,
      featrandonmess = 0.2,
      nlights  = rnd.rnd(6,8),
   }
end

function luaspob.bg_tundra ()
   return bg_generator{
      colbg    = { 0.6, 0.9, 0.9, 1 },
      colfeat  = { 0.4, 0.8, 0.8, 1 },
      collight = { 1.0, 1.0, 1.0, 1 },
      featrnd  = { 0.2, 0.3, 0.3 },
      featscale = 1.5,
      nlights  = rnd.rnd(6,8),
   }
end

function luaspob.bg_underwater ()
   return bg_generator{
      colbg    = { 0.3, 0.3, 0.8, 1 },
      colfeat  = { 0.2, 0.2, 0.8, 1 },
      collight = { 0.9, 0.9, 1.0, 1 },
      featrnd  = { 0.8, 0.8, 0.8 },
      nlights  = rnd.rnd(7,9),
   }
end

function luaspob.bg_mclass ()
   return bg_generator{
      colbg    = { 0.2, 0.6, 0.2, 1 },
      colfeat  = { 0.2, 0.8, 0.2, 1 },
      collight = { 0.95, 1.0, 0.95, 1 },
      featrnd  = { 0.6, 0.4, 0.6 },
      nlights  = rnd.rnd(3,5),
      featalpha = 0.1,
   }
end

function luaspob.bg_station ()
   return bg_generator{
      colbg    = { 0.4, 0.4, 0.4, 1 },
      colfeat  = { 0.1, 0.1, 0.1, 1 },
      collight = { 0.9, 0.9, 0.9, 1 },
      featrnd  = { 0.2, 0.2, 0.2 },
      featalpha = 0.4,
      featrandonmess = 0.2,
      featscale = 0.8,
      nfeats = 50,
      nlights = rnd.rnd(10,15),
      lightbrightness = 0.5,
   }
end

function luaspob.bg_generic ()
   return bg_generator{
      nlights = rnd.rnd(6,8)
   }
end

return luaspob
