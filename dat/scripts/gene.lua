local luatk = require "luatk"

local gene = {}

local skills = {
   [N_("Cannibalism I")] = {
      tier = 0,
   },
   [N_("Cannibalism II")] = {
      tier = 5,
      requires = { "Cannibalism I" },
   },
   -- Core Gene Drives
   [N_("Gene Drive I")] = {
      tier = 0,
   },
   [N_("Gene Drive II")] = {
      tier = 2,
      requires = { "Gene Drive I" },
   },
   [N_("Gene Drive III")] = {
      tier = 4,
      requires = { "Gene Drive II" },
   },
   [N_("Gene Drive IV")] = {
      tier = 6,
      requires = { "Gene Drive III" },
   },
   -- Core Brains
   [N_("Brain Stage I")] = {
      tier = 0,
   },
   [N_("Brain Stage II")] = {
      tier = 2,
      requires = { "Brain Stage I" },
   },
   [N_("Brain Stage III")] = {
      tier = 4,
      requires = { "Brain Stage II" },
   },
   [N_("Brain Stage IV")] = {
      tier = 6,
      requires = { "Brain Stage III" },
   },
   -- Core Shells
   [N_("Shell Stage I")] = {
      tier = 0,
   },
   [N_("Shell Stage II")] = {
      tier = 2,
      requires = { "Shell Stage I" },
   },
   [N_("Shell Stage III")] = {
      tier = 4,
      requires = { "Shell Stage II" },
   },
   [N_("Shell Stage IV")] = {
      tier = 6,
      requires = { "Shell Stage III" },
   },
   -- Weapons
   [N_("Stinger I")] = {
      tier = 1,
      conflicts = { "Claw I" },
   },
   [N_("Stinger II")] = {
      tier = 3,
      requires = { "Stinger I" },
   },
   [N_("Claw I")] = {
      tier = 1,
   },
   [N_("Claw II")] = {
      tier = 3,
      requires = { "Claw I" },
   }
}

function gene.window ()
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
      s.name = k
      s.x = 0
      s.y = s.tier
      local con = s.conflicts or {}
      for i,c in ipairs(con) do
         local s2 = skills[c]
         s2.conflicts = s2.conflicts or {}
         if not inlist( s2.conflicts, k ) then
            table.insert( s2.conflicts, k )
         end
         s2.conflicted_by = s
      end
      local req = s.requires or {}
      for i,r in ipairs(req) do
         local s2 = skills[r]
         s2.required_by = s
      end
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
      for i,c in ipairs(node.conflicts or {}) do
         grp = create_group_rec( grp, skills[c], x+1 )
      end
      if node.required_by then
         grp = create_group_rec( grp, node.required_by, x )
      end
      for i,r in ipairs(node.requires or {}) do
         grp = create_group_rec( grp, skills[r], x )
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
   table.sort( groups, function( a, b ) return a.w*a.h < b.w*b.h end ) -- Sort by largest first

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
            --print( string.format( "%s : %d x %d x %d x %d", g[1].name, x, g.y, g.w, g.h ) )
            g.x = x
            g.set = true
            for i,s in ipairs(g) do
               local px = g.x+s.x
               local py = s.y
               table.insert( skillslist, {
                  name = s.name,
                  x    = px,
                  y    = py,
               } )
               if s.required_by then
                  table.insert( skillslink, {
                     x1 = px,
                     y1 = py,
                     x2 = g.x+s.required_by.x,
                     y2 = s.required_by.y,
                  } )
               end
               if s.conflicted_by then
                  table.insert( skillslink, {
                     x1 = px,
                     y1 = py,
                     x2 = g.x+s.conflicted_by.x,
                     y2 = s.conflicted_by.y,
                  } )
               end
            end
            break
         end
      end
   end

   local w, h = 1000, 600
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
   local bx, by = 50, 50
   local sw, sh = 70, 70
   local scol = {1, 1, 1, 0.2}
   for k,l in ipairs(skillslink) do
      luatk.newRect( wdw, bx+sw*l.x1+30, by+sh*l.y1+30, sw*(l.x2-l.x1)+10, sh*(l.y2-l.y1)+10, scol )
   end
   for k,s in ipairs(skillslist) do
      luatk.newButton( wdw, bx+sw*s.x+10, by+sh*s.y+10, 50, 50, s.name, function () end )
   end

   luatk.run()
end

return gene
