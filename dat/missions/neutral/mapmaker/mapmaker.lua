--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Mapping the Universe">
 <unique />
 <chance>10</chance>
 <location>Bar</location>
 <cond>
   local c = spob.cur()
   local f = c:faction()
   if not f or not f:tags("generic") then
      return false
   end
   return true
 </cond>
</mission>
--]]
--[[
   Map Maker

   Help the Map Maker explore areas and find secret jumps!

   Player variables:
   - 'mapmaker_known' => have met once
--]]

local fmt = require "format"
local love_shaders = require "love_shaders"
local lmisn = require "lmisn"
local vn = require "vn"
local lg = require "love.graphics"

local mission_name = _("Mapping the Universe")
local npc_image = "mapmaker.webp"

local targets = {
   {
      centre = system.get("Alteris"),
      var = "mapmaker_wildones",
      rewards = {
         [0.4] = {
            jump.get("Sikh", "Rockbed"),
            jump.get("Coriolis", "Dune"),
         },
         [0.6] = {
            jump.get("Merisi", "Borla"),
            jump.get("Rockbed", "Haven"),
            jump.get("Eridani", "Draconis"),
         },
         [0.8] = {
            jump.get("Bastion", "Halo"),
            jump.get("Acheron", "Terminus")
         },
      },
   },
   {
      centre = system.get("Feye"),
      var = "mapmaker_sirius",
      rewards = {
         [0.4] = {
            jump.get("Feye", "Uridhrion"),
         },
         [0.6] = {
            jump.get("Mizar", "Possum"),
            jump.get("Elza", "Babylon"),
         },
         [0.8] = {
            jump.get("Moordra", "Dust"),
            jump.get("Moordra", "Corvus"),
         },
      },
   },
   {
      centre = system.get("Dvaer"),
      var = "mapmaker_dvaered",
      rewards = {
         [0.4] = {
            jump.get("Ksher", "Farizel"),
            jump.get("Gremlin", "Blacin"),
         },
         [0.6] = {
            jump.get("Castellan", "Maus"),
            jump.get("Klantar", "Theras"),
         },
         [0.8] = {
            jump.get("Blacin", "Wochii"),
            jump.get("Gold", "Perhelion"),
         },
      },
   },
   {
      centre = system.get("Eye of Night"),
      var = "mapmaker_sirius",
      rewards = {
         [0.4] = {
            jump.get("Eye of Night", "Druid"),
            jump.get("Tartan", "Modus Manis"),
         },
         [0.6] = {
            jump.get("Druid", "Peter"),
            jump.get("Norpin", "Lynx"),
         },
         [0.8] = {
            jump.get("Peter", "Felzen"),
            jump.get("Felzen", "Sagittarius Prime"),
         },
      },
   },
}

local function explored( centre, distance )
   local sys_todiscover = 0
   local sys_discovered = 0
   local sys_completed = 0
   local syslist = lmisn.getSysAtDistance( centre, 0, distance )
   for i,sys in ipairs(syslist) do
      local todiscover = 0
      local discovered = 0
      for j,spb in ipairs(sys:spobs()) do
         todiscover = todiscover+1
         if spb:known() then
            discovered = discovered+1
         end
      end
      for j,jmp in ipairs(sys:jumps()) do
         if not jmp:hidden() then
            todiscover = todiscover+1
            if jmp:known() then
               discovered = discovered+1
            end
         end
      end
      sys_todiscover = sys_todiscover+todiscover
      sys_discovered = sys_discovered+discovered
      if todiscover==discovered then
         sys_completed = sys_completed+1
      end
   end
   return sys_todiscover, sys_discovered, sys_completed
end

local function update_marker_single( t )
   local todiscover, discovered = explored( t.centre, t.distance or 5 )
   local progress = discovered / todiscover
   -- Haven't reached final reward
   if progress < 0.8 then
      misn.markerAdd( t.centre, "low" )
      return true
   end
   -- Not all rewards gotten yet
   for th,rwd in pairs(t.rewards) do
      for i,jmp in ipairs(rwd) do
         if not jmp:known() then
            misn.markerAdd( t.centre, "low" )
            return true
         end
      end
   end
   return false
end

