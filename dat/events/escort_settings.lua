--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Escort Settings">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]
local luatk = require "luatk"

local escort_gui, update_followers
local profile, aggressive, enemyclose, returndist, armourrun

function create ()
   -- Load variables
   profile    = var.peek( "escortai_profile" ) or "Default"
   aggressive = var.peek( "escortai_aggressive" )
   if aggressive == nil then
      aggressive = true
   end
   enemyclose = var.peek( "escortai_enemyclose" ) or 2e3
   returndist = var.peek( "escortai_returndist" ) or 5e3
   armourrun  = var.peek( "escortai_armourrun" ) or 30
   update_followers()

   -- Set an info button up
   player.infoButtonRegister( _("Escort AI"), escort_gui, 2, "E" )

   -- Hooks are cleared upon entering, so we have to readd the hook each time
   hook.enter( "setup" )
end

function setup ()
   -- Set up hook for all newly created pilots
   hook.pilot( nil, "creation", "create_hook" )
end

function update_followers ()
   local pp = player.pilot()
   for k,p in ipairs(pp:followers()) do
      local m = p:memory()
      m.aggressive = aggressive
      m.enemyclose = enemyclose
      m.leadermaxdist = returndist
      if m.carried then
         m.armour_run = armourrun
      end
      p:taskClear()
   end
end

function create_hook( p )
   -- Since this hook runs after direct creation, only pilots spawned
   -- directly as an escort of the player should trigger. This means
   -- that followers spawned by missions and such should not have the
   -- variables overwritten.
   if p:leader()==player.pilot() then
      local m = p:memory()
      m.aggressive = aggressive
      m.enemyclose = enemyclose
      m.leadermaxdist = returndist
      if m.carried then
         m.armour_run = armourrun
      end
   end
end

function escort_gui ()
   local profiles = {
      {
         name = _("Hyper-Offensive"),
         aggressive      = true,
         enemyclose      = math.huge,
         leadermaxdist   = math.huge,
         armour_run      = -1,
      }, {
         name = _("Offensive"),
         aggressive      = true,
         enemyclose      = 3e3,
         leadermaxdist   = 6e3,
         armour_run      = -1,
      }, {
         name = _("Default"),
         aggressive      = true,
         enemyclose      = 2e3,
         leadermaxdist   = 5e3,
         armour_run      = 30,
      }, {
         name = _("Defensive"),
         aggressive      = false,
         enemyclose      = 3e3,
         leadermaxdist   = 5e3,
         armour_run      = 50,
      }, {
         name = _("Hyper-Defensive"),
         aggressive      = false,
         enemyclose      = 1500,
         leadermaxdist   = 3e3,
         armour_run      = 100,
      }, {
         name = _("Custom"),
      }
   }

   local chk_aggressive, fad_enemyclose, fad_returndist, fad_armourrun, lst_profiles

   local function update_pilots ()
      profile    = lst_profiles:get()
      aggressive = chk_aggressive:get()
      enemyclose = fad_enemyclose:get()
      returndist = fad_returndist:get()
      armourrun  = fad_armourrun:get()
      update_followers()
   end

   local profiles_list = {}
   for k,v in ipairs(profiles) do
      profiles_list[k] = v.name
   end

   local w, h = 600, 420
   local wdw = luatk.newWindow( nil, nil, w, h )
   wdw:setCancel( luatk.close )
   luatk.newText( wdw, 0, 10, w, 20, _("Escort Manager"), nil, "center" )

   local function update_wgt ()
      lst_profiles:setItem( "Custom", true )
      update_pilots()
   end

   local y = 55+120+20
   chk_aggressive = luatk.newCheckbox( wdw, 20, y, w-40, 20, _("#nAggressive:#0 escorts seek out hostiles"), update_wgt, aggressive )
   y = y + 30
   local txt_enemyclose = luatk.newText( wdw, 20, y,    w, 20, "#n".._("Engage distance:") )
   local txt_returndist = luatk.newText( wdw, 20, y+40, w, 20, "#n".._("Return distance:") )
   local txtw = math.max( txt_enemyclose:width(), txt_returndist:width() )
   fad_enemyclose = luatk.newFader( wdw, 20+txtw+20, y, w-40-txtw-40, 30, 0, 10e3, enemyclose, update_wgt, {
      labels = true,
   } )
   fad_returndist = luatk.newFader( wdw, 20+txtw+20, y+40, w-40-txtw-40, 30, 0, 10e3, returndist, update_wgt, {
      labels = true,
   } )
   y = y + 80
   local txt_armourrun  = luatk.newText( wdw, 20, y, w, 20, "#n".._("Armour to return to mothership at (only fighters):") )
   txtw = txt_armourrun:width()

   fad_armourrun = luatk.newFader( wdw, 20+txtw+20, y, w-40-txtw-40, 30, 0, 100, armourrun, update_wgt, {
      labels = true,
   } )

   luatk.newText( wdw, 20, 30, 260, 20, "#n".._("Profiles:") )
   lst_profiles = luatk.newList( wdw, 20, 55, 260, 120, profiles_list, function ( itm, idx )
      if itm=="Custom" then return end
      local prof = profiles[idx]
      if prof.aggressive then
         chk_aggressive:set( prof.aggressive, true )
      end
      if prof.enemyclose then
         fad_enemyclose:set( prof.enemyclose, true )
      end
      if prof.leadermaxdist then
         fad_returndist:set( prof.leadermaxdist, true )
      end
      if prof.armour_run then
         fad_armourrun:set(  prof.armour_run, true )
      end
      update_pilots()
   end, 1 )
   lst_profiles:setItem( profile )
   luatk.newButton( wdw, -20, -20, 80, 40, _("Close"), luatk.close )
   luatk.run()

   -- Save as variables
   var.push( "escortai_profile", profile )
   var.push( "escortai_aggressive", aggressive )
   var.push( "escortai_enemyclose", enemyclose )
   var.push( "escortai_returndist", returndist )
   var.push( "escortai_armourrun", armourrun )
end
