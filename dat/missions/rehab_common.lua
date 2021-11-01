--[[

   Rehabilitation Mission Framework

   This mission allows you to remain neutral with a faction until you've done services for them.
   This file is used by the various faction missions, which must set the faction variable.

--]]
local fmt   = require "format"
local vntk  = require 'vntk'

function create()
    -- Note: this mission does not make any system claims.

    -- Only spawn this mission if the player needs it.
    rep = fac:playerStanding()
    if rep >= 0 then
       misn.finish()
    end

    -- Don't spawn this mission if the player is buddies with this faction's enemies.
    for _k, enemy in pairs(fac:enemies()) do
       if enemy:playerStanding() > 20 then
          misn.finish()
       end
    end

    setFine(rep)

    misn.setTitle(fmt.f(_("{fct} Rehabilitation"), {fct=fac}))
    misn.setDesc(fmt.f(_([[You may pay a fine for a chance to redeem yourself in the eyes of a faction you have offended. You may interact with this faction as if your reputation were neutral, but your reputation will not actually improve until you've regained their trust. ANY hostile action against this faction will immediately void this agreement.
Faction: {fct}
Cost: {credits}]]), {fct=fac, credits=fmt.credits(fine)}))
    misn.setReward(_("None"))
end

function accept()
    if player.credits() < fine then
        vntk.msg("", fmt.f(_("You don't have enough money. You need at least {credits} to buy a cessation of hostilities with this faction."), {credits=fmt.credits(fine)}))
        misn.finish()
    end

    player.pay(-fine)
    vntk.msg(fmt.f(_("{fct} Rehabilitation"), {fct=fac}), fmt.f(_([[Your application has been processed. The {fct} security forces will no longer attack you on sight. You may conduct your business in {fct} space again, but remember that you still have a criminal record! If you attack any traders, civilians or {fct} ships, or commit any other felony against this faction, you will immediately become their enemy again.
While this agreement is active your reputation will not change, but if you continue to behave properly and perform beneficial services, your past offenses will eventually be stricken from the record.]]), {fct=fac}))

    fac:modPlayerRaw(-rep)

    misn.accept()
    local osd_msg = { n_(
       "You need to gain %d more reputation",
       "You need to gain %d more reputation",
       -rep
    ):format(-rep) }
    misn.osdCreate(fmt.f(_("{fct} Rehabilitation"), {fct=fac}), osd_msg)

    standhook = hook.standing("standing")

    excess = 5 -- The maximum amount of reputation the player can LOSE before the contract is void.
end

-- Function to set the height of the fine. Missions that require this script may override this.
function setFine(standing)
    fine = (-standing)^2 * 1000 -- A value between 0 and 10M credits
end

-- Standing hook. Manages faction reputation, keeping it at 0 until it goes positive.
function standing(hookfac, delta)
    if hookfac == fac then
        if delta >= 0 then
            rep = rep + delta
            if rep >= 0 then
                -- The player has successfully erased his criminal record.
                excess = excess + delta
                fac:modPlayerRaw(-delta + rep)
                vntk.msg(fmt.f(_("{fct} Rehabilitation Successful"), {fct=fac}), _([[Congratulations, you have successfully worked your way back into good standing with this faction. Try not to relapse into your life of crime!]]))
                misn.finish(true)
            end

            excess = excess + delta
            fac:modPlayerRaw(-delta)
            local osd_msg = { n_(
               "You need to gain %d more reputation",
               "You need to gain %d more reputation",
               -rep
            ):format(-rep) }
            misn.osdCreate(fmt.f(_("{fct} Rehabilitation"), {fct=fac}), osd_msg)
        else
            excess = excess + delta
            if excess < 0 or fac:playerStanding() < 0 then
                abort()
            end
        end
    end
end

-- On abort, reset reputation.
function abort()
    -- Remove the standing hook prior to modifying reputation.
    hook.rm(standhook)

    -- Reapply the original negative reputation.
    fac:modPlayerRaw(rep)

    vntk.msg(fmt.f(_("{fct} Rehabilitation Canceled"), {fct=fac}), _([[You have committed another offense against this faction! Your rehabilitation procedure has been canceled, and your reputation is once again bad. You may start another rehabilitation procedure at a later time.]]))
    misn.finish(false)
end
