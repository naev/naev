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
factionlist = Set[]
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
%>
 <!-- Card -->
 <%= card_spob( s ) %>
 <!-- Modal -->
 <%= modal_spob( s ) %>
<% end %>
</div>
