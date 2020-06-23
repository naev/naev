--[[
--
-- Rehabilitation Mission
--
-- This mission allows you to remain neutral with a faction until you've done services for them.
-- This file is used by the various faction missions, which must set the faction variable.
--
--]]

require "dat/scripts/numstring.lua"


misn_title = _("%s Rehabilitation")
misn_desc = _([[You may pay a fine for a chance to redeem yourself in the eyes of a faction you have offended. You may interact with this faction as if your reputation were neutral, but your reputation will not actually improve until you've regained their trust. ANY hostile action against this faction will immediately void this agreement.
Faction: %s
Cost: %s credits]])
misn_reward = _("None")

lowmoney = _("You don't have enough money. You need at least %s credits to buy a cessation of hostilities with this faction.")
accepted = _([[Your application has been processed. The %s security forces will no longer attack you on sight. You may conduct your business in %s space again, but remember that you still have a criminal record! If you attack any traders, civilians or %s ships, or commit any other felony against this faction, you will immediately become their enemy again.
While this agreement is active your reputation will not change, but if you continue to behave properly and perform beneficial services, your past offenses will eventually be stricken from the record.]])

failuretitle = _("%s Rehabilitation Canceled")
failuretext = _([[You have committed another offense against this faction! Your rehabilitation procedure has been canceled, and your reputation is once again bad. You may start another rehabilitation procedure at a later time.]])

successtitle = _("%s Rehabilitation Successful")
successtext = _([[Congratulations, you have successfully worked your way back into good standing with this faction. Try not to relapse into your life of crime!]])

osd_msg = {}
osd_msg[1] = _("You need to gain %d more reputation")
osd_msg1 = _("You need to gain %d more reputation")
osd_msg["__save"] = true


function create()
    -- Note: this mission does not make any system claims.

    -- Only spawn this mission if the player needs it.
    rep = fac:playerStanding()
    if rep >= 0 then
       misn.finish()
    end
    
    -- Don't spawn this mission if the player is buddies with this faction's enemies.
    for _, enemy in pairs(fac:enemies()) do
       if enemy:playerStanding() > 20 then
          misn.finish()
       end
    end

    setFine(rep)
    
    misn.setTitle(misn_title:format(fac:name()))
    misn.setDesc(misn_desc:format(fac:name(), numstring(fine)))
    misn.setReward(misn_reward)
end

function accept()
    if player.credits() < fine then
        tk.msg("", lowmoney:format(numstring(fine)))
        misn.finish()
    end
    
    player.pay(-fine)
    tk.msg(misn_title:format(fac:name()), accepted:format(fac:name(), fac:name(), fac:name()))
    
    fac:modPlayerRaw(-rep)
    
    misn.accept()
    osd_msg[1] = osd_msg1:format(-rep)
    misn.osdCreate(misn_title:format(fac:name()), osd_msg)
    
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
                tk.msg(successtitle:format(fac:name()), successtext)
                misn.finish(true)
            end

            excess = excess + delta
            fac:modPlayerRaw(-delta)
            osd_msg[1] = osd_msg1:format(-rep)
            misn.osdCreate(misn_title:format(fac:name()), osd_msg)
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

    tk.msg(failuretitle:format(fac:name()), failuretext)
    misn.finish(false)
end
