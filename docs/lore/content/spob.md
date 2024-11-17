---
title: Space Objects (Spobs)
---
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

$('div.modal.spob').on('shown.bs.modal', function (e) {
    let name = $(this).data("spob-modal");
    history.pushState({ spob: name }, "Naev - "+name, "?spob="+name);
})
$('div.modal.spob').on('hidden.bs.modal', function (e) {
    //history.pushState({ spob: "" }, "Naev - Space Objects", "");
    history.back()
})

function update_spobs () {
    $("#spobs").children(".col").removeClass("d-none");
    $("#spobs").children(".col").each( function () {
        $(this).data("show-tag",0);
    } );
    $('input.filter-faction').each( function () {
        var fct = $(this).data('faction');
        var checked = $(this).is(":checked");
        if (!checked) {
            $("#spobs").children(".col[data-Faction=\""+fct+"\"]").addClass("d-none");
        }
    } );
    $('input.filter-tag').each( function () {
        var tag = $(this).data('tag');
        var checked = $(this).is(":checked");
        if (checked) {
            $("#spobs").children(".col.tag-"+tag).data("show-tag",1);
        }
    } );
    $("#spobs").children(".col").each( function () {
        if ($(this).data("show-tag") <= 0) {
            $(this).addClass("d-none");
        }
    } );
}

$('input#fct-all').change( function () {
    var checked = $(this).is(":checked");
    $('input.filter-faction').each( function () {
        if (checked) {
            $("input#fct-none").prop("checked",false);
            $("input.filter-faction").prop("checked",true);
        }
    } );
    update_spobs();
} );
$('input#fct-none').change( function () {
    var checked = $(this).is(":checked");
    if (checked) {
        $("input#fct-all").prop("checked",false);
        $("input.filter-faction").prop("checked",false);
    }
    update_spobs();
} );
$('input.filter-faction').change( function () {
    var checked = $(this).is(":checked");
    if (checked) {
        $("input#fct-none").prop("checked",false);
    }
    update_spobs();
} );

$('input#tag-all').change( function () {
    var checked = $(this).is(":checked");
    $('input.filter-tag').each( function () {
        if (checked) {
            $("input#tag-none").prop("checked",false);
            $("input.filter-tag").prop("checked",true);
        }
    } );
    update_spobs();
} );
$('input#tag-none').change( function () {
    var checked = $(this).is(":checked");
    if (checked) {
        $("input#tag-all").prop("checked",false);
        $("input.filter-tag").prop("checked",false);
    }
    update_spobs();
} );
$('input.filter-tag').change( function () {
    var checked = $(this).is(":checked");
    if (checked) {
        $("input#tag-none").prop("checked",false);
    }
    update_spobs();
} );

// Open modal on new window
window.onload = function(e){
    let params = new URLSearchParams( window.location.search );
    if (params.has('spob')) {
        let spobname = params.get('spob')
        let modal = new bootstrap.Modal( $('div[data-spob-modal="'+spobname+'"]')[0] );
        modal.show();
    }
    update_spobs();
};
</script>

<% end %>
<!-- First get some global stuff. -->
<%
factionlist = Set[]
taglist = Set[]
classlist = Set[]
@items.find_all('/spob/*.md').each do |s| # **
    factionlist.add( s[:faction] )
    s[:tags].each do |t|
        taglist.add( t )
    end
    classlist.add( s[:spobclass] )
end
%>

<div class="container m-3"><div class="row">
 <div id="selection-sort" class="dropdown col-md-auto">
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

 <div id="selection-factions" class="dropdown col-md-auto">
  <button class="btn btn-secondary dropdown-toggle" type="button" data-bs-toggle="dropdown" data-bs-auto-close="outside" aria-expanded="false">
  Filter Factions
  </button>
  <ul class="dropdown-menu">
   <li><span class="dropdown-item form-check form-switch">
    <input class="form-check-input" type="checkbox" role="switch" id="fct-all" checked>
    <label class="form-check-label" for="fct-all">Enable All</label>
   </span></li>
   <li><span class="dropdown-item form-check form-switch">
    <input class="form-check-input" type="checkbox" role="switch" id="fct-none">
    <label class="form-check-label" for="fct-none">Disable All</label>
   </span></li>
   <%= out = ""
    factionlist.each do |f|
        id = Base64.encode64(f)
        out += <<-EOF
   <li><span class="dropdown-item form-check form-switch">
    <input class="form-check-input filter-faction" type="checkbox" role="switch" data-faction="#{f}" id="#{id}" checked>
    <label class="form-check-label" for="#{id}">#{f}</label>
   </span></li>
EOF
     end
     out
   %>
  </ul>
 </div>

 <div id="selection-tags" class="dropdown col-md-auto">
  <button class="btn btn-secondary dropdown-toggle" type="button" data-bs-toggle="dropdown" data-bs-auto-close="outside" aria-expanded="false">
  Filter Tags
  </button>
  <ul class="dropdown-menu">
   <li><span class="dropdown-item form-check form-switch">
    <input class="form-check-input" type="checkbox" role="switch" id="tag-all" checked>
    <label class="form-check-label" for="tag-all">Enable All</label>
   </span></li>
   <li><span class="dropdown-item form-check form-switch">
    <input class="form-check-input" type="checkbox" role="switch" id="tag-none">
    <label class="form-check-label" for="tag-none">Disable All</label>
   </span></li>
   <%= out = ""
    taglist.each do |t|
        id = Base64.encode64(t)
        out += <<-EOF
   <li><span class="dropdown-item form-check form-switch">
    <input class="form-check-input filter-tag" type="checkbox" role="switch" data-tag="#{t}" id="#{id}" checked>
    <label class="form-check-label" for="#{id}">#{t}</label>
   </span></li>
EOF
     end
     out
   %>
  </ul>
 </div>
</div></div>

<!-- Now display all the spobs. -->
<div class="row row-cols-1 row-cols-md-5 g-4" id="spobs">
<% @items.find_all('/spob/*.md').sort{ |a,b| a[:name]<=>b[:name] }.each do |s| #* %>
 <%= spob_card( s ) %>
<% end %>
</div>

<%= modal_addAll() %>
