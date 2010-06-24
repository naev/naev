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

    refusetitle = "Sorry, not today"
    refusetext = [[    "Okay, fair enough," the man says with a disappointed look on his face. "I'll try to find someone else. But maybe we'll run into each other again, so if you change your mind..."]]
    
    title[2] = "At your beck and call"
    text[2] = [[    "Splendid. Please go see his lordship at the earliest opportunity. He doesn't like to be kept waiting. I will send word that you will be coming, so contact the Pinnacle when you arrive at %s, and they will allow you to board."]]

    title[3] = "Green light for docking"
    text[3] = [[Your comm is answered by a communications officer on the bridge of the Pinnacle. You tell her you've got a delivery for the baron. She runs a few checks on a console off the screen, then tells you you've been cleared for docking and that the Pinnacle will be brought to a halt.]] 
    
    title[4] = "An audience with the Baron"
    text[4] = [[    You find yourself once again aboard the Pinnacle, Baron Sauterfeldt's flag ship. After a short time, an attendant ushers you into the Baron's personal quarters, which are as extravagant as you remember them. You notice the holopainting is now firmly fixed on one of the walls.
    Baron Dovai Sauterfeldt greets you with a pompous wave of his hand. "Ahh yes, there you are at last. %s, was it? Do have a seat." He then offers you a drink, but you decline on the basis that you still have to drive. "Now then, %s, I assume you're wondering why I've called you here. As you've no doubt heard, I have an interest in the unique, the exquisite." The Baron gestures around the room. "I have built up quite an impressive collection, as you can see, but it is still lacking something. Fortunately, news has reached me about a priceless artefact from Earth itself, dating back to before the Faction Wars. I must have it. It belongs in the hands of a connoiseur like myself."]]
    
    text[5] = [[    "Unfortunately, news of this artefact has reached more ears than just mine. All over the galaxy there are people who will try to sell you 'ancient artefacts', which always turn out to be imitations at best and worthless scrap they picked up from the streets at worst." The Baron snorts derisively. "Even the contacts who usually fenc- ah, I mean, supply me with new items for my collection are in on the frenzy.
    "I've narrowed down my search to three of these people. I'm confident that one of them is selling the genuine article, while the other two are shams. And this is where you come in, %s. I want you to visit these vendors, buy their wares off them and bring me the authentic artefact. You will have the help of a man named Flintley, who is a history buff or some such rot. You will find him on %s in the %s system. Simply tell him you're working for me and show him any artefacts in your possession. He will tell you which are authentic and which are fake.
    "I should warn you, %s. Some of my, ah, colleagues have also set their sights on this item, and so you can expect their henchmen to try to take it off you. I trust you are equipped to defend yourself against their despicable sort."]]
    
    title[5] = "Off to the shops"
    
    text[6] = [[    You are swiftly escorted back to your ship. You didn't really get the chance to ask the Baron any questions, such as who these potential attackers are, how you're supposed to pay for the artefacts once you locate the sellers, or what you will get out of all this. You do, however, find an update to your galaxy map that shows the location of the sellers, as well as a list of names and portraits. It would seem that the only way to find out what you're dealing with is the hard way.]]
    
    title[6] = "What are you doing here?"
    
    text[7] = [[You have not yet collected and identified the genuine artefact. Buy the artefacts from the sellers and visit Flintley on %s (%s) to identify the real one.]]
    
    title[7] = "Flintley, at your service"
    
    text[8] = [[    You approach the nervous looking man and inquire if he is Flintley, the historian in Baron Sauterfeldt's employ.
    "Oh, yes. Yes! That is me! I'm Flintley," the man responds. "And you must be %s. I know what's going on, the people from the Pinnacle have informed me. Oh, but where are my manners. Let me properly introduce myself. My name is Flintley, and I'm an archaeologist and historian. The best in the galaxy, some might say, ha-ha!" He gives you a look. "Well, maybe not. But I'm quite knowledgeable about the history of the galaxy. Too bad not too many people seem interested in that these days."
    Flintley sighs. "Well, that's that. Come to me with any artefacts you manage to procure, and I'll analyze them to the best of my ability."]]
    
    title[8] = "A suspicious salesman"
    
    text[9] = [[    "Hello there," the guy says to you when you approach. "Can I interest you in this bona fide relic from an ancient past? Unlike all those scammers out there, I offer you the real deal, no fakes here!"]]

    text[10] = [[    The man grins at you. "Ah, I can tell you have the eye of a connoiseur! I deal only in the finest, counterfeit-free antiques. If you're smart, and I can see that you are, you won't trust all those opportunists who will try to sell you fakes! How about it?"]]

    text[11] = [[    The man beckons you over to the bar. "Listen, friend. I have here a unique, extremely rare remnant of prehistoric times. This is the genuine article, trust me on that. One hundred per cent legit! And you wouldn't want to spend good credits on a fake, right?"]]

    title[9] = "This is not the artefact you're looking for"

    text[12] = [[    Miss!]]

    text[13] = [[    Miss!]]

    text[14] = [[    Miss!]]
    
    title[10] = "Time capsule"
    
    text[15] = [[    You am found it(s).]]
    
    title[11] = "The Baron has his prize"
    
    text[16] = [[    Jeee]]

    -- Mission details
    misn_title = "Prince"
    misn_reward = "You weren't told!"
    misn_desc[1] = "Baron Sauterfeldt has summoned you to his ship, which is in the %s system."
    misn_desc[2] = "Baron Sauterfeldt has tasked you with finding an ancient artefact, but he doesn't know exactly where to get it."
    
    credits = 100000 -- 100K

    -- NPC stuff
    npc_desc = "An unfamiliar man"
    bar_desc = "A man you've never seen before makes eye contact with you. It seems he knows who you are."
    
    flint_npc1 = "A reedy looking man"
    flint_bar1 = "You spot a thin, nervous looking individual. He does not seem to want to be here. This could be that Flintley fellow the Baron told you about."
    
    flint_npc2 = "Flintley"
    flint_bar2 = "Flintley is here. He nervously sips from his drink, clearly uncomfortable in this environment."
    
    sellerdesc = "You spot a dodgy individual who matches one of the portraits in your ship's database. This must be one of the artefact sellers."

    buy = "Buy the artefact\
(15,000 credits)"
    nobuy = "Don't buy the artefact"

    nomoneytitle = "Not enough money!"
    nomoneytext = "You can't currently afford to buy this artefact. You need 15,000 credits."
    
    -- OSD stuff
    osd_msg[1] = "Fly to the %s system and dock with (board) Kahan Pinnacle"
    osd_msg[2] = "Buy the artefact and take it to Flintley"
    osd_msg[3] = "Take the artefact to Baron Sauterfeldt"
end

function create ()
    misn.setNPC(npc_desc, "thief2")
    misn.setDesc(bar_desc)
end

function accept()
    baronsys = system.get("Ingot")

    dest = {}
    dest[1] = planet.get("Varaati")
    dest[2] = planet.get("Arck")
    dest[3] = planet.get("Hurada")
    dest["__save"] = true
    flintloc = planet.get("Tau Prime")

    stage = 1
    
    flintleyfirst = true
    artefactsfound = 0
    
    if tk.yesno(title[1], text[1]:format(baronsys:name())) then
        tk.msg(title[2], text[2]:format(baronsys:name()))

        misn.setTitle(misn_title)
        misn.setReward(misn_reward)
        misn.setDesc(misn_desc[1]:format(baronsys:name()))
        misn.osdCreate(misn_title[1], { osd_msg[1]:format(baronsys:name()),
                                      })
        misn.setMarker(baronsys, "misc")
        misn.accept()
        var.push("baron_active", true)
        
        jumpinhook = hook.jumpin("jumpin")
    else
        tk.msg(refusetitle, refusetext)
        abort()
    end
end

function board()
    if stage == 1 then
        misn.setMarker()
        tk.msg(title[4], text[4]:format(mangle(player.name()), mangle(player.name())))
        tk.msg(title[4], text[5]:format(mangle(player.name()), flintloc[1], flintloc[2], mangle(player.name())))
        misn.osdCreate(misn_title[1], { osd_msg[1]:format(baronsys:name()),
                                        osd_msg[2]
                                      })
        misn.setDesc(misn_desc[2])
        misn.osdActive(2)
        
        stage = 2
        
        -- TODO: needs better mission marker system! All three markers should be displayed at the same time.
        misn.setMarker(dest[1][2], "misc")
        misn.setMarker(dest[2][2], "misc")
        misn.setMarker(dest[3][2], "misc")
        
        hook.land("land")
        player.unboard()
        pinnacle:setHealth(100,100)
        stopping = false
        idle()
    elseif stage == 2 then
        tk.msg(title[6], text[7]:format(flintloc[1], flintloc[2]))
        player.unboard()
        pinnacle:setHealth(100,100)
        stopping = false
        idle()
    elseif stage == 3 then
        tk.msg(title[11], text[16])
        player.unboard()
        pinnacle:setHealth(100,100)
        pinnacle:control(false)
        var.pop("baron_active")
        mission.finish(true)
    else -- Should never happen!
        player.unboard()
        print(STDERR, "baron_prince: reached invalid else clause! stage = " .. stage)
        abort()
    end
end

function land()
    if system.cur() == dest[1][1]then
        sellnpc = misn.npcAdd("seller", "Artefact seller", thief1, sellerdesc, 4)
    elseif system.cur() == dest[2][1] then
        sellnpc = misn.npcAdd("seller", "Artefact seller", thief2, sellerdesc, 4)
    elseif system.cur() == dest[3][1] then
        sellnpc = misn.npcAdd("seller", "Artefact seller", thief3, sellerdesc, 4)
    elseif system.cur() == flintloc[1] then
        local bingo = false
        
        if flintleyfirst then
            flintleyfirst = false
            tk.msg(title[7], text[8]:format(player.name()))
        end
        
        if player.cargoHas("Artefact? A") == 0 then
            artefact1 = false
            if rnd.rnd(1, 3 - artefactsfound) == 1 then
                bingo = true
            else
                tk.msg(title[9], text[12])
                artefactsfound = artefactsfound + 1
            end
        end
        if player.cargoHas("Artefact? B") == 0 then
            artefact2 = false
            if rnd.rnd(1, 3 - artefactsfound) == 1 then
                bingo = true
            else
                tk.msg(title[9], text[13])
                artefactsfound = artefactsfound + 1
            end
        end
        if player.cargoHas("Artefact? C") == 0 then
            artefact3 = false
            if rnd.rnd(1, 3 - artefactsfound) == 1 then
                bingo = true
            else
                tk.msg(title[9], text[14])
                artefactsfound = artefactsfound + 1
            end
        end

        if bingo then
            tk.msg(title[10], text[15])
            misn.osdCreate(misn_title[1], { osd_msg[1]:format(baronsys:name()),
                                            osd_msg[2],
                                            osd_msg[3]
                                          })
            misn.osdActive(3)
            stage = 3
            -- TODO: clear mission markers
            misn.setMarker(baronsys, "misc")
        end
    end
end

function seller()
    if system.cur() == dest[1][1]then
        if tk.choice(title[8], text[9], buy, nobuy) == 1 then
            if player.credits() >= 15000 then
                misn.npcRm(sellnpc)
                player.pay(-15000)
                player.cargoAdd("Artefact A", 0)
            else
                tk.msg(nomoneytitle, nomoneytext)
            end
        end
    elseif system.cur() == dest[2][1] then
        if tk.choice(title[8], text[10], buy, nobuy) == 1 then
            if player.credits() >= 15000 then
                misn.npcRm(sellnpc)
                player.pay(-15000)
                player.cargoAdd("Artefact B", 0)
            else
                tk.msg(nomoneytitle, nomoneytext)
            end
        end
    elseif system.cur() == dest[2][1] then
        if tk.choice(title[8], text[11], buy, nobuy) == 1 then
            if player.credits() >= 15000 then
                misn.npcRm(sellnpc)
                player.pay(-15000)
                player.cargoAdd("Artefact C", 0)
            else
                tk.msg(nomoneytitle, nomoneytext)
            end
        end
    end
end

function jumpin()
    if sys.cur() == baronsys then
        pinnacle = pilot.add("Proteron Kahan", "trader", planet.get("Ulios"):pos() + vec2.new(-400,-400))[1]
        pinnacle:setFaction("Civilian")
        pinnacle:rename("Pinnacle")
        pinnacle:setInvincible(true)
        pinnacle:control()
        pinnacle:goto(planet.get("Ulios"):pos() + vec2.new( 400, -400), false)
        hook.pilot(pinnacle, "idle", "idle")
        hook.pilot(pinnacle, "hail", "hail")
    elseif artefactsfound > 0 then
        -- Spawn artefact hunters, maybe.
        local choice = rnd.rnd(1, 5)
        local fleep
        if choice == 1 then
            fleep = pilot.add2("Mercenary Wing 1", "baddie")
        elseif choice == 2 then
            fleep = pilot.add2("Mercenary Wing 2", "baddie")
        elseif choice == 3 then
            fleep = pilot.add2("Mercenary Wing 3", "baddie")
        end
        for i, j in ipairs(fleep) do
            if j:exists() then
                j:control()
                j:setHostile(true)
                j:attack(player.pilot())
            end
        end
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
    tk.msg(title[3], text[3])
    pinnacle:taskClear()
    pinnacle:brake()
    stopping = true
    boardhook = hook.board("board")
end

-- Function that tries to misspell whatever string is passed to it.
function mangle(intext)
    local outtext = intext
    
    local vowels = {"a", "e", "i", "o", "u", "y"}
    local consonants = {"b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "q", "r", "s", "t", "v", "w", "x", "z"}
    
    local i = 1
    local found = false
    
    while i < #intext - 1 do
        if isIn(intext:sub(i, i):lower(), consonants) and isIn(intext:sub(i + 1, i + 1):lower(), vowels) and isIn(intext:sub(i + 2, i + 2):lower(), consonants) then
            found = true
            break
        end
        i = i + 1
    end
    
    if found then
        local first = consonants[rnd.rnd(1, #consonants)]
        local second = vowels[rnd.rnd(1, #vowels)]
        if intext:sub(i, i):upper() == intext:sub(i, i) then first = first:upper() end -- preserve case
        if intext:sub(i + 1, i + 1):upper() == intext:sub(i, i) then second = second:upper() end -- preserve case
        outtext = intext:sub(-#intext, -(#intext - i + 2)) .. first .. second .. intext:sub(i + 2)
    end
    
    return outtext
end

-- Helper function for mangle()
function isIn(char, table)
    for _, j in pairs(table) do
        if j == char then return true end
    end
    return false
end

-- Function to handle jumping in for NPC ships better
pilot{"add2"} = function (fleetname, ai)
    local adjs = system.cur():adjacentSystems()
    local planets = system.cur():planets()
    
    local choice = rnd.rnd(1, #adjs + #planets)
    
    local flit
    
    if choice <= #adjs then flit = pilot.add(fleetname, ai, adjs[choice]) end
    else flit = pilot.add(fleetname, ai, planets[choice - #adjs]) end

    return flit
end

function abort()
    misn.finish(false)
    var.pop("baron_active")
end