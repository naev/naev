--[[
<?xml version='1.0' encoding='utf8'?>
<event name="Bioship Manager">
 <location>load</location>
 <chance>100</chance>
 <unique />
</event>
--]]
local fmt = require "format"
local bioship = require "bioship"
local textoverlay = require "textoverlay"
local audio = require 'love.audio'

local function bioship_click ()
   bioship.window()
   update_bioship()
end

local infobtn, canrankup
function update_bioship ()
   local is_bioship = bioship.playerisbioship()

   -- Easier to always reset
   if infobtn then
      player.infoButtonUnregister( infobtn )
      infobtn = nil
   end

   -- Enable info window button based on bioship status
   if is_bioship then
      local pp = player.pilot()
      -- Indicate to equipopt that we'll be handling the stages
      pp:shipvarPush("bioship_init",true)
      local stage = pp:shipvarPeek("biostage")
      -- Initialize in the case stage isn't set
      if not stage then
         bioship.setstage( pp, 1 )
      end
      local caption = _("Bioship")
      if bioship.skillpointsfree(pp) > 0 then
         caption = caption .. "#r" .. _(" !!") .. "#0"
      end
      infobtn = player.infoButtonRegister( caption, bioship_click )
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
         local q = player.outfitNum( o, true )
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
   if not bioship.playerisbioship() then return end

   canrankup = nil

   -- Check for stage up!
   local pp = player.pilot()
   local exp = pp:shipvarPeek("bioshipexp") or 0
   local stage = pp:shipvarPeek("biostage")
   local maxstage = bioship.maxstage( player.pilot() )
   local expstage = bioship.curstage( exp, maxstage )
   if expstage >= stage then
      bioship.setstage( pp, expstage )
   end
end

local sfx
function bioship_pay( amount, _reason )
   if amount < 0 then return end
   if not bioship.playerisbioship() then return end

   local pp = player.pilot()
   local stage = pp:shipvarPeek("biostage")
   local maxstage = bioship.maxstage( player.pilot() )
   -- Already max level
   if stage >= maxstage then
      return
   end
   local exp = pp:shipvarPeek("bioshipexp") or 0
   -- Enough exp for max level
   if bioship.curstage( exp, maxstage ) >= maxstage then
      return
   end

   -- Add exp
   local BREAKPOINT = 500e3
   if amount > BREAKPOINT then
      local div = BREAKPOINT/1e3
      exp = exp + math.floor( div / math.sqrt(div) * math.sqrt(amount / 1e3) + 0.5 )
   else
      exp = exp + math.floor( amount / 1e3 )
   end
   exp = math.min( exp, bioship.exptostage( maxstage ) )
   pp:shipvarPush("bioshipexp",exp)

   -- Stage up!
   if exp >= bioship.exptostage( stage+1 ) and not canrankup then
      -- TODO sound and effect?
      canrankup = true
      player.msg("#g".._("Your bioship has enough experience to advance a stage!").."#0")
      textoverlay.init(_("Bioship Rank Up"), nil, { fadein = 1, fadeout = 1, length = 5 })
      if not sfx then
         sfx = audio.newSource( 'snd/sounds/jingles/victory.ogg' )
      end
      sfx:play()
      if player.isLanded() then
         bioship.setstage( pp, bioship.curstage( exp, maxstage ) )
      end
   end
end
