
--[[
-- Event for creating random characters in the spaceport bar.
--]]


--[[
--    Entry point for the event.
--]]

lang = naev.lang()
if lang == 'es' then --not translated atm
else --default english
    civ_name = "Civilian"
    civ_portraits = {
        "thief1",
        "thief2"
    }
    civ_desc = {
        "A worker on his day off.",
        "Just some drunk.",
        "You see a civilian.",
        "You see a highly drunk civilian.",
        "You have the eery suspicion that the individual you spot is severely intoxicated.",
        "There is a civilian sitting on one of the tables. He ignores you.",
        "There is a civilian sitting there, looking somewhere else.",
        "A worker sits at one of the tables, he's wearing a nametag saying \"Go away\".",
        "A civilian sits at the bar, bragging about how many Alterian burning cocktails he can drink.",
        "You see a drunk civilian, babbling to himself.",
        "You see a civilian wearing a shirt saying: \"Ask me about Jaegnhild\"",
        "There is a civilian sitting in the corner.",
        "A civilian on his day off.",
        "The man seems to be new to drinking and is disgusted by the other people in the bar.",
        "A man feverishly concentrating on the fluorescing drink in his hand."
    }
    civ_msg = {}
    civ_msg["general"] = {
        "I could make you rich, you know.",
        "I once knew a guy who looked like you.",
        "He passes out.",
        "So, as I was saying, merchants suck.",
        "He tells you his entire life story before demanding you pay his bill.",
        "They should start giving out heavy combat licenses to civilians, we'd clean this place up for good!",
        "Have you tried reverse skydiving? No? You should.",
        "Pineapples!",
        "I have nothing to say to you.",
        "This guy I know flew into the Nebula... he never came back. That's why I stay here.",
        "There's too many hoodlums around these days. You can't feel safe anymore.",
        "Man, nobody saw the Incident coming... I lost half my family on Earth.",
        "How do we know that the Sol system is really destroyed? Did someone go there and check?",
        "Piracy is blooming since the Incident...",
        "Do you want to hear my opinion? No? Well, that's fine too."
    }
    civ_msg["Empire"] = {
        "The Emperor really is watching, you know.",
        "Between you and me, I don't think the Emperor is all that powerful anymore.",
        "Don't tell anyone, but I think the Incident was the deathblow for the Empire.",
        "What's going to become of the Empire?",
        "I hear the Pirates are coming closer and closer to Empire space now. Soon they'll be able to march freely around Polaris Prime, I tell you.",
        "If I were the Emperor, we wouldn't be having those Piracy problems.",
        "Do you think the Empire will ever become as powerful as during the Golden Age?",
        "The enemies of the Empire are circling like vultures, it won't be long until it falls.",
        "If you ask me, the Emperor has no power anymore. It's all in the military. They might as well declare martial law.",
        "If those fleets are supposed to keep us safe, why did I just receive word that another colony has been raided by the Pirates?",
        "Pirates, FLF, and now the Incident... it's getting too much for the Empire."
    }
    civ_msg["Dvaered"] = {
        "Damn FLF...",
        "Those FLF terrorists don't even value their own lives... They'll just rush into a system to do as much damage as possible...",
        "I don't know what all that hassle is about... those colonies would be better off ruled by us.",
        "Those colonies belong to the Dvaered! The so-called \"Frontier Alliance\" doesn't even have a legal right to exist!",
        "Frontier Liberation Front, pah, more like Frontier Liquidation Front! If they stay at it like this, we'll crush the whole lot of them soon, just you wait!",
        "I understand that the FLF only wants to protect their colonies, but is all this violence really necessary?",
        "The FLF has to be destryoed! They are terrorists, they don't deserve any better!",
        "An Invasion, that's what we need!",
        "I don't understand those FLF guys, how can they claim to want to make things better and then kill so many people?",
        "Someone oughta show those FLF terrorists their place: A work camp. If only the High Command would answer to my letters.",
        "I think we should give in to the FLF before they kill more people. It's not safe to go to the next system anymore."
    }
    civ_msg["FLF"] = {
        "Damn Dvaered...",
        "The Dvaered can resist as much as they want, in the end, we will succeed!",
        "If you ask me, we're doomed. When the Dvaered manage to mobilize their war machine, they can crush us at any time!",
        "We're loosing pilots every day, how long can we hold the resistance up?",
        "We need a miracle, and we need one fast.",
        "I someone goes ahead and unites the generals of the Dvaered High Command, we're doomed.",
        "For the Frontier!",
        "We will fight until the last man!",
        "Keep it to yourself, but I think some of the Frontier govermnents are among our supporters.",
        "I wonder how command can acquire the resources to keep the resistance up? What, do you think I should know that? Oh, well.",
        "Death to the oppressors!"
    }
    civ_msg["Frontier"] = {
        "Those FLF guys are doing the right thing, you know.",
        "The FLF stands up for us when nobody else will, we should really be supporting them.",
        "I don't want the Dvaered in here either, but do we have to resort to violence like the FLF does?",
        "I'm a proud member of the Frontier Alliance and even to me, the FLF are criminals.",
        "I heard the government is supporting the FLF. It's all secret of course. Don't believe their lies.",
        "I think we need to talk to the Emperor. Maybe he can get the Dvaered to stop?",
        "We need to gather allies! Raise a force to resist the Dvaered!",
        "It's only a matter of time if you ask me...",
        "I'm telling you, the Dvaered are up to no good!",
        "I don't know what everbody's problem is, I see the Dvaered as a symbol of progress, we should welcome them!",
        "We should give in to the Dvaered before more people get hurt."
    }
    civ_msg["Proteron"] = {
        "I don't know what to say..."
    }
    civ_msg["Za'Lek"] = {
        "Do you want to see my prototype?",
        "The Soromid should've just stayed in their pesthole, they're putting me out of business...",
        "I hear they're working on a time machine now.",
        "I feel like I'm trapped in this life! Why should I provide for those damn lazy scientists.",
        "Those Soromids are immoral! Genesplicing is murder, buy my implants!",
        "We are a frickin' faction full of scientists and nobody is able to explain the Incident?!",
        "The Incident cut off the Proteron completely, and we can't even figure out why!",
        "I'm sick of providing for those lazy-ass scientists!",
        "I'm working my ass off every day to feed our scientists, and what do I get from it... But I guess I'm still better off than the Proteron...",
        "Those lesser minds shouldn't complain, it is us who keep them in prosparity!"
    }
    civ_msg["Sirius"] = {
        "I wish I could see the Sirichana",
        "Have you seen the glow in the eyes of the Touched?",
        "I hear the Sirichana recently spoke again.",
        "A friend of mine sold everything he owned for a transport to Mutris.",
        "I fear the Dvaered may lay down claims for our territories next.",
        "Those are dark times, may the Sirichana keep us safe.",
        "There are doubters, but I trust the wisdom of the Sirichana.",
        "Pirates raided a ship with a Touched on board, is nothing sacred?",
        "How do the Touched even know that they will arrive at Mutris? I guess it's all faith.",
        "Have you seen the determination the Touched preach in? It really makes you believe...",
        "The Pirate raids have been getting worse, I hope they can keep Mutris safe."
    }
    civ_msg["Goddard"] = {
        "Sometimes I feel we are just a glorified shipyard. Then I realize: We are!",
        "I think the Empire only tolerates us because of the cruisers.",
        "I often wonder what it would be like if we had a large territory. We'd actually have a say!"
    }
    civ_msg["Independent"] = {
        "I'm glad that we stay out of the fights the factions get in..."
    }
    
    merch_name = "Merchant"
    merch_portraits = {
        "jorek",
        "thief1",
        "thief2"
    }
    merch_desc = {
        "You spot a merchant.",
        "You spot a drunk merchant.",
        "A merchant between missions.",
        "A merchant, he reminds you of someone.",
        "There is sufficient evidence for you to make the educated assumption that the person in question has ingested more than the recommended dose of alcohol.",
        "A big man with a trucker cap.",
        "A man wearing a shirt saying: \"I visited 78 worlds, ask me about it\".",
        "An unfriendly looking trader.",
        "A trader bragging about his ship.",
        "A merchant telling people how many pirates he has killed.",
        "An unpleasant looking merchant.",
        "You see a man enjoying his beer sitting in the corner.",
        "A merchant.",
        "A small man, looking around.",
        "A typical merchant"
    }
    merch_msg = {
        "You wanna buy some speedsticks?",
        "I have these crystals, you simply must buy some.",
        "How about you buy me a beer?",
        "Go away.",
        "Do you think I look like a mule?",
        "I've been on more worlds than you can count.",
        "It's become quite dangerous out there. The Pirates are multiplying! I hear they're organized now.",
        "I lost many important trade contacts in the Incident. It was a tragedy.",
        "Those damn Soromids are controlling the market in augmentations. It's criminal!",
        "You don't support those Soromids, do you?",
        "Are you sure you can be seen talking to me? I've got a history here...",
        "Do you want to hear about the time I almost managed to land on Mutris?",
        "I know a guy who started with a paperclip and traded and traded. He ended up with a Goddard, I think.",
        "Those Pirates are everywhere! And don't even get me started on the FLF!",
        "A man can hardly go after his lawful business anymore, it's just not safe out there.",
        "I got ripped off by a Soromid once! They're all traitors, I tell you!"
    }
    
    misn_msg = {} --mission hints, to be displayed before the mission has been completed, i.e. hints on where to find it.
                  --syntax: misn_msg["mission name"] = "text"
