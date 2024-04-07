---
title: Missions
---

<h3>Campaigns</h3>
<div class="row row-cols-1 row-cols-md-5 g-4" id="spobs">
<%= out = ""
@items.find_all('/cmpn/**/*.md').each do |c| #***
    out += <<-EOF
 <div class="col" data-Name="#{c[:name]}">
  <div class="card bg-black">
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
EOF
end
out %>
</div>

<h3>Missions</h3>
