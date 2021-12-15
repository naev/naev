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

   -- Enable info window button based on bioship status
   if is_bioship then
      local stage = player.shipvarPeek("biostage")
      -- Initialize in the case stage isn't set
      if not stage then
         bioship.setstage( 1 )
         stage = 1
      end
      if not infobtn then
         local sp = stage - bioship.skillpointsused()
         local caption = _("Bioship")
         if sp > 0 then
            caption = caption .. _(" #r!!#0")
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

function bioship_land ()
   if not playerisbioship() then return end

   -- Check for stage up!
   local exp = player.shipvarPeek("bioshipexp") or 0
   local stage = player.shipvarPeek("biostage")
   if exp >= bioship.exptostage( stage+1 ) then
      bioship.setstage( stage+1 )
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
   exp = exp + math.floor(amount / 1e3)
   player.shipvarPush("bioshipexp",exp)

   -- Stage up!
   if exp >= bioship.exptostage( stage+1 ) then
      -- TODO sound and effect?
      player.msg(_("#gYour bioship has enough experience to advance a stage!#0"))
      if player.isLanded() then
         bioship.setstage( stage+1 )
      end
   end
end
