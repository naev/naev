--[[
<?xml version='1.0' encoding='utf8'?>
<mission name="Proteron Dissident Dead Or Alive Bounty">
  <avail>
   <priority>4</priority>
   <cond>player.numOutfit("Mercenary License") &gt; 0</cond>
   <chance>360</chance>
   <location>Computer</location>
   <faction>Proteron</faction>
  </avail>
  <notes>
   <tier>3</tier>
  </notes>
 </mission>
 --]]
--[[

   Dead or Alive Proteron Dissident Bounty

--]]

require "missions.neutral.pirbounty_dead"

subdue_title   = _("Captured Alive")
subdue_text    = {}
subdue_text[1] = _("You and your crew infiltrate the ship's pathetic security and subdue the dissident. You transport them to your ship.")
subdue_text[2] = _("Your crew has a difficult time getting past the ship's security, but eventually succeeds and subdues the dissident.")
subdue_text[3] = _("The ship's security system turns out to be no match for your crew. You infiltrate the ship and capture the dissident.")
subdue_text[4] = _("Your crew infiltrates the ship and captures the dissident.")
subdue_text[5] = _("Getting past this ship's security was surprisingly easy. Didn't this dissident know that they were wanted?")

subdue_fail_title   = _("Capture Failed")
subdue_fail_text    = {}
subdue_fail_text[1] = _("Try as you might, you cannot get past the dissident's security system. Defeated, you and your crew return to the ship.")
subdue_fail_text[2] = _("The ship's security system locks you out.")
subdue_fail_text[3] = _("Your crew comes close to getting past the dissident's security system, but ultimately fails.")
subdue_fail_text[4] = _("It seems your crew is no match for this ship's security system. You return to your ship.")

pay_title   = _("Mission Completed")

pay_kill_text    = {}
pay_kill_text[1] = _("After verifying that you killed your target, an officer hands you your pay.")
pay_kill_text[2] = _("After verifying that the target is indeed dead, the tired-looking officer smiles and hands you your pay.")
pay_kill_text[3] = _("The officer thanks you and promptly hands you your pay.")
pay_kill_text[4] = _("The officer takes you into a locked room, where the death of your target is quietly verified. The officer then pays you and sends you off.")
pay_kill_text[5] = _("The officer verifies the death of your target, goes through the necessary paperwork, and hands you your pay, looking bored the entire time.")

pay_capture_text    = {}
pay_capture_text[1] = _("An officer takes the dissident into custody and hands you your pay.")
pay_capture_text[2] = _("The dissident puts up a fight as you take them to the officer, who then hands you your pay.")
pay_capture_text[3] = _("The officer you deal with seems to especially dislike dissidents. The one you captured is taken off your hands and you are handed your pay without a word.")
pay_capture_text[4] = _("An officer rushes the fearful-looking dissident into a secure hold, pays you the appropriate bounty, and takes the dissident into custody.")
pay_capture_text[5] = _("The officer you greet gives you a puzzled look when you say that you captured your target alive. Nonetheless, they politely take the dissident off of your hands and hand you your pay.")

share_title   = _("A Smaller Reward")
share_text    = {}
share_text[1] = _([["Greetings. I can see that you were trying to collect a bounty. Well, as you can see, I earned the bounty, but I don't think I would have succeeded without your help, so I've transferred a portion of the bounty into your account."]])
share_text[2] = _([["Sorry about getting in the way of your bounty. I don't really care too much about the money, but I just wanted to make sure the galaxy would be rid of that scum. So as an apology, I would like to offer you the portion of the bounty you clearly earned. The money will be in your account shortly."]])
share_text[3] = _([["Hey, thanks for the help back there. I don't know if I would have been able to handle that dissident alone! Anyway, since you were such a big help, I have transferred what I think is your fair share of the bounty to your bank account."]])
share_text[4] = _([["Heh, thanks! I think I would have been able to take out the target by myself, but still, I appreciate your assistance. Here, I'll transfer some of the bounty to you, as a token of my appreciation."]])
share_text[5] = _([["Ha ha ha, looks like I beat you to it this time, eh? Well, I don't do this often, but here, have some of the bounty. I think you deserve it."]])


-- Mission details
misn_title  = _("PD: Dead or Alive Bounty in %s")
misn_desc   = _("A political dissident was recently seen in the %s system. %s authorities want this dissident dead or alive.")

-- Messages
msg    = {}
msg[1] = _("MISSION FAILURE! Your target got away.")
msg[2] = _("MISSION FAILURE! Another pilot eliminated your target.")
msg[3] = _("MISSION FAILURE! You have left the %s system.")

osd_title = _("Bounty Hunt")
osd_msg    = {}
osd_msg[1] = _("Fly to the %s system")
osd_msg[2] = _("Kill or capture your target")
osd_msg[3] = _("Land in %s territory to collect your bounty")
osd_msg["__save"] = true


function create ()
   paying_faction = planet.cur():faction()
   target_faction = faction.get( "Proteron Dissident" )

   local systems = lmisn.getSysAtDistance( system.cur(), 1, 3,
      function(s)
         local p = s:presences()["Proteron Dissident"]
         return p ~= nil and p > 0
      end )

   if #systems == 0 then
      -- No dissidents nearby
      misn.finish( false )
   end

   missys = systems[ rnd.rnd( 1, #systems ) ]
   if not misn.claim( missys ) then misn.finish( false ) end

   jumps_permitted = system.cur():jumpDist(missys) + rnd.rnd( 5 )
   if rnd.rnd() < 0.05 then
      jumps_permitted = jumps_permitted - 1
   end

   level = 1
   name = _("Target Dissident")
   pship = "Schroedinger"
   credits = 50e3
   reputation = 0
   board_failed = false
   bounty_setup()

   -- Set mission details
   misn.setTitle( misn_title:format( missys:name() ) )
   misn.setDesc( misn_desc:format( missys:name(), paying_faction:name() ) )
   misn.setReward( fmt.credits( credits ) )
   marker = misn.markerAdd( missys, "computer" )
end


function set_faction( p )
   if not _target_faction then
      _target_faction = faction.dynAdd( "Independent", "Proteron Dissident", _("Proteron Dissident") )
      _target_faction:dynEnemy( "Proteron" )
   end
   p:setFaction( _target_faction )
end


-- Set up the ship, credits, and reputation.
function bounty_setup ()
   local choices = { "Schroedinger", "Hyena", "Llama", "Gawain" }
   pship = choices[ rnd.rnd( 1, #choices ) ]
   credits = 150e3 + rnd.sigma() * 15e3
   reputation = rnd.rnd( 1, 2 )
end
