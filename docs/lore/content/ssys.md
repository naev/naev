---
title: Space Objects (Spobs)
---
<!-- Now display all the spobs. -->
<div class="row row-cols-1 row-cols-md-5 g-4" id="systems">
<% @items.find_all('/ssys/*.md').sort{ |a,b| a[:name]<=>b[:name] }.each do |s| %> <!--*-->
 <%= card_ssys( s ) %>
 <%= modal_ssys( s ) %>
<% end %>
</div>

<% @items.find_all('/spob/*.md').sort{ |a,b| a[:name]<=>b[:name] }.each do |s| %> <!--*-->
 <%= modal_spob( s ) %>
<% end %>
