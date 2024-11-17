---
title: Non-Player Characters (NPCs)
---
<!-- Now display all the spobs. -->
<div class="row row-cols-1 row-cols-md-5 g-4" id="systems">
<% @items.find_all('/npcs/*.md').sort{ |a,b| a[:name]<=>b[:name] }.each do |n| #* %>
 <%= npc_card( n ) %>
<% end %>
</div>

<%= modal_addAll() %>
