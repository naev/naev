--[[
-- This is the second mission in the crazy baron string.
--]]

-- localization stuff, translators would work here
lang = naev.lang()
if lang == "es" then
else -- default english
    title = {}
    text = {}
    osd_msg = {}
    misn_desc = {}
    
    title[1] = "His Baronship remembers you"
    text[1] = [[    As you approach the stranger, he extends his hand in greeting. He introduces himself as an associate of Baron Sauterfeldt, the man you helped "acquire" a holopainting not too long ago.
    "The Baron was quite pleased with your performance in that matter," he confides. "He has asked me to try to find you again for another job not unlike the last one. The Baron is a collector, you see, and his hunger for new possessions is a hard one to satiate." He makes a face. "Of course, his methods aren't always completely respectable, as you've experience for yourself. But I assure you that the Baron is not a bad man, he is simply very enthusiastic."
    You decide to keep your opinion of the fat aristocrat to yourself. Instead you inquire as to what the man wants from you this time. "To tell the truth, I don't actually know," the man says. "The Baron wants you to meet him so he can brief you in person. You will find his ship in the %s system. Shall I inform his lordship that you will be paying him a visit?"]]

    title[2] = "At your beck and call"
    text[2] = [[    "Splendid. Please go see his lordship at the earliest opportunity. He doesn't like to be kept waiting. I will send word that you will be coming, so contact the %s when you arrive at %s, and they will allow you to board."]]

    title[3] = "Green light for docking"
    text[3] = [[Your comm is answered by a communications officer on the bridge of the %s. You tell her you've got a delivery for the baron. She runs a few checks on a console off the screen, then tells you you've been cleared for docking and that the %s will be brought to a halt.]] 
    
    title[4] = "An audience with the Baron"
    text[4] = [[    You find yourself once again aboard the %s, Baron Sauterfeldt's flag ship. After a short time, an attendant ushers you into the Baron's personal quarters, which are as extravagant as you remember them. You notice the holopainting is now firmly fixed on one of the walls.
    Baron Dovai Sauterfeldt greets you with a pompous wave of his hand. "Ahh yes, there you are at last. %s, was it? Do have a seat."]]
    
    refusetitle = "Sorry, not today"
    refusetext = [[    "Okay, fair enough," the man says with a disappointed look on his face. "I'll try to find someone else. But maybe we'll run into each other again, so if you change your mind..."]]
    
    -- Mission details
    misn_title = "Prince"
    misn_reward = "A tidy sum of money"
    misn_desc[1] = "Baron Sauterfeldt has summoned you to his ship, which is in the %s system."
    misn_desc[2] = "Baron Sauterfeldt has tasked you with finding an ancient artifact, but he doesn't know exactly where it is."
    
    credits = 80000 -- 80K

    -- NPC stuff
    npc_desc = "An unfamiliar man"
    bar_desc = "A man you've never seen before makes eye contact with you. It seems he knows who you are."
    
    -- OSD stuff
    osd_msg[1] = "Meet Baron Sauterfeldt in the %s system"
    osd_msg[2] = "Recover the artifact"
end

function create ()
    misn.setNPC(npc_desc, "thief2")
    misn.setDesc(bar_desc)
end

function accept()
    shipname = "Pinnacle" 
    baronsys = system.get("Ingot")

    dest = {}
    dest[1] = planet.get("Varaati")
    dest[2] = planet.get("Arck")
    dest[3] = planet.get("Hurada")
    dest["__save"] = true
    flintloc = planet.get("Tau Prime")

    stage = 1
    
    if tk.yesno(title[1], string.format(text[1], baronsys:name())) then
        tk.msg(title[2], string.format(text[2], shipname, baronsys:name()))

        misn.setTitle(misn_title)
        misn.setReward(misn_reward)
        misn.setDesc(string.format(misn_desc[1], baronsys:name()))
        misn.osdCreate(misn_title[1], { string.format(osd_msg[1], baronsys:name())
                                      })
        misn.setMarker(baronsys, "misc")
        misn.accept()
        
        jumpinhook = hook.jumpin("jumpin")
    else
        tk.msg(refusetitle, refusetext)
        abort()
    end