-- Remove all markers and add them if still haven't met 80% reward threshold
local function update_markers ()
   local hasrewardleft = false
   misn.markerRm()
   misn.osdDestroy()

   local desc = _("Explore the vicinity around the following marked systems:")
   for k,t in ipairs(targets) do
      if update_marker_single( t ) then
         local todiscover, discovered = explored( t.centre, t.distance or 5 )
         local progress = discovered / todiscover
         local sysname
         if t.centre:known() then
            sysname = t.centre:name()
         else
            sysname = _("Unknown")
         end
         desc = desc.."\n".. fmt.f(_("・{sysname} ({progress}% explored)"),
            {sysname=sysname, progress=fmt.number(progress*100)})
         hasrewardleft = true
      end
   end

   if hasrewardleft then
      misn.setDesc(desc)
      misn.osdCreate( mission_name, {
         _("Explore the area around the marked systems."),
      })
   else
      misn.setDesc(_("You have explored everything for now."))
   end
end

function create()
   misn.setNPC( _("Map Maker"), npc_image, _("A person hunched over a small portable holographic display. It seems like their drink is untouched."))
end

function accept()
   local accepted = false
   local met = var.peek("mapmaker_met")

   vn.clear()
   vn.scene()
   local mm = vn.newCharacter( _("Map Maker"), { image = npc_image } )
   vn.transition()

   if met then
      vn.na(_([[You approach the Map Maker, who, like always, is fidgeting with a small portable holographic display. However, they quickly notice your presence.]]))
      mm(_([["Hey, you again! Would you be willing to help me with my maps?"]]))
   else
      vn.na(_([[You approach the figure, they don't seem to notice.]]))
      vn.menu{
         { _([[Say something.]]), "notice" },
         { _([[Wait until they notice.]]), "cont01" },
      }

      vn.label("cont01")
      vn.na(_([[They keep on fidgeting with a small portable holographic display, oblivious to your presence.]]))
      vn.menu{
         { _([[Cough loudly.]]), "notice" },
         { _([[Wait more.]]), "cont02" },
      }

      vn.label("cont02")
      vn.na(_([[They slouch further, completely engrossed in the device.]]))
      vn.menu{
         { _([[Tap your foot.]]), "notice" },
         { _([[Continue waiting.]]), "cont03" },
      }

      vn.label("cont03")
      vn.na(_([[Space time seems to come to a stop as they keep on fidgeting with their holographic device.]]))
      vn.menu{
         { _([[Wave.]]), "notice" },
         { _([[Wait a bit more.]]), "final_notice" },
      }

      vn.label("final_notice")
      vn.na(_([[They look up right at you, and then go back to the device. After a few seconds they look up again, this time actually seeming to notice you.]]))
      mm(_([["EEP!"
They almost drop the holographic device, and clutch at their chest.
"Crikes! Almost had my heart jump out my mouth."]]))
      vn.jump("cont04")

      vn.label("notice")
      vn.na(_([[They seem to not notice, making you believe you failed to grab their attention when they suddenly snap to you with a delayed response.]]))
      mm(_([["Eep! How did you sneak up on me so fast?"]]))
      vn.jump("cont04")

      vn.label("cont04")
      mm(_([["Hey, wait."
They squint at you and adjust their glasses.
"You look like a ship pilot!"]]))
      mm(_([["My name is Angel, but everyone calls me Map Maker. I've been working on updating the space maps, but you'd be surprised at what inconsistencies there are between the official Imperial maps from the Great Houses."]]))
      vn.func( function ()
         var.push("mapmaker_met",true)
      end )
      mm(_([["It's like they don't care at all about getting it correct! Obviously, something has to be done about this."]]))
      mm(_([["Would you be willing to help me properly map things once and for all?"]]))
   end
   vn.menu{
      { _([[Accept]]), "accept" },
      { _([[Refuse]]), "refuse" },
   }

   vn.label("refuse")
   vn.na(_([[You mention that you are busy and take your leave to much of the Map Maker's chagrin.]]))
   -- TODO make make them disappear a while if player refuses
   vn.done()

   vn.label("accept")
   mm(fmt.f(_([["OK! I've marked some locations on your map. All you have to do is double check all the jumps, planets, and stations in the vicinity, at least {numjumps} jumps from the marked system. I'll hook it up so your ship automatically forwards the information to me."]]),
      {numjumps=5}))
   mm(_([["You can see if a system is fully explored on your map by setting the #bDiscovery#0 mode. If it has a green circle in the middle, it should be all set and you can look at other systems."]]))
   mm(_([["It's going to be awesome! I know it. I'll get in touch with you as I process the information. Now I have to go back to my workplace."
They can't hold their excitement and do a little jig as they wonder off.]]))
   vn.func(function()
      accepted = true
   end)
   vn.run()

   if not accepted then return end

   misn.accept()
   misn.osdSetHide( true ) -- Hide OSD by default
   misn.setTitle( mission_name )
   misn.setReward(_("Power, and by that I mean Knowledge!"))
   update_markers()

   hook.discover("discover")
   hook.land("land")
   hook.load("load")
end

function discover ()
   update_markers()
end

function load ()
   -- Just in case, also updates status
   update_markers()
end

local rewards = {}
local rewardsys = nil
local rewardnpc
function land ()
   rewards = {}
   rewardsys = nil

   -- Must land on a "normal" spob
   local spb = spob.cur()
   local fct = spb:faction()
   if fct==nil or (not fct:tags().generic) then
      return
   end

   -- See if there is any reward that can be given
   for k,t in ipairs(targets) do
      local todiscover, discovered = explored( t.centre, t.distance or 5 )
      local progress = discovered / todiscover
      local lastprog = 0
      -- Require knowing the focused system
      if t.centre:known() then
         for th,rwd in pairs(t.rewards) do
            -- See if we can give rewards
            if th > lastprog and th < progress then
               for i,jmp in ipairs(rwd) do
                  if not jmp:known() and (jmp:dest():known() or jmp:system():known()) then
                     -- Don't add an equivalent jump twice
                     local found = false
                     for j,r in ipairs(rewards) do
                        if jmp==r or jmp==r:reverse() then
                           found = true
                           break
                        end
                     end
                     if not found then
                        table.insert( rewards, jmp )
                     end
                  end
               end
            end
         end
         -- We'll only do one reward for one area at a time
         if #rewards > 0 then
            rewardsys = t.centre
            break
         end
      end
   end

   -- No new rewards, so ignore
   if #rewards<=0 then return end

   local npc_hologram = love_shaders.shaderimage2canvas( love_shaders.hologram(), lg.newImage("gfx/portraits/"..npc_image) )
   rewardnpc = misn.npcAdd( "approach_mm", _("Map Maker"), npc_hologram.t.tex, _("You are receiving a transmission from the Map Maker.") )
end

local msg_double = {
   _([["I've found a hidden jump between the {src} and {dst} systems!"]]),
   _([["A hidden route seems to exist between the {src} and {dst} systems!"]]),
   _([["Marvellously, the {src} and {dst} systems seem to be connected by a hidden jump route!"]]),
   _([["Against prior knowledge, it seems like the {src} and {dst} systems are connected!"]]),
   _([["Data analysis proves that there has to be a hidden jump between the {src} and {dst} systems!"]]),
}
local msg_single = {
   _([["I've found a hidden jump in the {sys} system!"]]),
   _([["There seems to be a hidden route from the {sys} system!"]]),
   _([["It seems like the {sys} system is connected to somewhere with a hidden jump!"]]),
   _([["I would have never guessed, but the {sys} system seems to have an outgoing hidden jump!"]]),
   _([["Analysis seems to indicate that the {sys} system has a hidden jump to somewhere!"]]),
}

function approach_mm ()
   vn.clear()
   vn.scene()
   local mm = vn.newCharacter( _("Map Maker"), {
      image = npc_image,
      shader = love_shaders.hologram(),
   } )
   vn.transition("electric")

   mm(fmt.f(_([["Hey, {playername}! I've been tracking the data you sent me, and it looks like there are some interesting hyperspace abnormalities in the data near {sysname}."]]),
      {playername=player.name(), sysname=rewardsys}))
   shiplog.create( "mapmaker", _("Miscellaneous"), _("Map Maker") )
   for i,jmp in ipairs(rewards) do
      local dst = jmp:dest()
      local src = jmp:system()
      local knowdst = dst:known()
      local knowsrc = src:known()
      if knowdst and knowsrc then
         mm(fmt.f( msg_double[ rnd.rnd(1,#msg_double) ], {src=src, dst=dst}))
         shiplog.append( "mapmaker", fmt.f(_("You discovered a hidden jump between the {src} and {dst} systems."), {src=src, dst=dst}))
      elseif knowdst then
         mm(fmt.f( msg_single[ rnd.rnd(1,#msg_single) ], {sys=dst}))
         shiplog.append( "mapmaker", fmt.f(_("You discovered a hidden jump in the {sys} system."), {sys=dst}))
      elseif knowsrc then
         mm(fmt.f( msg_single[ rnd.rnd(1,#msg_single) ], {sys=src}))
         shiplog.append( "mapmaker", fmt.f(_("You discovered a hidden jump in the {sys} system."), {sys=src}))
      end
      jmp:setKnown(true)
   end
   vn.sfxVictory()
   mm(_([["I've updated your navigation system with the new information! You should check it out!"]]))
   mm(_([["I have to get back to my surveying, I'll catch up with you later!"]]))

   vn.done("electric")
   vn.run()

   misn.npcRm( rewardnpc )
end
