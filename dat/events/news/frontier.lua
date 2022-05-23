local header = {
   _("News from the Frontier Alliance."),
}
local greeting = {
   _("News you can trust."),
}
local articles = {
   --[[
      Science and technology
   --]]
   --[[
      Business
   --]]
   --[[
      Politics
   --]]
   {
      header = N_([[Election on Caladan Marred by Fraud]]),
      body = _([[As many as two of every hundred votes counted after the recent polling decaperiod may be falsified, an ombudsman reports. The opposition party demanded the election be annulled.]])
   },
   {
      header = N_([[Empire Relies on Prison Labour]]),
      body = _([[A recent report by the Soromid House Ways and Means Committee suggests infrastructure may be too dependent the on the incarcerated population.]])
   },
   {
      header = N_([[Imperial Oversight Scandal]]),
      body = _([[Sources close to the Imperial Chancellor say they see the failure at the Department of Oversight, whose inspectors overlooked serious failings in other supervisory bodies, as a serious oversight.]])
   },
   --[[
      Human interest.
   --]]
}

return function ()
   return "Frontier", header, greeting, articles
end