end

function board()
    if stage == 1 then
        misn.setMarker()
        tk.msg(title[4], string.format(text[4], shipname, mangle(player.name())))
        misn.osdCreate(misn_title[1], { string.format(osd_msg[1], baronsys:name()),
                                        osd_msg[2]
                                      })
        misn.setDesc(misn_desc[2])
        misn.osdActive(2)
        
        -- TODO: needs better mission marker system! All three markers should be displayed at the same time.
        misn.setMarker(dest[1][2], "misc")
        misn.setMarker(dest[2][2], "misc")
        misn.setMarker(dest[3][2], "misc")
        
        hook.land("land")
        player.unboard()
        pinnacle:setHealth(100,100)
        pinnacle:control(false)
    elseif stage == 2 then
        -- Mission finish stuff here
        player.unboard()
        pinnacle:setHealth(100,100)
        pinnacle:control(false)
        mission.finish(true)
    else -- Should never happen!
        player.unboard()
        print(STDERR, "baron_prince: reached invalid else clause! stage = " .. stage)
    end
end

function land()
    if system.cur() == dest[1][1] or system.cur() == dest[2][1] or system.cur() == dest[3][1] then
        -- Artifact world
    elseif system.cur() == flintloc[1] then
        -- Flintley's world
    end
end

function jumpin()
    if sys.cur() == baronsys then
        pinnacle = pilot.add("Proteron Kahan", "trader", planet.get("Ulios"):pos() + vec2.new(-400,-400))[1]
        pinnacle:setFaction("Civilian")
        pinnacle:rename(shipname)
        pinnacle:setInvincible(true)
        pinnacle:control()
        pinnacle:goto(planet.get("Ulios"):pos() + vec2.new( 400, -400), false)
        hook.pilot(pinnacle, "idle", "idle")
        hook.pilot(pinnacle, "hail", "hail")
    end
end

function idle()
    if stopping then
        pinnacle:disable()
    else
        pinnacle:goto(planet.get("Ulios"):pos() + vec2.new( 400,  400), false)
        pinnacle:goto(planet.get("Ulios"):pos() + vec2.new(-400,  400), false)
        pinnacle:goto(planet.get("Ulios"):pos() + vec2.new(-400, -400), false)
        pinnacle:goto(planet.get("Ulios"):pos() + vec2.new( 400, -400), false)
    end
end

function hail()
    tk.msg(title[3], string.format(text[3], shipname, shipname))
    pinnacle:taskClear()
    pinnacle:brake()
    stopping = true
    boardhook = hook.board("board")
end

-- Function that tries to misspell whatever is passed to it.
function mangle(intext)
    local outtext = intext
    
    local consonants = {"b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "v", "w", "x", "z"}
    local vowels = {"a", "e", "i", "o", "u"} -- Not y though.
    
    local first = intext:sub(1, 1):lower()
    local second = intext:sub(2, 2)
    
    if isIn(first, consonants) and isIn(second, vowels) then
        first = consonants[rnd.rnd(1, #consonants)]:upper()
        second = vowels[rnd.rnd(1, #vowels)]
        outtext = first .. second .. intext:sub(2)
    elseif isIn(first, consonants) and isIn(second, vowels) then
        first = vowels[rnd.rnd(1, #vowels)]:upper()
        second = consonants[rnd.rnd(1, #consonants)]
        outtext = first .. second .. intext:sub(2)
    elseif isIn(first, consonants) and isIn(second, consonants) then
        first = consonants[rnd.rnd(1, #consonants)]:upper()
        second = consonants[rnd.rnd(1, #consonants)]
        outtext = first .. second .. intext:sub(2)
    elseif isIn(first, vowels) and isIn(second, vowels) then
        first = vowels[rnd.rnd(1, #vowels)]:upper()
        second = vowels[rnd.rnd(1, #vowels)]
        outtext = first .. second .. intext:sub(2)
    end
    
    return outtext
end

function isIn(char, table)
    for _, j in pairs(table) do
        if j == char then return true end
    end
    return false
end

function abort()
    misn.finish(false)
end