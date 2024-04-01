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
    taglist.add( s[:faction] )
    taglist.add( s[:tags] )
    classlist.add( s[:spobclass] )
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
    if not s[:spob][:GFX].nil? and not s[:spob][:GFX][:space].nil?
        gfx = relative_path_to(@items["/gfx/spob/space/"+s[:spob][:GFX][:space]])
    end
    if not s[:spob][:GFX].nil? and not s[:spob][:GFX][:exterior].nil?
        exterior = relative_path_to(@items["/gfx/spob/exterior/"+s[:spob][:GFX][:exterior]])
    end

    cls = ""
    cls += " fct-"+s[:faction]
    cls += " cls-"+s[:spobclass]
    s[:services].each do |s|
        cls += " srv-"+s.to_s
    end
    s[:tags].each do |t|
        cls += " tag-"+t
    end
%>
 <!-- Card -->
 <div class="col <%= cls %>" data-Name="<%= s[:name] %>" data-Faction="<%= s[:faction] %>" data-Class="<%= s[:spobclass] %>" data-Population="<%= s[:population] %>" >
  <div class="card bg-black" data-bs-toggle="modal" data-bs-target="#modal-<%= s[:id] %>" >
   <% if not gfx.nil? %>
   <img src="<%= gfx %>" class="card-img-top" alt="<%= s[:spob][:GFX][:space] %>">
   <% end %>
   <div class="card-body">
    <h5 class="card-title"><%= s[:name] %></h5>
    <div class="card-text">
     <div>
      <span class="badge rounded-pill text-bg-primary"><%= s[:faction] %></span>
      <span class="badge rounded-pill text-bg-primary">Class <%= s[:spobclass] %></span>
     </div>
     <div>
     <% s[:services].each do |srv| %>
      <span class="badge rounded-pill text-bg-secondary"><%= srv %></span>
     <% end %>
     </div>
     <div>
     <% s[:tags].each do |t| %>
      <span class="badge rounded-pill text-bg-info"><%= t %></span>
     <% end %>
     </div>
    </div>
   </div>
  </div>
 </div>
 <!-- Modal -->
 <div class="modal fade" id="modal-<%= s[:id] %>" tabindex="-1" aria-labelledby="modal-label-<%= s[:id] %>" aria-hidden="true">
  <div class="modal-dialog modal-xl modal-dialog-centered modal-dialog-scrollable">
   <div class="modal-content">
    <div class="modal-header">
     <h1 class="modal-title fs-5" id="modal-label-<%= s[:id] %>"><%= s[:name] %></h1>
     <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
    </div>
    <div class="modal-body clearfix">
     <% if not exterior.nil? %>
     <img src="<%= exterior %>" class="rounded col-md-6 float-md-end mb-3 ms-md-3" alt="<%= s[:spob][:GFX][:exterior] %>">
     <% elsif not gfx.nil? %>
     <img src="<%= gfx %>" class="col-md-6 float-md-end mb-3 ms-md-3" alt="<%= s[:spob][:GFX][:gfx] %>">
     <% end %>
     <div>
      <span class="badge rounded-pill text-bg-primary"><%= s[:faction] %></span>
      <span class="badge rounded-pill text-bg-primary">Class <%= s[:spobclass] %></span>
     </div>
     <div>
     <% s[:services].each do |s| %>
      <span class="badge rounded-pill text-bg-secondary"><%= s %></span>
     <% end %>
     </div>
     <div>
     <% s[:tags].each do |t| %>
      <span class="badge rounded-pill text-bg-info"><%= t %></span>
     <% end %>
     </div>
     <% if not s[:description].nil? %>
     <div>
     <h5>Description:</h5>
     <p><%= s[:description] %></p>
     </div>
     <% end %>
     <% if not s[:bar].nil? %>
     <div>
     <h5>Spaceport Bar:</h5>
     <p><%= s[:bar] %></p>
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
