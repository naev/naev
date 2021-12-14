--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Bioship Manager">
 <trigger>load</trigger>
 <chance>100</chance>
 <flags>
  <unique />
 </flags>
</event>
--]]
local fmt = require "format"
local bioship = require "bioship"

-- luacheck: globals update_bioship bioship_pay bioship_land (Hook functions passed by name)

local function playerisbioship ()
   return player.pilot():ship():tags().bioship
end

local infobtn
function update_bioship ()
   local is_bioship = playerisbioship()
   local stage = player.shipvarPeek("biostage")
   -- Initialize in the case stage isn't set
   if not stage then
      bioship.setstage( 1 )
      stage = 1
   end

   -- Enable info window button based on bioship status
   if is_bioship then
      if not infobtn then
         local sp = stage - bioship.skillpointsused()
         local caption = _("Bioship")
         if sp > 0 then
            caption = caption .. _(" #r!#0")
         end
         infobtn = player.infoButtonRegister( caption, bioship.window )
      end
   else
      if infobtn then
         player.infoButtonUnregister( infobtn )
         infobtn = nil
      end
   end
end

local remove_types = {
   ["Bioship Organ"] = true,
   ["Bioship Brain"] = true,
   ["Bioship Gene Drive"] = true,
   ["Bioship Shell"] = true,
   ["Bioship Weapon Organ"] = true,
}
function create ()
   -- Try to clean up outfits
   for k,o in ipairs(player.outfits()) do
      if remove_types[ o:type() ] then
         local q = player.numOutfit( o, true )
         player.outfitRm( o, q )
         print(fmt.f(_("Removing unobtainable Bioship outfit '{outfit}'!"),{outfit=o}))
      end
   end

   -- Try update
   update_bioship()

   hook.ship_swap( "update_bioship" )
   hook.pay( "bioship_pay" )
   hook.land( "bioship_land" )
end

local function expstage ()
   if not playerisbioship() then return 0 end

   local exp = player.shipvarPeek("bioshipexp") or 0
   local maxstage = bioship.maxstage( player.pilot() )

   local curstage
   local nextlevel = 100
   for i=1,maxstage do
      curstage = i
      nextlevel = nextlevel * math.pow(1.5,i)
      if exp < nextlevel then
         break
      end
   end
   return curstage
end

function bioship_land ()
   if not playerisbioship() then return end

   local curstage = player.shipvarPeek("biostage")
   local stage = expstage()
   if stage > curstage then
      bioship.setstage( stage )
   end
end

function bioship_pay( amount, _reason )
   if amount < 0 then return end
   if not playerisbioship() then return end

   local stage = player.shipvarPeek("biostage")
   -- Max level
   if stage >= bioship.maxstage( player.pilot() ) then
      return
   end

   local exp = player.shipvarPeek("bioshipexp") or 0
   exp = exp + math.floor(amount / 10e3)
   player.shipvarPush("bioshipexp",exp)

   local nextstage = expstage()

   -- Level up!
   if nextstage > stage then
      -- TODO sound and effect?
      player.msg(_("#gYour bioship has advanced a stage!#0"))
      if player.isLanded() then
         bioship.setstage( stage+1 )
      end
   end
end
