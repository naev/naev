# All files in the 'lib' directory will be loaded
# before nanoc starts compiling.

include Nanoc::Helpers::LinkTo
include Nanoc::Helpers::XMLSitemap
include Nanoc::Helpers::ChildParent
include Nanoc::Helpers::Capturing
require 'securerandom'

def spob_get( name )
   return config[:spob][name]
end

def ssys_get( name )
   return config[:ssys][name]
end

def faction_get( name )
   return config[:fcts][name]
end

def npc_get( name )
   return config[:npcs][name]
end

def listmisnevt( header, lst )
   if lst.length() > 0
      out = "<div>"
      out += "<h5>#{header}</h5>"
      out += "<ul>"
      lst.each_with_index do |m,i|
         id = SecureRandom.uuid
         out += "<li data-bs-toggle='collapse' data-bs-target='##{id}' role='button' aria-expanded='false' aria-control='#{id}'>#{m[:name]}</li>"
         out += "<div class='collapse' id='#{id}'>"
         out +=   "<div class='card card-body'>"
         out +=    "<ul>"
         m[:lines].each do |l|
            out += "<li><code>#{l}</code></li>"
         end
         out +=    "</ul>"
         out +=   "</div>"
         out += "</div>"
      end
      out += "</ul>"
      out += "</div>"
   else
      out = ""
   end
   return out
end

def fcts_card( f )
   cls = ""
   if f[:tags].include? 'spoiler'
      cls += " spoiler d-none"
   end

   tags = ""
   if f[:tags].length() > 0
      tags = "<div class='m-1'>"
      f[:tags].each do |t|
         tags += "<span class='badge rounded-pill text-bg-info'>#{t}</span>"
      end
      tags += "</div>"
   end

   gfx = ""
   if not f[:logo].nil?
      gfxpath = relative_path_to(@items[f[:logo]])
      gfx += "<div class='ratio ratio-1x1'>"
      gfx += "<div class='container d-flex align-items-center'>"
      gfx += "<img src='#{gfxpath}' class='card-img-top object-fit-scale mh-100' alt=\"#{f[:longname]} Logo\">"
      gfx += "</div>"
      gfx += "</div>"
   end
<<-EOF
 <div class="col#{cls}" data-Name="#{f[:name]}">
  <div class="card bg-black" data-bs-toggle="modal" data-bs-target="#modal-fcts-#{f[:id]}" >
   #{gfx}
    <div class="card-body">
     <h5 class="card-title">#{f[:name]}</h5>
     <div class="card-text">
      #{tags}
     </div>
   </div>
  </div>
 </div>
EOF
end

def fcts_modal( f )
   gfx = ""
   if not f[:logo].nil?
      gfxpath = relative_path_to(f[:logo])
      gfx += "<img src='#{gfxpath}' class='col-md-6 float-md-end mb-3 ms-md-3' alt=\"#{f[:longname]} Logo\">"
   end

   tags = ""
   if f[:tags].length() > 0
      tags = "<div class='m-1'>"
      f[:tags].each do |t|
         tags += "<span class='badge rounded-pill text-bg-info'>#{t}</span>"
      end
      tags += "</div>"
   end

<<-EOF
 <div class="modal fade fcts" id="modal-fcts-#{f[:id]}" tabindex="-1" aria-labelledby="modal-fcts-label-#{f[:id]}" data-fcts-modal="#{f[:name]}" aria-hidden="true">
  <div class="modal-dialog modal-xl modal-dialog-centered modal-dialog-scrollable">
   <div class="modal-content">
    <div class="modal-header">
     <h1 class="modal-title fs-5" id="modal-fcts-label-#{f[:id]}">#{f[:longname]}</h1>
     <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
    </div>
    <div class="modal-body clearfix">
     #{gfx}
     #{tags}
     <p>#{f[:description]}</p>
    </div>
   </div>
  </div>
 </div>
EOF
end

