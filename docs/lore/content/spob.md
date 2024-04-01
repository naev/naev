---
title: Space Objects
---
<div class="row row-cols-1 row-cols-md-5 g-4">
<% @items.find_all('/spob/*.md').each do |s| %> <!--*-->
<%
    name = s[:spob][:"+@name"]
    id = Base64.encode64( name )
    if not s[:spob][:GFX].nil? and not s[:spob][:GFX][:space].nil?
        gfx = relative_path_to(@items["/gfx/spob/space/"+s[:spob][:GFX][:space]])
    end
    if not s[:spob][:GFX].nil? and not s[:spob][:GFX][:exterior].nil?
        exterior = relative_path_to(@items["/gfx/spob/exterior/"+s[:spob][:GFX][:exterior]])
    end
    if not s[:spob][:general][:description].nil? then
        description = s[:spob][:general][:description]
    end
    if not s[:spob][:general][:bar].nil? then
        bar = s[:spob][:general][:bar]
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
 <!-- Card -->
 <div class="col">
  <div class="card bg-black <%= cls %>" data-bs-toggle="modal" data-bs-target="#modal-<%= id %>" >
   <% if not gfx.nil? %>
   <img src="<%= gfx %>" class="card-img-top" alt="<%= s[:spob][:GFX][:space] %>">
   <% end %>
   <div class="card-body">
    <h5 class="card-title"><%= name %></h5>
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
 <!-- Modal -->
 <div class="modal fade" id="modal-<%= id %>" tabindex="-1" aria-labelledby="modal-label-<%= id %>" aria-hidden="true">
  <div class="modal-dialog modal-xl modal-dialog-centered modal-dialog-scrollable">
   <div class="modal-content">
    <div class="modal-header">
     <h1 class="modal-title fs-5" id="modal-label-<%= id %>"><%= name %></h1>
     <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
    </div>
    <div class="modal-body clearfix">
     <% if not exterior.nil? %>
     <img src="<%= exterior %>" class="rounded col-md-6 float-md-end mb-3 ms-md-3" alt="<%= s[:spob][:GFX][:exterior] %>">
     <% elsif not gfx.nil? %>
     <img src="<%= gfx %>" class="col-md-6 float-md-end mb-3 ms-md-3" alt="<%= s[:spob][:GFX][:gfx] %>">
     <% end %>
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
     <% if not description.nil? %>
     <div>
     <h5>Description:</h5>
     <p><%= description %></p>
     </div>
     <% end %>
     <% if not bar.nil? %>
     <div>
     <h5>Spaceport Bar:</h5>
     <p><%= bar %></p>
     </div>
     <% end %>
    </div>
    <div class="modal-footer">
     <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">Close</button>
    </div>
   </div>
  </div>
 </div>
<% end %>
</div>
