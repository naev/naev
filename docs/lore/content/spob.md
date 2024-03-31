---
title: Naev Space Objects
---
<div class="row row-cols-1 row-cols-md-3 g-4">
<% @items.find_all('/spob/*.md').each do |s| %> <!--*-->
 <div class="col">
  <div class="card">
   <% if not s[:spob][:GFX].nil? and not s[:spob][:GFX][:space].nil? %>
   <img src="/gfx/spob/space/<%= s[:spob][:GFX][:space] %>" class="card-img-top" alt="<%= s[:spob][:GFX][:space] %>">
   <% end %>
   <div class="card-body">
    <h5 class="card-title"><%= s[:spob][:"+@name"] %></h5>
    <div class="card-text">
     <% if not s[:spob][:faction].nil? %>
      <span class="badge rounded-pill text-bg-primary"><%= s[:spob][:faction] %></span>
     <% else %>
      <span class="badge rounded-pill text-bg-primary">Factionless</span>
     <% end %>
      <span class="badge rounded-pill text-bg-primary">Class <%= s[:spob][:general][:class] %></span>
     <% if not s[:spob][:tags].nil? %>
     <% Array(s[:spob][:tags][:tag]).each do |t| %>
      <span class="badge rounded-pill text-bg-secondary"><%= t %></span>
     <% end %>
     <% end %>
    </div>
   </div>
  </div>
 </div>
<% end %>
</div>