def spob_card( s )
   cls = ""
   if s[:tags].include? 'spoiler'
      cls += " spoiler d-none"
   end
   cls += " fct-"+s[:faction]
   cls += " cls-"+s[:spobclass]
   s[:services].each do |s|
      cls += " srv-"+s.to_s
   end
   s[:tags].each do |t|
      cls += " tag-"+t
   end

   services = ""
   if s[:services].length() > 0
      services += "<div>"
      s[:services].each do |srv|
         services += "<span class='badge rounded-pill text-bg-secondary'>#{srv}</span>"
      end
      services += "</div>"
   end
   tags = ""
   if s[:tags].length() > 0
      tags = "<div>"
      s[:tags].each do |t|
         tags += "<span class='badge rounded-pill text-bg-info'>#{t}</span>"
      end
      tags += "</div>"
   end

   if not s[:spob][:GFX].nil? and not s[:spob][:GFX][:space].nil?
      gfxpath = relative_path_to(@items["/gfx/spob/space/"+s[:spob][:GFX][:space]])
   end
   gfx = ""
   if not gfxpath.nil?
      gfx += "<div class='ratio ratio-1x1'>"
      gfx += "<div class='container d-flex align-items-center'>"
      gfx += "<img src='#{gfxpath}' class='card-img-top object-fit-scale mh-100' alt='#{s[:spob][:GFX][:space]}'>"
      gfx += "</div>"
      gfx += "</div>"
   end
<<-EOF
 <div class="col#{cls}" data-Name="#{s[:name]}" data-Faction="#{s[:faction]}" data-Class="#{s[:spobclass]}" data-Population="#{s[:population]}" >
  <div class="card bg-black" data-bs-toggle="modal" data-bs-target="#modal-spob-#{s[:id]}" >
   #{gfx}
   <div class="card-body">
    <h5 class="card-title">#{s[:name]}</h5>
    <div class="card-text">
     <div>
      <span class="badge rounded-pill text-bg-primary">#{s[:faction]}</span>
      <span class="badge rounded-pill text-bg-primary">Class #{s[:spobclass]}</span>
     </div>
     #{services}
     #{tags}
    </div>
   </div>
  </div>
 </div>
EOF
end

def spob_modal( s )
   header = s[:name]
   if not s[:ssys].nil?
      header += " (#{ssys_get(s[:ssys])[:name]} system)"
   end

   gfx = ""
   if not s[:spob][:GFX].nil? and not s[:spob][:GFX][:exterior].nil?
      gfxpath = relative_path_to(@items["/gfx/spob/exterior/"+s[:spob][:GFX][:exterior]])
      gfx += "<img src='#{gfxpath}' class='rounded col-md-6 float-md-end mb-3 ms-md-3' alt='#{s[:spob][:GFX][:space]}'>"
   elsif not s[:spob][:GFX].nil? and not s[:spob][:GFX][:space].nil?
      gfxpath = relative_path_to(@items["/gfx/spob/space/"+s[:spob][:GFX][:space]])
      gfx += "<img src='#{gfxpath}' class='col-md-6 float-md-end mb-3 ms-md-3' alt='#{s[:spob][:GFX][:space]}'>"
   end

   services = ""
   if s[:services].length() > 0
      services += "<div class='m-1'>"
      s[:services].each do |srv|
         services += "<span class='badge rounded-pill text-bg-secondary'>#{srv}</span>"
      end
      services += "</div>"
   end

   tags = ""
   if s[:tags].length() > 0
      tags = "<div class='m-1'>"
      s[:tags].each do |t|
         tags += "<span class='badge rounded-pill text-bg-info'>#{t}</span>"
      end
      tags += "</div>"
   end

   description = ""
   if not s[:description].nil?
      description += <<-EOF
      <div>
       <h5>Description:</h5>
       <p>#{s[:description]}</p>
      </div>
      EOF
   end

   bar = ""
   if not s[:bar].nil?
      bar += <<-EOF
       <div>
         <h5>Spaceport Bar:</h5>
         <p>#{s[:bar]}</p>
       </div>
      EOF
   end

   # Add mission / event stuff
   missions = listmisnevt( "Appears in the following missions:", s[:missions] )
   events = listmisnevt( "Appears in the following events:", s[:events] )

