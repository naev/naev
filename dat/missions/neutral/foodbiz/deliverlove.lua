
--[[
Mission Name: Deliver Love
Author: iwaschosen
Plot: Talk to man on Zeo, bargain, load some cargo, deliver it to Zhiru in Goddard, get $
--]]

lang = naev.lang()
if lang == "es" then -- Spanish version of the texts would follow
elseif lang == "de" then -- German version of the texts would follow
else -- default English text

-- Dialogue section

  npc_name = "Well Dressed Man"
  bar_desc = "A man sits in the corner of the bar, writing a letter."
  title = "The longer the distance the longer the love lasts"
  cargoname = "Love Letters"
  firstcontact = [[You can't stop wondering why the man in the corner is writing on paper instead of a datapad. As you approach the table he motions you to sit. "You must be wondering why I am using such an old fashion way of recording information" he remarks, while looking at you with a grin. You take a sip of your drink as he continues "I am writing a poem of love to my beloved. She lives on %s."you glance at the flowing hand writing, back at the man, and back at the paper. "You wouldn't happen to be heading to %s would you?" he asks.]]
  acceptornot = [["It is a nice place I hear!" he exclaims visibly excited. "Say, I have been writing a fair bit of these letters now, you wouldn't be able to drop them off on your way would you?" Some how you knew that question was coming. "There would be a few credits in it for you... say %s credits?" The man adds quickly with a hopeful expression. It seems like a low reward for a long journey...]]
  bargain = [[As you begin to get up the man grabs your arm. "Alright how about %s credits. look I wouldn't want The Empire reading these... The Emperor himself would blush...". you sigh and give the man a long pause before answering.]]
  not_enough_cargospace = [[You run a check of your cargo hold and notice it is packed to the brim. "Did I not mention I wrote a ton of these letters? anyway I will be in the bar if you free some space up" the man replies. You didn't expect him to literally have a TONNE of letters..]]
  reward_desc = [[%s credit transfer on delivery]]
  misn_desc = [[Deliver the %s to %s in the %s system.]]
  misn_accomplished = [[Upon landing you receive a credit transfer of %s. As you exit your ship you see a young woman waving to you. "I heard you had a message for me!" she yells excitedly. You grin and point towards the cargo hold in your ship  "There are a couple in there for you!". She runs over to the dockworkers who are busy unloading her crate. You get a warm and fuzzy feeling on the inside. You have made someones day.]]
  misn_title = [[Deliver Love]]

  --Start Functions

  function create () --No system shall be claimed by mission

    startworld, startworld_sys = planet.cur()

    targetworld_sys = system.get("Goddard")
    targetworld = planet.get("Zhiru")
    reward = 10000
    misn.setNPC(npc_name,"neutral/unique/youngbusinessman")
    misn.setDesc(bar_desc)
  end


  function accept ()
  --introductions and a bit of bargaining
    if not tk.yesno(title,string.format(firstcontact,targetworld:name(),targetworld:name()))then
      misn.finish()
    end
    if not tk.yesno(title,string.format(acceptornot,reward))then
      reward = reward*2 --look at you go, double the reward
      if not tk.yesno(title,string.format(bargain,reward)) then
        misn.finish()
      end
    end
    if player.pilot():cargoFree() <  1 then
      tk.msg( title, not_enough_cargospace )
      misn.finish()
    end

    -- Add Mission Cargo and set up the computer

    misn.accept()
    cargoID = misn.cargoAdd(cargoname,1)
    misn.setTitle(misn_title)
    misn.setReward( string.format(reward_desc,reward ))
    misn.setDesc( string.format( misn_desc, cargoname,targetworld:name(), targetworld_sys:name() ) )
    misn.markerAdd(targetworld_sys, "low")

    -- set up hooks

    hook.land("land")
  end

  function land()
    if planet.cur() == targetworld then
      misn.cargoRm(cargoID)
      player.pay(reward)
      tk.msg(title,string.format(misn_accomplished,reward))
      misn.finish(true)
    end
  end

  function abort()
    misn.cargoRm(cargoID)
    misn.finish(false)
  end
end
