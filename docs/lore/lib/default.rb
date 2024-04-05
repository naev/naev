# All files in the 'lib' directory will be loaded
# before nanoc starts compiling.

include Nanoc::Helpers::LinkTo
include Nanoc::Helpers::XMLSitemap
include Nanoc::Helpers::ChildParent
include Nanoc::Helpers::Capturing

def spob_get( name )
  return config[:spob][name]
end

def ssys_get( name )
  return config[:ssys][name]
end

def spob_card( s )
  cls = ""
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
    gfx += "<img src='#{gfxpath}' class='card-img-top' alt='#{s[:spob][:GFX][:space]}'>"
  end
<<-EOF
 <div class="col #{cls}" data-Name="#{s[:name]}" data-Faction="#{s[:faction]}" data-Class="#{s[:spobclass]}" data-Population="#{s[:population]}" >
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
    </div>
    <div class="modal-footer">
     <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">Close</button>
    </div>
   </div>
  </div>
 </div>
EOF
end

def ssys_card( s )
  cls = ""
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
 <div class="col #{cls}" data-Name="#{s[:name]}" data-Faction="#{s[:faction]}" >
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

<<-EOF
 <div class="modal fade" id="modal-ssys-#{s[:id]}" tabindex="-1" aria-labelledby="modal-ssys-label-#{s[:id]}" aria-hidden="true">
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
    </div>
    <div class="modal-footer">
     <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">Close</button>
    </div>
   </div>
  </div>
 </div>
EOF
end

def modal_addAll
  out = ""
  @items.find_all('/spob/*.md').sort{ |a,b| a[:name]<=>b[:name] }.each do |s|
    out += spob_modal( s )
  end
  @items.find_all('/ssys/*.md').sort{ |a,b| a[:name]<=>b[:name] }.each do |s|
    out += ssys_modal( s )
  end
  out
end
