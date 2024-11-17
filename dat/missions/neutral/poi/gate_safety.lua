local fmt = require "format"
local vn = require "vn"
local vne = require "vnextras"
local tut = require "common.tutorial"
local poi = require "common.poi"

local misnvar = "poi_gate_safety"

return function ( mem )
   -- Must be locked
   if not mem.locked then return end

   -- Already done
   if var.peek( misnvar ) then
      return
   end

   return {
      type = "function",
      ship = "Za'lek Sting",
      func = function ()

         local sai = tut.vn_shipai()
         vn.appear( sai, tut.shipai.transition )
         sai(_([[Your ship AI appears as you access the system.
"Now that we are in, let me see what there is available. Oh, it looks like there are some unencrypted communication exchanges. Let me pull it up."]]))
         vn.disappear( sai, tut.shipai.transition )

         vn.scene()
         local log = vne.flashbackTextStart()
         log(_([[#nTo:#0 Dr. Krellis
#nFrom:#0 Imperial Gate Experiment Safety Committee
#nSubject:#0 Re: Fwd: Re: Regarding Experimental Gate Safety

Dear Postdoc Krellis,

Thank you for the detailed explanation of your theory on heterogenic metaspace breakdowns. While it is very interesting, it relies on some hypothesis that our initial safety experiments have disproved over and over again. We remind you that we at the Empire take security and protocol very seriously, and we do not believe such a dangerous flaw could have escaped our experimental protocols.

If you have any questions on our experimental protocols, please file an EE-X-232 form with your inquiry.

We remind you to refrain from publicizing your view, as it is subject to Article 8914-3b.

Sincerely,
Imperial Gate Experiment Safety Committee]]))
         log(_([[#nTo:#0 Imperial Gate Experiment Safety Committee
#nFrom:#0 Dr. Krellis
#nSubject:#0 Re: Fwd: Re: Regarding Experimental Gate Safety

Dear Safety Committee,

I would like to point out that I have addressed the flaws of the experimental protocols in appendix G. The "worst case" considered is too conservative, a margin coefficient of 2 is clearly insufficient in this case.

You do realize that this is not a hypothetical, but practical concern, do you not? My theory irrefutably shows that if the right conditions are met, this could result in a catastrophe not seen in all of human history! Considering the potential risk, I do stress that the points I elaborate throughout the 523-page document should be taken into account before further experiments.

Regards,
Dr. Krellis
]]))
         log(_([[#nTo:#0 Dr. Krellis
#nFrom:#0 Imperial Gate Experiment Safety Committee
#nSubject:#0 Re: Fwd: Re: Regarding Experimental Gate Safety

Dear Postdoc Krellis,

We reiterate that everything is being done according to protocol, with approval of the advisory board which includes several prominent House Za'lek professors. We doubt that they have missed something so important that could be found by a postdoc.

Science is not done by claiming flaws in the excellent work of intergalactic researchers, but by collaborating and working together to make the Empire and Houses better.

We will be filing a complaint with your superior, please do not pursue this further.

Sincerely,
Imperial Gate Experiment Safety Committee]]))
         vne.flashbackTextEnd()

         vn.appear( sai, tut.shipai.transition )
         sai(_([["That is all that I was able to recover, as it seems that the owner of the ship attempted to format all the data."]]))

         local reward = poi.data_str(1)
         vn.na(fmt.f(_([[Other than the data recovered by {shipai}, the only thing of use you were able to find on the ship was a {reward}, which you take with you.]]),
            {shipai=tut.ainame(), reward=reward}))
         vn.na(fmt.reward(reward))

         vn.func( function ()
            var.push( misnvar, true )
            poi.data_give(1)
            poi.log(fmt.f(_([[You found a derelict ship in the {sys} system with an exchange between a postdoc and imperial gate experiment safety committee. You also were able to find to recover {reward} from the ship.]]),
               {sys=mem.sys, reward=reward}))
         end )
      end,
   }
end
