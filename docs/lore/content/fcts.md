---
title: Factions
---
<!-- Now display all the spobs. -->
<div class="row row-cols-1 row-cols-md-5 g-4" id="spobs">
<% @items.find_all('/fcts/*.md').sort{ |a,b| a[:name]<=>b[:name] }.each do |f| #* %>
 <%= fcts_card( f ) %>
<% end %>
</div>

<%= modal_addAll() %>
