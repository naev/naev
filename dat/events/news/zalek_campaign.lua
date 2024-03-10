local articles = {
   -- Particle Physics campaign
   {
      test = function ()
         return not player.misnDone("Za'lek Particle Physics 6")
      end,
      head = N_([[Chairwoman Missing]]),
      body = _([[The Za'lek Leadership Committee has been adjourned due to officials not being able to get in touch with the chairwoman. "She must be indulging in her research again" commented a research leader.]]),
   },
   -- Black Hole Campaign
   {
      test = function ()
         return not player.misnDone("Za'lek Black Hole 11")
      end,
      head = N_([[Strange Signals near Anubis Black Hole]]),
      body = _([[An unmanned listening post picked up strange signals near the Anubis Black Hole. Scientists are baffled as there is no explanation with the current model of physics that can explain the signal and are looking for alternate theories.]]),
   },
   {
      test = function ()
         return not player.misnDone("Za'lek Black Hole 11")
      end,
      head = N_([[Scientific Rivalries on the Rise]]),
      body = _([[Following a worrisome trend, two rival scientists bickering escalated to all-out warfare in the Stelman system. Forensic researchers are currently investigating the strewn debris.]]),
   },
}

return function ()
   return "Za'lek", nil, nil, articles
end
