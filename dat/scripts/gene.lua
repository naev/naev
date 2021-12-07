local luatk = require "luatk"
local lg = require "love.graphics"
local utility = require "pilotname.utility"
local fmt = require "format"

local gene = {}

local skills = {
   ["cannibal1"] = {
      name = _("Cannibalism I"),
      tier = 0,
      desc = _("The ship is able to cannibalize boarded vessels to restore armour. For every 2 points of armour cannibalized, the ship will gain a single point of armour."),
   },
   ["cannibal2"] = {
      name = _("Cannibalism II"),
      tier = 5,
      requires = { "cannibal1" },
      desc = _("Cannibalizing boarded ships will now restore 2 points of armour per 3 points of armour cannibalized, and will also similarly restore energy."),
   },
   -- Core Gene Drives
   ["engines1"] = {
      name = _("Gene Drive I"),
      tier = 0,
   },
   ["engines2"] = {
      name = _("Gene Drive II"),
      tier = 2,
      requires = { "engines1" },
   },
   ["engines3"] = {
      name = _("Gene Drive III"),
      tier = 4,
      requires = { "engines2" },
   },
   ["engines4"] = {
      name = _("Gene Drive IV"),
      tier = 6,
      requires = { "engines3" },
   },
   -- Core Brains
   ["systems1"] = {
      name = _("Brain Stage I"),
      tier = 0,
   },
   ["systems2"] = {
      name = _("Brain Stage II"),
      tier = 2,
      requires = { "systems1" },
   },
   ["systems3"] = {
      name = _("Brain Stage III"),
      tier = 4,
      requires = { "systems2" },
   },
   ["systems4"] = {
      name = _("Brain Stage IV"),
      tier = 6,
      requires = { "systems3" },
   },
   -- Core Shells
   ["hull1"] = {
      name = _("Shell Stage I"),
      tier = 0,
   },
   ["hull2"] = {
      name = _("Shell Stage II"),
      tier = 2,
      requires = { "hull1" },
   },
   ["hull3"] = {
      name = _("Shell Stage III"),
      tier = 4,
      requires = { "hull2" },
   },
   ["hull4"] = {
      name = _("Shell Stage IV"),
      tier = 6,
      requires = { "hull3" },
   },
   -- Left Weapon
   ["weap1a1"] = {
      name = _("Left Stinger I"),
      tier = 1,
      conflicts = { "weap1b1" },
   },
   ["weap1a2"] = {
      name = _("Left Stinger I"),
      tier = 3,
      requires = { "weap1a1" },
   },
   ["weap1b1"] = {
      name = _("Left Claw I"),
      tier = 1,
   },
   ["weap1b2"] = {
      name = _("Left Claw II"),
      tier = 3,
      requires = { "weap1b1" },
   },
   -- Movement Line
   ["compoundeyes"] = {
      name = _("Compound Eyes"),
      tier = 3,
   },
   ["hunterspirit"] = {
      name =_("Hunter Spirit"),
      tier = 5,
      requires = { "compoundeyes" },
      conflicts = { "wandererspirit" },
   },
   ["afterburner1"] = {
      name = _("Adrenaline Gland I"),
      tier = 2,
   },
   ["afterburner2"] = {
      name = _("Adrenaline Gland II"),
      tier = 4,
      requires = { "afterburner1" },
   },
   ["wandererspirit"] = {
      name = _("Wanderer Spirit"),
      tier = 5,
      requires = { "afterburner2" },
   },
   -- Health Line
   ["health1"] = {
      name = _("Bulky Abdomen"),
      tier = 1,
   },
   ["health2"] = {
      name = _("Regeneration I"),
      tier = 3,
      requires = { "health1" },
   },
   ["health3"] = {
      name = _("Hard Shell"),
      tier = 4,
      requires = { "health2" },
   },
   ["health4"] = {
      name = _("Regeneration II"),
      tier = 5,
      requires = { "health3" }
   },
   -- Attack Line
   ["attack1"] = {
      name = _("Feral Rage"),
      tier = 3,
   },
   ["attack2"] = {
      name = _("Adrenaline Hormone"),
      tier = 5,
      requires = { "attack1" },
   },
}

