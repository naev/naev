---
title: Space Objects (Spobs)
---
<% print("SPACE OBJECTS START") %>

<% content_for :javascript do %>
<script>
let sort = "Name";
let reverse = false;
function sortbydata( d ) {
    let dsort = "data-"+d;
    let $spobs = $('#spobs');
    let $spoblist = $spobs.children(".col").detach();
    if (sort==d) {
        reverse = !reverse;
    }
    sort = d;
    $spoblist.sort( function( a, b ) {
        let ad = a.getAttribute(dsort);
        let bd = b.getAttribute(dsort);
        let c =  (''+ad).localeCompare(bd);
        if (reverse)
            c = -c;
        if (c)
            return c;
        let an = a.getAttribute("data-Name");
        let bn = b.getAttribute("data-Name");
        if (reverse)
            return (''+bn).localeCompare(an);
        else
            return (''+an).localeCompare(bn);
    } );
    $spoblist.appendTo($spobs);
    let dir;
    if (reverse)
        dir = "↓";
    else
        dir = "↑";
    $('button#btn-sort').text("Sort by: "+d+dir);
}
function sortbydatanumber( d ) {
    let dsort = "data-"+d;
    let $spobs = $('#spobs');
    let $spoblist = $spobs.children(".col").detach();
    if (sort==d) {
        reverse = !reverse;
    }
    sort = d;
    $spoblist.sort( function( a, b ) {
        let ad = a.getAttribute(dsort);
        let bd = b.getAttribute(dsort);
        let c =  ad-bd;
        if (reverse)
            c = -c;
        if (c)
            return c;
        let an = a.getAttribute("data-Name");
        let bn = b.getAttribute("data-Name");
        if (reverse)
            return (''+bn).localeCompare(an);
        else
            return (''+an).localeCompare(bn);
    } );
    $spoblist.appendTo($spobs);
    let dir;
    if (reverse)
        dir = "↓";
    else
        dir = "↑";
    $('button#btn-sort').text("Sort by: "+d+dir);
}
function randomize() {
    let $spobs = $('#spobs');
    let $spoblist = $spobs.children(".col").detach();
    $spoblist.sort( function( a, b ) {
        return Math.random() < 0.5;
    } );
    $spoblist.appendTo($spobs);
    sort = "Random";
    reverse = false;
    $('button#btn-sort').text("Sort by: Random");
}

// Open modal on new window
window.onload = function(e){
    let params = new URLSearchParams( window.location.search );
    if (params.has('spob')) {
        let spobname = params.get('spob')
        let modal = new bootstrap.Modal( $('div[data-spob-modal="'+spobname+'"]')[0] );
        modal.show();
    }
};
$('div.modal.spob').on('shown.bs.modal', function (e) {
    let name = $(this).data("spob-modal");
    history.pushState({ spob: name }, "Naev - "+name, "?spob="+name);
})
$('div.modal.spob').on('hidden.bs.modal', function (e) {
    //history.pushState({ spob: "" }, "Naev - Space Objects", "");
    history.back()
})
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
<% @items.find_all('/spob/*.md').sort{ |a,b| a[:name]<=>b[:name] }.each do |s| %> <!--*-->
 <%= card_spob( s ) %>
 <%= modal_spob( s ) %>
<% end %>
</div>
