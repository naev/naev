---
title: Space Objects
---

<% content_for :javascript do %>
<script>
var sort = "Name";
var reverse = false;
function sortbydata( d ) {
    var dsort = "data-"+d;
    var $spobs = $('#spobs');
    var $spoblist = $spobs.children(".col").detach();
    if (sort==d) {
        reverse = !reverse;
    }
    sort = d;
    $spoblist.sort( function( a, b ) {
        var ad = a.getAttribute(dsort);
        var bd = b.getAttribute(dsort);
        var c =  (''+ad).localeCompare(bd);
        if (reverse)
            c = -c;
        if (c)
            return c;
        var an = a.getAttribute("data-Name");
        var bn = b.getAttribute("data-Name");
        if (reverse)
            return (''+bn).localeCompare(an);
        else
            return (''+an).localeCompare(bn);
    } );
    $spoblist.appendTo($spobs);
    var dir;
    if (reverse)
        dir = "↓";
    else
        dir = "↑";
    $('button#btn-sort').text("Sort by: "+d+dir);
}
function sortbydatanumber( d ) {
    var dsort = "data-"+d;
    var $spobs = $('#spobs');
    var $spoblist = $spobs.children(".col").detach();
    if (sort==d) {
        reverse = !reverse;
    }
    sort = d;
    $spoblist.sort( function( a, b ) {
        var ad = a.getAttribute(dsort);
        var bd = b.getAttribute(dsort);
        var c =  ad-bd;
        if (reverse)
            c = -c;
        if (c)
            return c;
        var an = a.getAttribute("data-Name");
        var bn = b.getAttribute("data-Name");
        if (reverse)
            return (''+bn).localeCompare(an);
        else
            return (''+an).localeCompare(bn);
    } );
    $spoblist.appendTo($spobs);
    var dir;
    if (reverse)
        dir = "↓";
    else
        dir = "↑";
    $('button#btn-sort').text("Sort by: "+d+dir);
}
function randomize() {
    var $spobs = $('#spobs');
    var $spoblist = $spobs.children(".col").detach();
    $spoblist.sort( function( a, b ) {
        return Math.random() < 0.5;
    } );
    $spoblist.appendTo($spobs);
    sort = "Random";
    reverse = false;
    $('button#btn-sort').text("Sort by: Random");
}
</script>

<% end %>
<!-- First get some global stuff. -->
<%
spobdata = {}

factionlist = Set[ "Factionless" ]
taglist = Set[]
classlist = Set[]
@items.find_all('/spob/*.md').each do |s| # **
    if not s[:spob][:presence].nil? and not s[:spob][:presence][:faction].nil?
        factionlist.add( s[:spob][:presence][:faction] )
    end
    if not s[:spob][:tags].nil?
        taglist.add( Array(s[:spob][:tags][:tag]) )
    end
    classlist.add( s[:spob][:general][:class] )

    # Store useful data here
    spobdata[s.identifier] = {
        name: s[:spob][:"+@name"],
        id: Base64.encode64( s[:spob][:"+@name"] ),
    }
    sd = spobdata[s.identifier]

    if not s[:spob][:GFX].nil? and not s[:spob][:GFX][:space].nil?
        sd[:gfx] = relative_path_to(@items["/gfx/spob/space/"+s[:spob][:GFX][:space]])
    end
    if not s[:spob][:GFX].nil? and not s[:spob][:GFX][:exterior].nil?
        sd[:exterior] = relative_path_to(@items["/gfx/spob/exterior/"+s[:spob][:GFX][:exterior]])
    end
    if not s[:spob][:general][:description].nil? then
        sd[:description] = s[:spob][:general][:description]
    end
    if not s[:spob][:general][:bar].nil? then
        sd[:bar] = s[:spob][:general][:bar]
    end
    if not s[:spob][:general][:population].nil? then
        sd[:population] = s[:spob][:general][:population].to_f.to_i.to_s
    else
        sd[:population] = 0
    end
    if not s[:spob][:presence].nil? and not s[:spob][:presence][:faction].nil?
        sd[:faction] = s[:spob][:presence][:faction]
    else
        sd[:faction] = "Factionless"
    end
    sd[:spobclass] = s[:spob][:general][:class]
    if not s[:spob][:general][:services].nil?
        sd[:services] = s[:spob][:general][:services].keys
    else
        sd[:services] = []
    end
    if not s[:spob][:tags].nil?
        sd[:tags] = Array(s[:spob][:tags][:tag])
    else
        sd[:tags] = []
    end
end
%>