end   


function create ()
    global_npc = {}
    
    faction = planet.cur():faction() --get current planets faction
    if faction == nil then
       return
    end

    reputation = player.getFaction( faction:name() ) --get players affiliation with said faction
    civ_msg_fac = civ_msg["general"] --the stuff that's always said
    local msg_tab = civ_msg[faction:name()]
    if msg_tab == nil then -- Must have messages available
       hook.takeoff( "leave" )
       return
    end
    for key, value in pairs(msg_tab) do --add faction-specific stuff
        table.insert(civ_msg_fac, value)
    end
    civ_misn_msg = {}
    merch_misn_msg = {}
    for key, value in pairs(misn_msg) do
        if player.misnDone(key) ~= true then --test if the player has done the mission, if not, add it to the list of possible phrases
            table.insert(civ_misn_msg, value)
            table.insert(merch_misn_msg, value)
        end
    end 
    
    local n = rnd.rnd(1,5)
   
    local i = 0
    while i < n do
        create_npc()
        i = i + 1
    end
    -- End event on takeoff.
    hook.takeoff( "leave" )
end

--[[
--    Generates an NPC
--]]
function create_npc ()
   -- Choose type
   local data
   r = rnd.rnd()
   if r > 0.5 then
      data = neutral_genMerchant()
   else
      data = neutral_genCivilian()
   end

   -- Extract the data
   local name = data[1]
   local portrait = data[2]
   local desc = data[3]
   local func = "npc_talk"
   local msg  = data [4]

   -- Create the NPC
   local id = evt.npcAdd( func, name, portrait, desc, 10 )

   -- Create new data
   local data = {}
   data["name"] = name
   data["msg"]  = msg

   -- Add to global table
   global_npc[ id ] = data
