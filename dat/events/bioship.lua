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
local bioskills= require "bioship.skills"

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
      else
         -- In case the player's ship got messed up, we reset intrinsics
         bioship.setstage( pp, stage )
      end
      local caption = _("Bioship")
      if bioship.skillpointsfree(pp) > 0 then
         caption = caption .. "#r" .. _(" !!") .. "#0"
      end
      infobtn = player.infoButtonRegister( caption, bioship_click )

      -- Some changes can cause the bioship outfits to break (shouldn't anymore
      -- with slotname saving though), so restore skills if necessary.
      -- In particular, this was triggered by some of the 0.13.0-alphaX
      -- pre-releases because they added new slots that would displace the
      -- bioship skills and cause them not to equip.
      -- TODO maybe move this into the updater script or an onload script
      local playerskills ={}
      for i,set in pairs(bioskills.set) do
         for j,skill in pairs(set) do
            local svar = "bio_"..j
            if pp:shipvarPeek( svar ) then
               table.insert( playerskills, skill )
            end
            -- Clear everything for now
            bioship.skill_disable( pp, skill )
         end
      end
      -- Sort to make sure it is OK
      table.sort( playerskills, function( a, b )
         return a.tier < b.tier
      end )
      for k,skill in ipairs(playerskills) do
         bioship.skill_enable( pp, skill )
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
   local soromid = (player.pilot():ship():faction()==faction.get("Soromid"))
   local stage = pp:shipvarPeek("biostage")
   local maxstage = bioship.maxstage( player.pilot() )

   -- Already max level
   if stage >= maxstage then
      return
   end

   local factor
   if soromid then
      factor=1.0
   else
      factor=0.5
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
      exp = exp + math.floor( factor * div / math.sqrt(div) * math.sqrt(amount / 1e3) + 0.5 )
   else
      exp = exp + math.floor( factor * amount / 1e3 )
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
         sfx = audio.newSource( 'snd/sounds/jingles/victory' )
      end
      sfx:play()
      if player.isLanded() then
         bioship.setstage( pp, bioship.curstage( exp, maxstage ) )
      end
   end
end