function gene.window ()
   --local level = 10

   local function inlist( lst, item )
      for k,v in ipairs(lst) do
         if v==item then
            return true
         end
      end
      return false
   end

   -- Set up some helper fields
   for k,s in pairs(skills) do
      s.x = 0
      s.y = s.tier
      s._conflicts = s._conflicts or {}
      for i,c in ipairs(s.conflicts or {}) do
         local s2 = skills[c]
         s2._conflicts = s2._conflicts or {}
         if not inlist( s2._conflicts, s ) then
            table.insert( s2._conflicts, s )
         end
         table.insert( s._conflicts, s2 )
      end
      s._requires = s._requires or {}
      local req = s.requires or {}
      for i,r in ipairs(req) do
         local s2 = skills[r]
         s2._required_by = s2._required_by or {}
         if not inlist( s2._required_by, s ) then
            table.insert( s2._required_by, s )
         end
         table.insert( s._requires, s2 )
      end
      s._required_by = s._required_by or {}
   end

   -- Recursive group creation
   local function create_group_rec( grp, node, x )
      if node._g then return grp end
      node._g= true
      node.x = x
      grp.x  = math.min( grp.x, node.x )
      grp.y  = math.min( grp.y, node.y )
      grp.x2 = math.max( grp.x2, node.x )
      grp.y2 = math.max( grp.y2, node.y )
      table.insert( grp, node )
      for i,c in ipairs(node._conflicts) do
         grp = create_group_rec( grp, c, x+1 )
      end
      for i,r in ipairs(node._required_by) do
         grp = create_group_rec( grp, r, x )
      end
      for i,r in ipairs(node._requires) do
         grp = create_group_rec( grp, r, x )
      end
      return grp
   end

   -- Create the group list
   local groups = {}
   for k,s in pairs(skills) do
      if not s._g then
         local grp = create_group_rec( {x=math.huge,y=math.huge,x2=-math.huge,y2=-math.huge}, s, 0 )
         grp.w = grp.x2-grp.x
         grp.h = grp.y2-grp.y
         table.insert( groups, grp )
      end
   end
   --table.sort( groups, function( a, b ) return #a-#b < 0 end ) -- Sort by largest first
   table.sort( groups, function( a, b ) return a.w*a.h > b.w*b.h end ) -- Sort by largest first

   -- true if intersects
   local function aabb_vs_aabb( a, b )
      if a.x+a.w < b.x then
         return false
      elseif a.y+a.h < b.y then
         return false
      elseif a.x > b.x+b.w then
         return false
      elseif a.y > b.y+b.h then
         return false
      end
      return true
   end

   local function fits_location( x, y, w, h )
      local t = {x=x,y=y,w=w,h=h}
      for k,g in ipairs(groups) do
         if g.set then
            if aabb_vs_aabb( g, t ) then
               return false
            end
         end
      end
      return true
   end
   -- Figure out location greedily
   local skillslist = {}
   local skillslink = {}
   for k,g in ipairs(groups) do
      for x=0,20 do
         if fits_location( x, g.y, g.w, g.h ) then
            g.x = x
            g.set = true
            for i,s in ipairs(g) do
               local px = g.x+s.x
               local py = s.y
               local alt = "#o"..s.name.."#0"
               if s.desc then
                  alt = alt.."\n\n"..s.desc
               end
               if #s._requires > 0 then
                  local req = {}
                  for j,r in ipairs(s._requires) do
                     table.insert( req, r.name )
                  end
                  alt = alt.."\n#b".._("Requires: ").."#0"..fmt.list( req )
               end
               if #s._conflicts > 0 then
                  local con = {}
                  for j,c in ipairs(s._conflicts) do
                     -- TODO colour code based on whether acquired
                     table.insert( con, c.name )
                  end
                  alt = alt.."\n#b".._("Conflicts: ").."#0"..fmt.list( con )
               end
               s.rx = px
               s.ry = py
               s.alt = alt
               s.enabled = (s.tier==0)
               table.insert( skillslist, s )
               for j,r in ipairs(s._required_by) do
                  table.insert( skillslink, {
                     x1 = px,
                     y1 = py,
                     x2 = g.x+r.x,
                     y2 = r.y,
                  } )
               end
               for j,c in ipairs(s._conflicts) do
                  table.insert( skillslink, {
                     x1 = px,
                     y1 = py,
                     x2 = g.x+c.x,
                     y2 = c.y,
                  } )
               end
            end
            break
         end
      end
   end

   local function skill_canEnable( s )
      for k,r in ipairs(s._requires) do
         if not r.enabled then
            return false
         end
      end
      for k,c in ipairs(s._conflicts) do
         if c.enabled then
            return false
         end
      end
      return true
   end

   local SkillIcon = {}
   setmetatable( SkillIcon, { __index = luatk.Widget } )
   local SkillIcon_mt = { __index = SkillIcon }
   local function newSkillIcon( parent, x, y, w, h, s )
      local wgt   = luatk.newWidget( parent, x, y, w, h )
      setmetatable( wgt, SkillIcon_mt )
      wgt.skill   = s
      return wgt
   end
   local font = lg.newFont(12)
   function SkillIcon:draw( bx, by )
      local s = self.skill
      local x, y = bx+self.x, by+self.y
      if s.enabled then
         lg.setColor( {0,0.4,0.8,1} )
      elseif not skill_canEnable(s) then
         lg.setColor( {0.5,0.2,0.2,1} )
      else
         lg.setColor( {0,0,0,1} )
      end
      lg.rectangle( "fill", x+10,y+10,50,50 )
      lg.setColor( {1,1,1,1} )
      lg.printf( self.skill.name, font, x+15, y+15, self.w-30 )
   end
   function SkillIcon:drawover( bx, by )
      local x, y = bx+self.x, by+self.y
      if self.mouseover then
         luatk.drawAltText( x+60, y+10, self.skill.alt, 300 )
      end
   end
   function SkillIcon:clicked ()
      local s = self.skill
      if skill_canEnable( s ) then
         self.skill.enabled = true
      end
   end

   local w, h = 1100, 600
   local wdw = luatk.newWindow( nil, nil, w, h )
   local function wdw_done( dying_wdw )
      dying_wdw:destroy()
      return true
   end
   wdw:setCancel( wdw_done )
   luatk.newText( wdw, 0, 10, w, 20, _("BioShip Skills"), nil, "center" )
   luatk.newButton( wdw, w-100-20, h-40-20, 100, 40, _("OK"), function( wgt )
      wgt.parent:destroy()
   end )
   local bx, by = 20, 40
   local sw, sh = 70, 70
   -- Tier stuff
   for i=0,7 do
      local col = { 0.95, 0.95, 0.95 }
      luatk.newText( wdw, bx, by+sh*i+(sh-12)/2, 70, 30, string.format(_("TIER %s"),utility.roman_encode(i)), col, "center", font )
   end
   bx = bx + sw
   -- Elements
   local scol = {1, 1, 1, 0.2}
   for k,l in ipairs(skillslink) do
      luatk.newRect( wdw, bx+sw*l.x1+30, by+sh*l.y1+30, sw*(l.x2-l.x1)+10, sh*(l.y2-l.y1)+10, scol )
   end
   for k,s in ipairs(skillslist) do
      newSkillIcon( wdw, bx+sw*s.rx, by+sh*s.ry, 70, 70, s )
      --luatk.newButton( wdw, bx+sw*s.x+10, by+sh*s.y+10, 50, 50, s.name, function( wgt ) end )
   end

   luatk.run()
end

return gene
