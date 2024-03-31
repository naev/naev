---
title: Naev Space Objects
---
<div class="row row-cols-1 row-cols-md-5 g-4">
<% @items.find_all('/spob/*.md').each do |s| %> <!--*-->
<%
    if not s[:spob][:GFX].nil? and not s[:spob][:GFX][:space].nil?
        gfx = "/gfx/spob/space/"+s[:spob][:GFX][:space]
    end
    cls = ""
    if not s[:spob][:presence].nil? and not s[:spob][:presence][:faction].nil?
        faction = s[:spob][:presence][:faction]
    else
        faction = "Factionless"
    end
    cls += " fct-"+faction
    spobclass = s[:spob][:general][:class]
    cls += " cls-"+spobclass
    if not s[:spob][:general][:services].nil?
        services = s[:spob][:general][:services].keys
    else
        services = []
    end
    services.each do |s|
        cls += " srv-"+s.to_s
    end
    if not s[:spob][:tags].nil?
        tags = Array(s[:spob][:tags][:tag])
    else
        tags = []
    end
    tags.each do |t|
        cls += " tag-"+t
    end
%>
 <div class="col">
  <div class="card <%= cls %>">
   <% if not gfx.nil? %>
   <img src="<%= gfx %>" class="card-img-top" alt="<%= s[:spob][:GFX][:space] %>">
   <% end %>
   <div class="card-body">
    <h5 class="card-title"><%= s[:spob][:"+@name"] %></h5>
    <div class="card-text">
     <div>
      <span class="badge rounded-pill text-bg-primary"><%= faction %></span>
      <span class="badge rounded-pill text-bg-primary">Class <%= spobclass %></span>
     </div>
     <div>
     <% services.each do |s| %>
      <span class="badge rounded-pill text-bg-secondary"><%= s %></span>
     <% end %>
     </div>
     <div>
     <% tags.each do |t| %>
      <span class="badge rounded-pill text-bg-info"><%= t %></span>
     <% end %>
     </div>
    </div>
   </div>
  </div>
 </div>
<% end %>
</div>
