local luatk = require "luatk"
local lg = require "love.graphics"
local utility = require "pilotname.utility"
local fmt = require "format"
local bioskills = require "bioship.skills"

local bioship = {}

local function inlist( lst, item )
   for k,v in ipairs(lst) do
      if v==item then
         return true
      end
   end
   return false
end

local stage, skills, intrinsics, skillpoints, skilltxt
function bioship.window ()
   local pp = player.pilot()
   local ps = pp:ship()

   -- Only bioships are good for now
   if not ps:tags().bioship then
      return
   end

   local skilllist = { "bite", "health", "attack", "misc", "plasma" }
   local maxtier = 3
   local pss = ps:size()
   if pss > 4 then
      maxtier = 5
   elseif pss > 2 then
      maxtier = 4
   end
   if pss <= 4 then
      table.insert( skilllist, "move" )
      table.insert( skilllist, "stealth" )
   end
   -- TODO other ways of increasing tiers
   skills = bioskills.get( skilllist )
   intrinsics = bioskills.ship[ ps:nameRaw() ]

   -- Filter out high tiers if necessary
   local nskills = {}
   for k,s in pairs(skills) do
      if s.tier <= maxtier then
         nskills[k] = s
      end
   end
   skills = nskills

   -- Set up some helper fields
   for k,s in ipairs(intrinsics) do
      s.stage = k
      local alt = "#o"..s.name.."#0"
      if s.stage==1 then
         alt = alt.."\n".._("Always available")
      else
         alt = alt.."\n"..fmt.f(_("Obtained at stage {stage}"),{stage=s.stage})
      end
      if s.desc then
         if type(s.desc)=="function" then
            alt = alt.."\n\n"..s.desc(pp)
         else
            alt = alt.."\n\n"..s.desc
         end
      end
      local outfit = s.outfit
      if type(outfit)=="string" then
         outfit = {outfit}
      end
      if #outfit > 0 then
         alt = alt.."\n#b".._("Provides:").."#0"
      end
      for i,o in ipairs(outfit) do
         alt = alt.."\n".._("* ")..naev.outfit.get(o):name()
      end
      s.alt = alt
      s.name = fmt.f(_("Stage {stage}"),{stage=s.stage}).."\n"..s.name
   end
   for k,s in pairs(skills) do
      -- Tests outfits if they exist
      if __debugging then
         local outfit = s.outfit or {}
         if type(outfit)=="string" then
            outfit = {outfit}
         end
         for i,o in ipairs(outfit) do
            if naev.outfit.get(o) == nil then
               warn(fmt.f(_("Bioship skill '{skill}' can't find outfit '{outfit}'!"),{skill=k, outfit=o}))
            end
         end
      end
      s.id = "bio_"..k
      s.x = 0
      s.y = s.tier
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

      s.enabled = player.shipvarPeek( s.id )
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
      local xoff = x-1
      for i,r in ipairs(node._required_by) do
         if not r._g then
            xoff = xoff+1
            grp = create_group_rec( grp, r, xoff )
         end
      end
      for i,r in ipairs(node._requires) do
         grp = create_group_rec( grp, r, x+i-1 )
      end
      return grp
   end

   -- Create the group list
   local groups = {}
   for k,s in pairs(skills) do
      if not s._g and #s._requires <= 0 then
         local grp = create_group_rec( {x=math.huge,y=math.huge,x2=-math.huge,y2=-math.huge}, s, 0 )
         grp.w = grp.x2-grp.x
         grp.h = grp.y2-grp.y
         table.insert( groups, grp )
      end
   end
   --table.sort( groups, function( a, b ) return #a-#b < 0 end ) -- Sort by largest first
   --table.sort( groups, function( a, b ) return a.w*a.h > b.w*b.h end ) -- Sort by largest first
   table.sort( groups, function( a, b ) return a[1].name < b[1].name end ) -- Sort by name

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
                  if type(s.desc)=="function" then
                     alt = alt.."\n\n"..s.desc(pp)
                  else
                     alt = alt.."\n\n"..s.desc
                  end
               end
               if #s._requires > 0 then
                  local req = {}
                  for j,r in ipairs(s._requires) do
                     table.insert( req, r.name )
                  end
                  alt = alt.."\n#b".._("Requires: ").."#0"..fmt.list( req )
               end
               s.rx = px
               s.ry = py
               s.alt = alt
               table.insert( skillslist, s )
               for j,r in ipairs(s._required_by) do
                  table.insert( skillslink, {
                     x1 = px,
                     y1 = py,
                     x2 = g.x+r.x,
                     y2 = r.y,
                  } )
               end
            end
            break
         end
      end
   end

   local function skill_canEnable( s )
      if skillpoints <= 0 then
         return false
      end
      if s.stage then
         return false
      end
      for k,r in ipairs(s._requires) do
         if not r.enabled then
            return false
         end
      end
      return true
   end

   local function skill_disable( s )
      local outfit = s.outfit or {}
      local slot   = s.slot
      if type(outfit)=="string" then
         outfit = {outfit}
         if slot then
            slot = {slot}
         end
      end
      if #outfit>0 and slot and #outfit ~= #slot then
         warn(fmt.f(_("Number of outfits doesn't match number of slots for skill '{skill}'"),{skill=s.name}))
      end
      for k,o in ipairs(outfit) do
         if slot then
            local sl = slot[k]
            if pp:outfitSlot( sl ) == naev.outfit.get(o) then
               pp:outfitRmSlot( sl )
            end
         else
            pp:outfitRmIntrinsic( o )
         end
      end
      if s.id then
         player.shipvarPop( s.id )
      end
      if s.shipvar then
         player.shipvarPop( s.shipvar )
      end
      s.enabled = false
   end

   local function skill_enable( p, s )
      local outfit = s.outfit or {}
      local slot   = s.slot
      if type(outfit)=="string" then
         outfit = {outfit}
         if slot then
            slot = {slot}
         end
      end
      if #outfit>0 and slot and #outfit ~= #slot then
         warn(fmt.f(_("Number of outfits doesn't match number of slots for skill '{skill}'"),{skill=s.name}))
      end
      if s.test and not s.test(p) then
         return false
      end
      for k,o in ipairs(outfit) do
         if slot then
            local sl = slot[k]
            pp:outfitRmSlot( sl )
            pp:outfitAddSlot( o, sl, true, true )
         else
            pp:outfitAddIntrinsic( o )
         end
      end
      if s.id then
         player.shipvarPush( s.id, true )
      end
      if s.shipvar then
         player.shipvarPush( s.shipvar, true )
      end
      s.enabled = true
      return true
   end

   local function skill_count()
      local n = 0
      for k,s in pairs(skills) do
         if s.tier~=0 and s.enabled then
            n = n+1
         end
      end
      return n
   end

   local function skill_text ()
      if not skilltxt then
         return
      end
      skilltxt:set( fmt.f(n_("Skills ({point} point remaining)","Skills ({point} points remaining)", skillpoints),{point=skillpoints}) )
   end

   local function skill_reset ()
      -- Get rid of intrinsics
      for k,s in pairs(skills) do
         skill_disable( s )
      end
      skillpoints = stage - skill_count()
      skill_text()
   end

   stage = player.shipvarPeek( "biostage" ) or 1
   skillpoints = stage - skill_count()

   -- Make sure to enable intrinsics if applicable
   -- TODO handle elsewhere
   for k,s in ipairs(intrinsics) do
      if stage >= s.stage then
         skill_enable( pp, s )
      end
   end

   -- Case ship not initialized
   if not player.shipvarPeek( "bioship" ) then
      skill_reset()
      player.shipvarPush( "bioship", true )
   end

   local sfont = lg.newFont(10)
   sfont:setOutline(1)
   local font = lg.newFont(12)
   font:setOutline(1)

   local SkillIcon = {}
   setmetatable( SkillIcon, { __index = luatk.Widget } )
   local SkillIcon_mt = { __index = SkillIcon }
   local function newSkillIcon( parent, x, y, w, h, s )
      local wgt   = luatk.newWidget( parent, x, y, w, h )
      setmetatable( wgt, SkillIcon_mt )
      wgt.skill   = s
      local _maxw, wrapped = sfont:getWrap( s.name, 70 )
      wgt.txth    = #wrapped * sfont:getLineHeight()
      return wgt
   end
   function SkillIcon:draw( bx, by )
      local s = self.skill
      local x, y = bx+self.x, by+self.y
      if s.enabled then
         lg.setColor( {0,0.4,0.8,1} )
         lg.rectangle( "fill", x+10,y+10,70,70 )
      elseif not skill_canEnable(s) then
         lg.setColor( {0.5,0.2,0.2,1} )
         lg.rectangle( "fill", x+10,y+10,70,70 )
      else
         lg.setColor( {0,0.4,0.8,1} )
         lg.rectangle( "fill", x+7,y+7,76,76 )
         lg.setColor( {0,0,0,1} )
         lg.rectangle( "fill", x+10,y+10,70,70 )
      end
      lg.setColor( {1,1,1,1} )
      lg.printf( self.skill.name, sfont, x+10, y+10+(70-self.txth)/2, 70, "center" )
   end
   function SkillIcon:drawover( bx, by )
      local x, y = bx+self.x, by+self.y
      if self.mouseover then
         luatk.drawAltText( x+75, y+10, self.skill.alt, 300 )
      end
   end
   function SkillIcon:clicked ()
      local s = self.skill
      if not s.enabled and skill_canEnable( s ) then
         if skill_enable( pp, s ) then
            skillpoints = skillpoints-1
            skill_text()
         end
      end
   end

   -- Figure out dimension stuff
   local bx, by = 20, 0
   local sw, sh = 90, 90
   local skillx, skilly = 0, 0
   for k,s in pairs(skills) do
      skillx = math.max( skillx, s.rx+0.5 )
      skilly = math.max( skilly, s.ry )
   end
   local intx = 3
   local inty = math.floor((#intrinsics+2)/3)

   local w, h = bx+sw*(skillx+intx+1)+40, by+sh*(math.max(skilly,inty)+1)+80
   luatk.setDefaultFont( font )
   local wdw = luatk.newWindow( nil, nil, w, h )
   local function wdw_done( dying_wdw )
      dying_wdw:destroy()
      return true
   end
   wdw:setCancel( wdw_done )
   luatk.newText( wdw, 0, 10, w, 20, fmt.f(_("Stage {stage} {name}"),{stage=stage,name=player.pilot():ship():name()}), nil, "center" )
   luatk.newButton( wdw, w-120-100-20, h-40-20, 100, 40, _("Reset"), function ()
      skill_reset()
   end )
   luatk.newButton( wdw, w-100-20, h-40-20, 100, 40, _("OK"), function( wgt )
      wgt.parent:destroy()
   end )

   -- Background
   luatk.newRect( wdw, bx, by+sh-30, sw*skillx+sw, sh*skilly+30, {0,0,0,0.8} )
   skilltxt = luatk.newText( wdw, bx+sw, by+70, sw*skillx, 30, "", {1,1,1,1}, "center", font )
   skill_text()

   -- Tier stuff
   for i=1,maxtier do
      local col = { 0.95, 0.95, 0.95 }
      luatk.newText( wdw, bx, by+sh*i+(sh-12)/2, sw/2, sh, utility.roman_encode(i), col, "center", font )
   end
   bx = bx + sw/2-10

   -- Elements
   local scol = {1, 1, 1, 0.2}
   for k,l in ipairs(skillslink) do
      local dx = l.x2-l.x1
      local dy = l.y2-l.y1
      if dx~=0 and dy~=0 then
         -- Go to the side then up
         luatk.newRect( wdw, bx+sw*l.x1+40, by+sh*l.y1+40, sw*dx, 10, scol )
         luatk.newRect( wdw, bx+sw*l.x2+40, by+sh*l.y1+40, 10, sh*dy, scol )
      else
         luatk.newRect( wdw, bx+sw*l.x1+40, by+sh*l.y1+40, sw*dx+10, sh*dy+10, scol )
      end
   end
   for k,s in ipairs(skillslist) do
      newSkillIcon( wdw, bx+sw*s.rx, by+sh*s.ry, sw, sh, s )
   end

   -- Intrinsics
   bx = bx+sw*skillx+70
   luatk.newRect( wdw, bx, by+sh-30, sw*intx, sh*inty+30, {0,0,0,0.8} )
   luatk.newText( wdw, bx, by+70, sw*intx, 30, _("Stage Traits"), {1,1,1,1}, "center", font )
   for k,s in ipairs(intrinsics) do
      local x = bx + ((k-1)%3)*sw
      local y = by + math.floor((k-1)/3)*sh + sh
      newSkillIcon( wdw, x, y, sw, sh, s )
   end

   luatk.run()
end

return bioship
