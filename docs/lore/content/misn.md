---
title: Missions
---

<h3>Campaigns</h3>
<div class="row row-cols-1 row-cols-md-5 g-4" id="spobs">
<%= out = ""
@items.find_all('/cmpn/**/*.md').each do |c| #***
    missions = ""
    c[:missions].each do |m|
        missions += "<li>#{m[:name]}</li>"
    end
    events = ""
    c[:events].each do |e|
        events += "<li>#{e[:name]}</li>"
    end

    out += <<-EOF
 <div class="col" data-Name="#{c[:name]}">
  <div class="card bg-black" data-bs-toggle="modal" data-bs-target="#modal-cmpn-#{c[:id]}">
   <div class="card-body">
    <h5 class="card-title">#{c[:name]}</h5>
    <div class="card-text">
     <div>
      <span class="badge rounded-pill text-bg-primary">#{c[:missions].length()} Missions</span>
      <span class="badge rounded-pill text-bg-primary">#{c[:events].length()} Events</span>
     </div>
    </div>
   </div>
  </div>
 </div>

 <div class="modal fade cmpn" id="modal-cmpn-#{c[:id]}" tabindex="-1" aria-labelledby="modal-cmpn-label-#{c[:id]}" data-cmpn-modal="#{c[:name]}" aria-hidden="true">
  <div class="modal-dialog modal-xl modal-dialog-centered modal-dialog-scrollable">
   <div class="modal-content">
    <div class="modal-header">
     <h1 class="modal-title fs-5" id="modal-cmpn-label-#{c[:id]}">#{c[:name]}</h1>
     <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
    </div>
    <div class="modal-body clearfix">
     <div class='m-1'>
      <span class="badge rounded-pill text-bg-primary">#{c[:missions].length()} Missions</span>
      <span class="badge rounded-pill text-bg-primary">#{c[:events].length()} Events</span>
     </div>
     <div markdown="1">#{c.compiled_content(snapshot: :raw)}</div>
     <div><h5>Missions:</h5>
      <ul>
       #{missions}
      </ul>
     </div>
     <div><h5>Events:</h5>
      <ul>
       #{events}
      </ul>
     </div>
    </div>
    <div class="modal-footer">
     <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">Close</button>
    </div>
   </div>
  </div>
 </div>
EOF
end
out %>
</div>

<h3>Missions</h3>

<%= modal_addAll() %>
