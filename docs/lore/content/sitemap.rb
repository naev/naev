<%= xml_sitemap :items => @items.reject{ |i|
      i[:is_hidden] ||
      (i.binary? and not i[:extension] == 'pdf') ||
      i.identifier.to_s.match( /.(js|sass|scss|css)$/ ) ||
      i.identifier == '/sitemap.xml'
} %>
