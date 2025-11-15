--[[

   Rehabilitation Mission Framework

   This mission allows you to remain neutral with a faction until you've done services for them.
   This file is used by the various faction missions, which must set the faction variable.

--]]
local fmt   = require "format"
local vntk  = require 'vntk'
local prefix = require "common.prefix"

-- luacheck: globals create accept abort (used by missions)

local rehab = {}
function rehab.init( fct, params )
   local setFine -- Forward-declared functions
   local desc = params.desc or _([[You may pay a fine for a chance to redeem yourself in the eyes of a faction you have offended. You may interact with this faction as if your reputation were neutral, but your reputation will not actually improve until you've regained their trust. ANY hostile action against this faction will immediately void this agreement.]])
   local txtbroke = params.txtbroke or _("You don't have enough money. You need at least {credits} to buy a cessation of hostilities with this faction.")
   local txtaccept = params.txtaccept or fmt.f(_([[Your application has been processed. The {fct} security forces will no longer attack you on sight. You may conduct your business in {fct} space again, but remember that you still have a criminal record! If you attack any traders, civilians or {fct} ships, or commit any other felony against this faction, you will immediately become their enemy again.]]),
      {fct=fct})
   local txtsuccess = params.txtsuccess or _([[Congratulations, you have successfully worked your way back into good standing with this faction. Try not to relapse into your life of crime!]])
   local txtabort = params.txtabort or _([[You have committed another offence against this faction! Your rehabilitation procedure has been cancelled, and your reputation is once again tarnished. You may start another rehabilitation procedure at a later time.]])

   function create()
      -- Note: this mission does not make any system claims.

      -- Only spawn this mission if the player needs it.
      mem.rep = fct:reputationGlobal()
      if mem.rep >= 0 then
         misn.finish()
      end

      -- Don't spawn this mission if the player is buddies with this faction's enemies.
      for _k, enemy in pairs(fct:enemies()) do
         if enemy:reputationGlobal() > 20 then
            misn.finish()
         end
      end

      setFine( mem.rep )

      misn.setTitle(prefix.prefix(fct)..fmt.f(_("{fct} Rehabilitation"), {fct=fct}))
      local stdname = fct:reputationText( mem.rep )
      misn.setDesc(desc.."\n\n"..fmt.f(_([[#nFaction:#0 {fct}
#nCost:#0 {credits}
#nCurrent Standing:#0 #r{standingname} ({standingvalue})#0]]),
         {fct=fct:longname(), credits=fmt.credits(mem.fine), standingname=stdname, standingvalue=fmt.number(mem.rep)}))
      misn.setReward(_("None"))
   end

   local function setosd ()
      local r = math.ceil(-mem.rep)
      local osd_msg = { n_(
         "You need to gain %d more reputation",
         "You need to gain %d more reputation",
         r
      ):format(r) }
      misn.osdCreate(fmt.f(_("{fct} Rehabilitation"), {fct=fct}), osd_msg)
   end

   function accept()
      if player.credits() < mem.fine then
         vntk.msg("", fmt.f(txtbroke,
            {credits=fmt.credits(mem.fine)}))
         misn.finish()
      end

      player.pay(-mem.fine)
      vntk.msg(fmt.f(_("{fct} Rehabilitation"), {fct=fct}), {
         txtaccept,
         _([[While this agreement is active, your reputation will not change, but if you continue to behave properly and perform beneficial services, your past offences will eventually be stricken from the record.]])
      } )


      -- Store old standing
      mem.rep_global = fct:reputationGlobal()
      mem.rep_local = {}
      for k,s in ipairs(system.getAll()) do
         mem.rep_local[ s:nameRaw() ] = s:reputation( fct )
      end
      fct:setReputationGlobal( 0 ) -- Reset to 0

      misn.accept()
      setosd()

      mem.standhook = hook.standing("standing")

      mem.excess = 5 -- The maximum amount of reputation the player can LOSE before the contract is void.
   end

   -- Function to set the height of the fine. Missions that require this script may override this.
   function setFine(standing)
      mem.fine = (-standing)^2 * 1000 -- A value between 0 and 10M credits
   end

   -- Standing hook. Manages faction reputation, keeping it at 0 until it goes positive.
   function standing( hookfac, sys, delta, _source, _secondary, _primary_fct )
      if hookfac == fct then
         if delta >= 0 then
            -- No need for fake transponder stuff anymore, because it stops faction standing changes

            mem.rep = mem.rep + delta
            if mem.rep >= 0 then
               -- The player has successfully erased his criminal record.
               mem.excess = mem.excess + delta

               -- Just set to 0 for now
               for k,s in ipairs(system.getAll()) do
                  s:setReputation( fct, 0 )
               end

               vntk.msg(fmt.f(_("{fct} Rehabilitation Successful"), {fct=fct}), txtsuccess )
               misn.finish(true)
            end

            -- TODO restore all the local standings
            mem.excess = mem.excess + delta

            if sys then
               sys:setReputation( fct, 0 )
            else
               for k,s in ipairs(system.getAll()) do
                  s:setReputation( fct, 0 )
               end
            end

            setosd()
         else
            mem.excess = mem.excess + delta
            if mem.excess < 0 then
               abort()
            end
         end
      end
   end

   -- On abort, reset reputation.
   function abort()
      -- Have to remove hook first or applied infinitely
      hook.rm( mem.standhook )

      -- Reapply the original negative reputation.
      for k,s in ipairs(system.getAll()) do
         s:setReputation( fct, (mem.rep_local[ s:nameRaw() ] or mem.rep_global) )
      end
      vntk.msg(fmt.f(_("{fct} Rehabilitation Cancelled"), {fct=fct}), txtabort )
      misn.finish(false)
   end
end
return rehab
