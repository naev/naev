local head = {
   _("Scientific, Socioeconomic, and Sundry Events"),
}
local greeting = {
   _("Information by optimization."),
   _("Statistically significant news."),
   _("Peer reviewed news."),
}
local articles = {
   --[[
      Science and technology
   --]]
   {
      head = N_([[Drone Malfunctions at Record High]]),
      body = _([[Recent production models of drones have reached a new high of malfunctions per cycle. Za'lek fabrication leader points a finger to inconsistent Riemann metrics as a likely cause.]]),
   },
   {
      head = N_([[New Funding Call for Antimatter Research]]),
      body = _([[The Ruadan Research Bureau has issued a new call for antimatter research funding. The highly sought funding had a 0.2% acceptance rate last cycle and is expected to become even more competitive due to new research quality metrics.]]),
   },
   {
      head = N_([[Breakthrough in Spectral Lattice Crystal Amplifiers]]),
      body = _([[The PRP-3 Advanced Materials Laboratory presented an important advance in crystal amplifiers which has large potential in beam stabilization and active capacitors.]]),
   },
   {
      head = N_([[P versus NP Proof Proven False]]),
      body = _([[The highly anticipated 5,928 page P versus NP proof of Prof. Picazzo was proven false when a typo was found in Theorem 238.87 by a team of post-doctoral researchers.]]),
   },
   --[[
      Business
   --]]
   {
      head = N_([[New Startup to Harness Power of Nebula]]),
      body = _([[A new local startup at Stein has claimed they have found a way to stabilizer power generators in volatile nebula. They are seeking to secure funding to begin experiments this cycle. Skeptics point out that this contradicts Yang-Mills theory.]]),
   },
   --[[
      Politics
   --]]
   {
      head = N_([[Za'lek Leadership Committee Breaks Record Length]]),
      body = _([[Surpassing the previous record of 3.198 periods of uninterrupted deliberations, the Za'lek Leadership Committee was called to a stop when too many research leaders had to be taken to hospitals due to sleep deprivation and starvation.]]),
   },
   --[[
      Human interest.
   --]]
   {
      head = N_([[Altercation at Conference on Philosophy and Ethics]]),
      body = _([[A large fight broke out when Prof. Li was presenting her new theory on quantum dualism. Dualists and materialists used chairs as improvised weapons until riot police were able to disperse the scene. General Chair Picazzo boasted it was the least violent conference up to date with only 17 hospitalized.]]),
   },
   {
      head = N_([[Za'lek Citizen Arrested]]),
      body = _([[Long renowned scientist arrested for using an unauthorized nuclear device to make breakfast toast. A three headed cat attained from perpetrator's apartment and sent for further studies.]]),
   },
}

return function ()
   return "Za'lek", head, greeting, articles
end