<<-EOF
 <div class="modal fade spob" id="modal-spob-#{s[:id]}" tabindex="-1" aria-labelledby="modal-spob-label-#{s[:id]}" data-spob-modal="#{s[:name]}" aria-hidden="true">
  <div class="modal-dialog modal-xl modal-dialog-centered modal-dialog-scrollable">
   <div class="modal-content">
    <div class="modal-header">
     <h1 class="modal-title fs-5" id="modal-spob-label-#{s[:id]}">#{header}</h1>
     <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
    </div>
    <div class="modal-body clearfix">
     #{gfx}
    <div class='m-1'>
     <span class="badge rounded-pill text-bg-primary">#{s[:faction]}</span>
     <span class="badge rounded-pill text-bg-primary">Class #{s[:spobclass]}</span>
    </div>
     #{services}
     #{tags}
     #{description}
     #{bar}
     #{missions}
     #{events}
    </div>
   </div>
  </div>
 </div>
EOF
end

def ssys_card( s )
   cls = ""
   if s[:tags].include? 'spoiler'
      cls += " spoiler d-none"
   end
   s[:factions].each do |f|
      cls += " fct-"+f
   end
   s[:tags].each do |t|
      cls += " tag-"+t
   end

   factions = ""
   if s[:factions].length() > 0
      factions = "<div>"
      s[:factions].each do |f|
         factions += "<span class='badge rounded-pill text-bg-primary'>#{f}</span>"
      end
      factions += "</div>"
   end

   tags = ""
   if s[:tags].length() > 0
      tags = "<div>"
      s[:tags].each do |t|
            tags += "<span class='badge rounded-pill text-bg-info'>#{t}</span>"
      end
      tags += "</div>"
   end

<<-EOF
 <div class="col#{cls}" data-Name="#{s[:name]}" data-Faction="#{s[:faction]}" >
  <div class="card bg-black" data-bs-toggle="modal" data-bs-target="#modal-ssys-#{s[:id]}" >
   <div class="card-body">
    <h5 class="card-title">#{s[:name]}</h5>
    <div class="card-text">
     #{factions}
     #{tags}
     <span class='badge rounded-pill text-bg-warning'>#{s[:spobs].length} Space Objects</span>
    </div>
   </div>
  </div>
 </div>
EOF
end

def ssys_modal( s )

   factions = ""
   if s[:factions].length() > 0
      factions = "<div class='m-1'>"
      s[:factions].each do |f|
         factions += "<span class='badge rounded-pill text-bg-primary'>#{f}</span>"
      end
      factions += "</div>"
   end

   tags = ""
   if s[:tags].length() > 0
      tags = "<div class='m-1'>"
      s[:tags].each do |t|
         tags += "<span class='badge rounded-pill text-bg-info'>#{t}</span>"
      end
      tags += "</div>"
   end

   if s[:spobs].length() > 0
      spobs = "<div class='row row-cols-1 row-cols-md-5 g-4'>"
      s[:spobs].each do |spb|
         spobs += spob_card( spob_get(spb) )
      end
      spobs += "</div>"
   else
      spobs = "<div>This system has no space objects.</div>"
   end

   # Add mission / event stuff
   missions = listmisnevt( "Appears in the following missions:", s[:missions] )
   events = listmisnevt( "Appears in the following events:", s[:events] )

