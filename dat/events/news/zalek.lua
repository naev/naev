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
   {
      head = N_([[Tessellated Singularity Observed]]),
      body = _([[New paper published in Estimation Weekly provides proof of naturally occurring tessellated singularity. Future applications may include a fountain pen configuration and a new colour somewhere between pink and magenta.]]),
   },
   --[[
      Business
   --]]
   {
      head = N_([[New Startup to Harness Power of Nebula]]),
      body = _([[A new local startup at Stein has claimed they have found a way to stabilizer power generators in volatile nebula. They are seeking to secure funding to begin experiments this cycle. Sceptics point out that this contradicts Yang-Mills theory.]]),
   },
   {
      head = N_([[Company Selling Prisms Surprisingly a Pyramid Scheme]]),
      body = _([[Investors appalled as PointMe Inc. found by special investigation to be an elaborate Pyramid scheme. One investor is quoted as saying; "I can't believe what I'm seeing. This shattered my outlook completely!"]]),
   },
   --[[
      Politics
   --]]
   {
      head = N_([[Za'lek Leadership Committee Breaks Record Length]]),
      body = _([[Surpassing the previous record of 3.198 periods of uninterrupted deliberations, the Za'lek Leadership Committee was called to a stop when too many research leaders had to be taken to hospitals due to sleep deprivation and starvation.]]),
   },
   {
      head = N_([[Monument Desecrated]]),
      body = _([[The "On the Shoulder of Giants" monument was desecrated with the controversial phrase "1+1=3" repeated in red. Authorities are investigating and request anyone having knowledge about the issue contact them immediately.]]),
   },
   --[[
      Human interest.
   --]]
   {
      head = N_([[Altercation at Conference on Philosophy and Ethics]]),
      body = _([[A large fight broke out when Prof. Li was presenting her new theory on quantum dualism. Dualists and materialists used chairs as improvised weapons until riot police were able to disperse the scene. General Chair de la Figueira boasted it was the least violent conference up to date with only 17 hospitalized.]]),
   },
   {
      head = N_([[Za'lek Citizen Arrested]]),
      body = _([[A long renowned scientist arrested for using an unauthorized nuclear device to make toast. A three-headed cat attained from perpetrator's apartment and sent for further studies.]]),
   },
}

return function ()
   return "Za'lek", head, greeting, articles
end
