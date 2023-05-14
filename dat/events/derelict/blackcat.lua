local misnname = "Black Cat"

-- Must not have been done yet
if player.misnDone(misnname) or player.misnActive(misnname) then
   return
end

-- Must have Wild Ones presence
if system.cur():presence("Wild Ones") <= 0 then
   return
end

return {
   mission = "Black Cat",
}