end

--[[
--    NPC talk function
--]]
function npc_talk( id )
   local data = global_npc[ id ]

   -- Extract the data
   local name = data["name"]
   local msg  = data["msg"]

   -- Display text
   tk.msg( name, msg )
end


function neutral_genMerchant () --Merchants are faction neutral, they always say the same things, regardless of position.
   local npcdata = {}
   npcdata[1] = merch_name
   npcdata[2] = merch_portraits[ rnd.rnd( 1, #merch_portraits ) ]
   npcdata[3] = merch_desc[ rnd.rnd( 1, #merch_desc ) ]
   if rnd.rnd( 0, 100 ) < 11 and #merch_misn_msg >= 1 then --they also have the same chance of revealing a hint about a mission.
       npcdata[4] = merch_misn_msg[ rnd.rnd( 1, #merch_misn_msg ) ]
   else
       npcdata[4] = merch_msg[ rnd.rnd( 1, #merch_msg ) ]
   end
   return npcdata
end


function neutral_genCivilian () --Civilians always belong to the faction the planet belongs to.
   local npcdata = {}
   npcdata[1] = civ_name
   npcdata[2] = civ_portraits[ rnd.rnd( 1, #civ_portraits ) ]
   npcdata[3] = civ_desc[ rnd.rnd( 1, #civ_desc ) ]
   if rnd.rnd( 0, 100 ) < reputation and #civ_misn_msg >= 1 then --the chance for them to give hints depends on the players affiliation with that faction
       npcdata[4] = civ_misn_msg[ rnd.rnd( 1, #civ_misn_msg ) ]
   else
       npcdata[4] = civ_msg_fac[ rnd.rnd( 1, #civ_msg_fac ) ]
   end
   return npcdata
end

--[[
--    Event is over when player takes off.
--]]
function leave ()
    evt.finish()
end