<<-EOF
 <div class="modal fade" id="modal-ssys-#{s[:id]}" tabindex="-1" aria-labelledby="modal-ssys-label-#{s[:id]}" data-Name="#{s[:name]}" aria-hidden="true">
  <div class="modal-dialog modal-xl modal-dialog-centered modal-dialog-scrollable">
   <div class="modal-content">
    <div class="modal-header">
     <h1 class="modal-title fs-5" id="modal-ssys-label-#{s[:id]}">#{s[:name]}</h1>
     <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
    </div>
    <div class="modal-body clearfix">
     #{factions}
     #{tags}
     #{spobs}
     #{missions}
     #{events}
    </div>
   </div>
  </div>
 </div>
EOF
end

def npc_card( n )
   cls = ""
   if n[:tags].include? 'spoiler'
      cls += " spoiler d-none"
   end

   tags = ""
   if n[:tags].length() > 0
      tags = "<div class='m-1'>"
      n[:tags].each do |t|
         tags += "<span class='badge rounded-pill text-bg-info'>#{t}</span>"
      end
      tags += "</div>"
   end

   gfx = ""
   if not n[:gfx].nil?
      portrait = @items["/gfx/portraits/"+n[:gfx]]
      if portrait.nil?
         gfxpath = relative_path_to(@items["/gfx/vn/characters/"+n[:gfx]])
      else
         gfxpath = relative_path_to(portrait)
      end
      gfx += "<div class='container d-flex align-items-center'>"
      gfx += "<img src='#{gfxpath}' class='card-img-top object-fit-scale mh-100' alt=\"#{n[:name]} Portrait\">"
      gfx += "</div>"
   end
<<-EOF
 <div class="col#{cls}" data-Name="#{n[:name]}">
   <div class="card bg-black" data-bs-toggle="modal" data-bs-target="#modal-npcs-#{n[:id]}" >
    #{gfx}
    <div class="card-body">
      <h5 class="card-title">#{n[:name]}</h5>
      <div class="card-text">
       #{tags}
      </div>
    </div>
   </div>
 </div>
EOF
end

def npc_modal( n )
   gfx = ""
   if not n[:gfx].nil?
      vn = @items["/gfx/vn/characters/"+n[:gfx]]
      if vn.nil?
         gfxpath = relative_path_to(@items["/gfx/portraits/"+n[:gfx]])
      else
         gfxpath = relative_path_to(vn)
      end
      gfx += "<img src='#{gfxpath}' class='col-md-6 float-md-end mb-3 ms-md-3' alt=\"#{n[:name]} Image\">"
   end

   tags = ""
   if n[:tags].length() > 0
      tags = "<div class='m-1'>"
      n[:tags].each do |t|
         tags += "<span class='badge rounded-pill text-bg-info'>#{t}</span>"
      end
      tags += "</div>"
   end

   # Add mission / event stuff
   missions = listmisnevt( "Appears in the following missions:", n[:missions] )
   events = listmisnevt( "Appears in the following events:", n[:events] )

<<-EOF
 <div class="modal fade npcs" id="modal-npcs-#{n[:id]}" tabindex="-1" aria-labelledby="modal-npcs-label-#{n[:id]}" data-npcs-modal="#{n[:name]}" aria-hidden="true">
  <div class="modal-dialog modal-xl modal-dialog-centered modal-dialog-scrollable">
   <div class="modal-content">
    <div class="modal-header">
     <h1 class="modal-title fs-5" id="modal-npcs-label-#{n[:id]}">#{n[:name]}</h1>
     <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
    </div>
    <div class="modal-body clearfix">
    #{gfx}
    #{tags}
    <div class="container" markdown="1">#{n.compiled_content(snapshot: :raw)}</div>
     #{missions}
     #{events}
    </div>
   </div>
  </div>
 </div>
EOF
end

def modal_addAll
   out = ""
   @items.find_all('/spob/*.md').each do |s|
      out += spob_modal( s )
   end
   @items.find_all('/ssys/*.md').each do |s|
      out += ssys_modal( s )
   end
   @items.find_all('/fcts/*.md').each do |f|
      out += fcts_modal( f )
   end
   @items.find_all('/npcs/*.md').each do |n|
      out += npc_modal( n )
   end
   out
end
