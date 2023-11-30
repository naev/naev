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
   local txtaccept = params.txtaccept or {fmt.f(_([[Your application has been processed. The {fct} security forces will no longer attack you on sight. You may conduct your business in {fct} space again, but remember that you still have a criminal record! If you attack any traders, civilians or {fct} ships, or commit any other felony against this faction, you will immediately become their enemy again.]]),
      {fct=fct}),
   _([[While this agreement is active, your reputation will not change, but if you continue to behave properly and perform beneficial services, your past offenses will eventually be stricken from the record.]]) }
   local txtsuccess = params.txtsuccess or _([[Congratulations, you have successfully worked your way back into good standing with this faction. Try not to relapse into your life of crime!]])
   local txtabort = params.txtabort or _([[You have committed another offense against this faction! Your rehabilitation procedure has been canceled, and your reputation is once again tarnished. You may start another rehabilitation procedure at a later time.]])

   function create()
      -- Note: this mission does not make any system claims.

      -- Only spawn this mission if the player needs it.
      mem.rep = fct:playerStanding()
      if mem.rep >= 0 then
         misn.finish()
      end

      -- Don't spawn this mission if the player is buddies with this faction's enemies.
      for _k, enemy in pairs(fct:enemies()) do
         if enemy:playerStanding() > 20 then
         misn.finish()
         end
      end

      setFine( mem.rep )

      misn.setTitle(prefix.prefix(fct)..fmt.f(_("{fct} Rehabilitation"), {fct=fct}))
      local stdval, stdname = fct:playerStanding()
      misn.setDesc(desc.."\n\n"..fmt.f(_([[#nFaction:#0 {fct}
#nCost:#0 {credits}
#nCurrent Standing:#0 #r{standingname} ({standingvalue})#0]]),
         {fct=fct:longname(), credits=fmt.credits(mem.fine), standingname=stdname, standingvalue=stdval}))
      misn.setReward(_("None"))
   end

   local function setosd ()
      local osd_msg = { n_(
         "You need to gain %d more reputation",
         "You need to gain %d more reputation",
         -mem.rep
      ):format(-mem.rep) }
      misn.osdCreate(fmt.f(_("{fct} Rehabilitation"), {fct=fct}), osd_msg)
   end

   function accept()
      if player.credits() < mem.fine then
         vntk.msg("", fmt.f(txtbroke,
            {credits=fmt.credits(mem.fine)}))
         misn.finish()
      end

      player.pay(-mem.fine)
      vntk.msg(fmt.f(_("{fct} Rehabilitation"), {fct=fct}), txtaccept )

      fct:modPlayerRaw(-mem.rep)

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
   function standing( hookfac, delta )
      if hookfac == fct then
         if delta >= 0 then
            -- Be ware of the fake transponder!
            local pp = player.pilot()
            local hasft = false
            local ft = outfit.get("Fake Transponder")
            for k,v in ipairs(pp:outfitsList()) do
               if v==ft then
               hasft = true
               break
               end
            end
            if hasft then return end -- When fake transponder is equipped, we ignore all positive stuff

            mem.rep = mem.rep + delta
            if mem.rep >= 0 then
               -- The player has successfully erased his criminal record.
               mem.excess = mem.excess + delta
               fct:modPlayerRaw(-delta + mem.rep)
               vntk.msg(fmt.f(_("{fct} Rehabilitation Successful"), {fct=fct}), txtsuccess )
               misn.finish(true)
            end

            mem.excess = mem.excess + delta
            fct:modPlayerRaw(-delta)
            setosd()
         else
            mem.excess = mem.excess + delta
            if mem.excess < 0 or fct:playerStanding() < 0 then
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
      fct:modPlayerRaw(mem.rep)

      vntk.msg(fmt.f(_("{fct} Rehabilitation Canceled"), {fct=fct}), txtabort )
      misn.finish(false)
   end
end
return rehab