<div id="selection" class="m-3">
 <div class="dropdown">
  <button id="btn-sort" class="btn btn-primary dropdown-toggle" type="button" data-bs-toggle="dropdown" aria-expanded="false">
  Sort by: Name↑
  </button>
  <ul class="dropdown-menu">
   <li><a class="dropdown-item" href="#" onclick="sortbydata('Name');">Name</a></li>
   <li><a class="dropdown-item" href="#" onclick="sortbydata('Faction');">Faction</a></li>
   <li><a class="dropdown-item" href="#" onclick="sortbydata('Class');">Class</a></li>
   <li><a class="dropdown-item" href="#" onclick="sortbydatanumber('Population');">Population</a></li>
   <li><a class="dropdown-item" href="#" onclick="randomize();">Random</a></li>
  </ul>
 </div>
</div>

<!-- Now display all the spobs. -->
<div class="row row-cols-1 row-cols-md-5 g-4" id="spobs">
<% @items.find_all('/spob/*.md').sort{ |a,b| a[:spob][:"+@name"]<=>b[:spob][:"+@name"] }.each do |s| %> <!--*-->
<%
    # Useful spob Variables
    sd = spobdata[s.identifier]

    cls = ""
    cls += " fct-"+sd[:faction]
    cls += " cls-"+sd[:spobclass]
    sd[:services].each do |s|
        cls += " srv-"+s.to_s
    end
    sd[:tags].each do |t|
        cls += " tag-"+t
    end
%>
 <!-- Card -->
 <div class="col <%= cls %>" data-Name="<%= sd[:name] %>" data-Faction="<%= sd[:faction] %>" data-Class="<%= sd[:spobclass] %>" data-Population="<%= sd[:population] %>" >
  <div class="card bg-black" data-bs-toggle="modal" data-bs-target="#modal-<%= sd[:id] %>" >
   <% if not sd[:gfx].nil? %>
   <img src="<%= sd[:gfx] %>" class="card-img-top" alt="<%= s[:spob][:GFX][:space] %>">
   <% end %>
   <div class="card-body">
    <h5 class="card-title"><%= sd[:name] %></h5>
    <div class="card-text">
     <div>
      <span class="badge rounded-pill text-bg-primary"><%= sd[:faction] %></span>
      <span class="badge rounded-pill text-bg-primary">Class <%= sd[:spobclass] %></span>
     </div>
     <div>
     <% sd[:services].each do |s| %>
      <span class="badge rounded-pill text-bg-secondary"><%= s %></span>
     <% end %>
     </div>
     <div>
     <% sd[:tags].each do |t| %>
      <span class="badge rounded-pill text-bg-info"><%= t %></span>
     <% end %>
     </div>
    </div>
   </div>
  </div>
 </div>
 <!-- Modal -->
 <div class="modal fade" id="modal-<%= sd[:id] %>" tabindex="-1" aria-labelledby="modal-label-<%= sd[:id] %>" aria-hidden="true">
  <div class="modal-dialog modal-xl modal-dialog-centered modal-dialog-scrollable">
   <div class="modal-content">
    <div class="modal-header">
     <h1 class="modal-title fs-5" id="modal-label-<%= sd[:id] %>"><%= sd[:name] %></h1>
     <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
    </div>
    <div class="modal-body clearfix">
     <% if not sd[:exterior].nil? %>
     <img src="<%= sd[:exterior] %>" class="rounded col-md-6 float-md-end mb-3 ms-md-3" alt="<%= s[:spob][:GFX][:exterior] %>">
     <% elsif not sd[:gfx].nil? %>
     <img src="<%= sd[:gfx] %>" class="col-md-6 float-md-end mb-3 ms-md-3" alt="<%= s[:spob][:GFX][:gfx] %>">
     <% end %>
     <div>
      <span class="badge rounded-pill text-bg-primary"><%= sd[:faction] %></span>
      <span class="badge rounded-pill text-bg-primary">Class <%= sd[:spobclass] %></span>
     </div>
     <div>
     <% sd[:services].each do |s| %>
      <span class="badge rounded-pill text-bg-secondary"><%= s %></span>
     <% end %>
     </div>
     <div>
     <% sd[:tags].each do |t| %>
      <span class="badge rounded-pill text-bg-info"><%= t %></span>
     <% end %>
     </div>
     <% if not sd[:description].nil? %>
     <div>
     <h5>Description:</h5>
     <p><%= sd[:description] %></p>
     </div>
     <% end %>
     <% if not sd[:bar].nil? %>
     <div>
     <h5>Spaceport Bar:</h5>
     <p><%= sd[:bar] %></p>
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
