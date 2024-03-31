# All files in the 'lib' directory will be loaded
# before nanoc starts compiling.

include Nanoc::Helpers::LinkTo
include Nanoc::Helpers::Tagging
include Nanoc::Helpers::XMLSitemap
include Nanoc::Helpers::ChildParent
include Nanoc::Helpers::Blogging

require 'nanoc/filters/javascript_concatenator'
require 'image_size'

def image_with_size(item, alt='', class_code='', html_code='')
   begin
      if item.class == Nanoc::CompilationItemView and not item.attributes[:content_filename].nil?
         img = ImageSize.new(IO.read(item.attributes[:content_filename]))
         "<img src='#{item.path}' height='#{img.height}' width='#{img.width}' alt=\"#{alt}\" class='img-fluid #{class_code}' #{html_code}/>"
      else
         img = ImageSize.new(IO.read("output/#{item.path}"))
         "<img src='#{item.path}' height='#{img.height}' width='#{img.width}' alt=\"#{alt}\" class='img-fluid #{class_code}' #{html_code}/>"
      end
   rescue
      "<img src='#{item.path}' alt=\"#{alt}\" class='img-fluid #{class_code}' #{html_code}/>"
   end
end

class Integer
  def ordinalize
    if (11..13).include?(self % 100)
      "#{self}th"
    else
      case self % 10
        when 1; "#{self}st"
        when 2; "#{self}nd"
        when 3; "#{self}rd"
        else    "#{self}th"
      end
    end
  end
end

def video( item, caption, class_code='', aspectratio='16by9' )
  return <<-EOS
<figure class="figure embed-responsive #{class_code}">
 <div class="embed-responsive embed-responsive-#{aspectratio} figure-img">
  <video controls class="embed-responsive-item">
   <source src="#{item.path}" type='video/webm;codecs="vp9, opus"'>
  </video>
 </div>
 <figcaption class="figure-caption">#{caption}</figcaption>
</figure>
EOS
end

def pdf( item, width="100%", height="900" )
  return <<-EOS
<object width="#{width}" height="#{height}" type="application/pdf" data="#{item}?#zoom=85&scrollbar=0&toolbar=0&navpanes=0">
<a href="#{item}">Could not load PDF, Click here to download.</a>
</object>
EOS
end

def youtube( video_id, caption, class_code='', aspectratio='16by9' )
  return <<-EOS
<figure class="figure embed-responsive #{class_code}">
 <div class="embed-responsive embed-responsive-#{aspectratio} figure-img">
  <embed src="https://www.youtube.com/embed/#{video_id}">
 </div>
 <figcaption class="figure-caption">#{caption}</figcaption>
</figure>
EOS
end

def image( item, caption, alignment='center', extra_html='' )
  return <<-EOS
<figure class="figure embed-responsive" style="text-align:#{alignment};">
  <div class="embed-responsive figure-img">
    <img class='img-fluid' alt='#{caption}' src="#{item.path}" #{extra_html} />
  </div>
  <figcaption class="figure-caption">#{caption}</figcaption>
</figure>
EOS
end

# Embeds a badge
# Populate this case with additional badges as the need arises.
# Badges should be stored in imgs/badges/<badge_name>.svg

def badge( type, version='latest' )

  if version == 'latest'
    version=version
  else
    version="v#{version}"
  end

  case type
    when 'steam';
      link = "https://store.steampowered.com/app/598530/Naev"
      badge_image = @items['/imgs/badges/steam.svg'].path
      alt = "Get - Steam"
    when 'flathub';
      link = "https://flathub.org/apps/details/org.naev.Naev"
      badge_image = @items['/imgs/badges/flathub.svg'].path
      alt = "Get - Flathub"
    when 'itchio';
      link = "https://naev.itch.io/naev"
      badge_image = @items['/imgs/badges/itchio.svg'].path
      alt = "Get - Itch.io"
    when 'repology';
      link = "https://repology.org/project/naev/versions"
      badge_image = "https://repology.org/badge/latest-versions/naev.svg"
      alt = "latest packaged version(s)"
    when 'github';
      link = "https://github.com/naev/naev/releases/#{version}"
      badge_image = @items['/imgs/badges/github.svg'].path
      alt = "Get - Github"
    when 'matrix';
      link = "https://matrix.to/#/#naev-community:matrix.org"
      badge_image = @items['/imgs/badges/matrix.svg'].path
      alt = "Join - Matrix"
    when 'discord';
      link = "https://discord.com/invite/nd2M5BR"
      badge_image = @items['/imgs/badges/discord.svg'].path
      alt = "Join - Discord"
    when 'steamdisc';
      link = "https://steamcommunity.com/app/598530/discussions"
      badge_image = @items['/imgs/badges/steamdisc.svg'].path
      alt = "Discussions - Steam"
    when 'githubdisc';
      link = "https://github.com/naev/naev/discussions"
      badge_image = @items['/imgs/badges/githubdisc.svg'].path
      alt = "Discussions - Github"
    when 'issues';
      link = "https://github.com/naev/naev/issues"
      badge_image = @items['/imgs/badges/issues.svg'].path
      alt = "Issue Tracker - Github"
    else
      link = "https://www.youtube.com/watch?v=dQw4w9WgXcQ"
      badge_image = @items['/imgs/badges/error.svg'].path
      alt = "Error - Bad"
  end
  return <<-EOS
[![#{alt}](#{badge_image})](#{link})
EOS
end
