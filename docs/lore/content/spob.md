---
title: spobs
---
<ul class="foto-gallery">
<% @items.find_all('/spob/*.md').each do |s| %>
    <li><%= s[:spob][:"+@name"] %></li>
<% end %>
</ul>
