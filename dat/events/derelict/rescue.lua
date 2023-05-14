local misnname = "Derelict Rescue"

-- Lower duplicates by ignoring active
if player.misnActive(misnname) then
   return
end

return {
   mission = misnname,
}
