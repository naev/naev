local luatk = require 'luatk'
local lg = require "love.graphics"
--local lf = require "love.filesystem"
local cmark = require "cmark"

local luatk_markdown = {}

local function _nodestr( node_type )
   local types = {
      [cmark.NODE_DOCUMENT] = "DOCUMENT",
      [cmark.NODE_BLOCK_QUOTE] = "BLOCK_QUOTE",
      [cmark.NODE_LIST] = "LIST",
      [cmark.NODE_ITEM] = "ITEM",
      [cmark.NODE_CODE_BLOCK] = "CODE_BLOCK",
      [cmark.NODE_HTML_BLOCK] = "HTML_BLOCK",
      [cmark.NODE_CUSTOM_BLOCK] = "CUSTOM_BLOCK",
      [cmark.NODE_PARAGRAPH] = "PARAGRAPH",
      [cmark.NODE_HEADING] = "HEADING",
      [cmark.NODE_THEMATIC_BREAK] = "THEMATIC_BREAK",
      [cmark.NODE_FIRST_BLOCK] = "FIRST_BLOCK",
      [cmark.NODE_LAST_BLOCK] = "LAST_BLOCK",
      [cmark.NODE_TEXT] = "TEXT",
      [cmark.NODE_SOFTBREAK] = "SOFTBREAK",
      [cmark.NODE_LINEBREAK] = "LINEBREAK",
      [cmark.NODE_CODE] = "CODE",
      [cmark.NODE_HTML_INLINE] = "HTML_INLINE",
      [cmark.NODE_CUSTOM_INLINE] = "CUSTOM_INLINE",
      [cmark.NODE_EMPH] = "EMPH",
      [cmark.NODE_STRONG] = "STRONG",
      [cmark.NODE_LINK] = "LINK",
      [cmark.NODE_IMAGE] = "IMAGE",
      [cmark.NODE_FIRST_INLINE] = "FIRST_INLINE",
      [cmark.NODE_LAST_INLINE] = "LAST_INLINE",
   }
   return types[node_type]
end

local Markdown = {}
setmetatable( Markdown, { __index = luatk.Widget } )
local Markdown_mt = { __index = Markdown }

function luatk_markdown.newMarkdown( parent, doc, x, y, w, h, options )
   options = options or {}
   local wgt = luatk.newWidget( parent, x, y, w, h )
   setmetatable( wgt, Markdown_mt )
   wgt.type    = "markdown"
   --wgt.canfocus = true

   -- Modify for scrollbar
   w = w-12

   -- Some helpers
   wgt.linkfunc = options.linkfunc

   local deffont = options.font or luatk._deffont or lg.getFont()
   -- TODO load same font family
   local headerfont = options.fontheader or lg.newFont( math.floor(deffont:getHeight()*1.2+0.5) )

   wgt.blocks = {}
   local block = { x = 0, y = 0, w=w, h=0, text = "", font=deffont }
   local ty = 0
   local function block_end ()
      if #block.text <= 0 then
         return
      end
      local f = block.font
      local _mw, t = f:getWrap( block.text, w )
      local bh = #t * f:getLineHeight()
      ty = ty + bh
      table.insert( wgt.blocks, block )
      block = { x = 0, y = ty, w=w, h=bh, text = "", font=deffont }
   end
   wgt.links = {}

   local strong = false
   local emph = false
   local list = cmark.NO_LIST
   local listn
   local linkx, linky
   for cur, entering, node_type in cmark.walk(doc) do
      --print( string.format("%s [%s]", _nodestr(node_type), tostring(entering) ) )
      if node_type == cmark.NODE_PARAGRAPH then
         if not entering then
            block_end()
         end
      elseif node_type == cmark.NODE_HEADING then
         if entering then
            --block.text = block.text .. "#n"
            block.font = headerfont
            if block.y > 0 then
               ty = ty + headerfont:getHeight()
               block.y = ty
            end
         else
            ty = ty + headerfont:getHeight()*0.5
            block_end()
         end
      elseif node_type == cmark.NODE_STRONG then
         strong = entering
         if strong and not emph then
            block.text = block.text .. "#n"
         elseif not emph and not strong then
            block.text = block.text .. "#0"
         end
      elseif node_type == cmark.NODE_EMPH then
         emph = entering
         if emph and not strong then
            block.text = block.text .. "#n"
         elseif not emph and not strong then
            block.text = block.text .. "#0"
         end
      elseif node_type == cmark.NODE_LINK then
         if entering then
            local bx, by = naev.gfx.printfEnd( block.font.font, block.text, w )
            block.text = block.text .. "#b"
            linkx = block.x+bx
            linky = block.y+by
         else
            block.text = block.text .. "#0"
            local bx, by = naev.gfx.printfEnd( block.font.font, block.text, w )
            local target = cmark.node_get_url(cur)
            local fh = block.font:getHeight()
            local flh = block.font:getLineHeight()
            if block.y+by == linky then
               -- One line, great!
               table.insert( wgt.links, {
                  x1 = linkx-3,
                  y1 = linky-3,
                  x2 = block.x+bx+3,
                  y2 = block.y+by+fh+3,
                  target = target,
               } )
            else
               -- Two lines
               table.insert( wgt.links, {
                  x1 = linkx-3,
                  y1 = linky-3,
                  x2 = w+3,
                  y2 = linky+fh+3,
                  target = target,
               } )
               --  More than 2 lines, bad!
               if block.y+by > linky + flh then
                  table.insert( wgt.links, {
                     x1 = -3,
                     y1 = linky+flh-3,
                     x2 = w+3,
                     y2 = block.y+by-flh+fh+3,
                     target = target,
                  } )
               end
               table.insert( wgt.links, {
                  x1 = -3,
                  y1 = block.y+by-3,
                  x2 = block.x+bx+3,
                  y2 = block.y+by+fh+3,
                  target = target,
               } )
            end
            linkx = nil
            linky = nil
         end
      elseif node_type == cmark.NODE_LIST then
         if entering then
            list = cmark.node_get_list_type( cur )
            listn = 1
         else
            list = cmark.NO_LIST
         end
         ty = ty + headerfont:getHeight()
         block.y = ty
      elseif node_type == cmark.NODE_ITEM then
         if entering then
            if list == cmark.BULLET_LIST then
               block.text = block.text.."・"
            elseif list == cmark.ORDERED_LIST then
               block.text = block.text..string.format(p_("cmark_list","%d. "),listn)
            end
            listn = listn + 1
         end
      elseif node_type == cmark.NODE_FIRST_INLINE or node_type == cmark.NODE_LAST_INLINE then
         local str = cmark.node_get_literal(cur)
         if str then
            block.text = block.text .. _(str)
         end
      elseif node_type == cmark.NODE_IMAGE then
         print("IMAGE")
         print( cmark.node_get_url(cur) )
      end
      --print( cmark.node_get_url(cur) )
      --print( cmark.node_get_literal(cur) )
      --print( cmark.node_get_type_string(cur) )
   end

   -- Compute full size
   wgt.scrollh = 0
   if #wgt.blocks > 0 then
      local b = wgt.blocks[#wgt.blocks]
      wgt.scrollh = b.y+b.h
   end
   wgt.scrollh = math.max( 0, wgt.scrollh-wgt.h )
   if wgt.scrollh > 0 then
      wgt.scrolls = true
   end
   wgt.pos = 0

   return wgt
end
function Markdown:draw( wx, wy )
   local x, y, w, h = wx+self.x, wy+self.y, self.w, self.h

   -- Space for scrollbar
   w = w-12

   -- Draw scrollbar
   if self.scrolls then
      local scroll_pos = self.pos / self.scrollh
      luatk.drawScrollbar( x+w, y, 12, h, scroll_pos )
   end

   local scs = lg.getScissor()
   lg.setScissor( x, y, w, h )
   y = y - self.pos

   --[[
   lg.setColour( 1, 0, 0 )
   for k,l in ipairs(self.links) do
      lg.rectangle( "line", x+l.x1, y+l.y1, l.x2-l.x1, l.y2-l.y1 )
   end
   --]]

   -- Draw the text
   lg.setColour( 1, 1, 1 )
   for k,b in ipairs(self.blocks) do
      local bx, by = x+b.x, y+b.y
      lg.printf( b.text, b.font, bx, by, w, "left" )
   end

   lg.setScissor( scs )
end
function Markdown:drawover( bx, by )
   local x, y = bx+self.x, by+self.y-self.pos
   for k,l in ipairs(self.links) do
      if l.mouseover then
         lg.rectangle( "line", x+l.x1, y+l.y1, l.x2-l.x1, l.y2-l.y1 )
      end
   end
end
local function _checkbounds( l, mx, my )
   return not (mx < l.x1 or mx > l.x2 or my < l.y1 or my > l.y2)
end
local scrollbar_h = 30
function Markdown:mmoved( mx, my )
   if self.scrolling then
      self:setPos( (my-scrollbar_h*0.5) / (self.h-scrollbar_h) )
   end

   -- Correct location for scrolling
   my = my + self.pos

   for k,l in ipairs(self.links) do
      local m = l.mouseover
      l.mouseover = _checkbounds( l, mx, my )
      if m ~= l.mouseover then
         luatk.rerender()
      end
   end
end
function Markdown:pressed( mx, my )
   -- Check scrollbar first
   if self.scrolls and mx > self.w-12 then
      self:setPos( (my-scrollbar_h*0.5) / (self.h-scrollbar_h) )
      self.scrolling = true
      return true
   end

   -- Correct location for scrolling
   my = my + self.pos

   -- Check links
   for k,l in ipairs(self.links) do
      if _checkbounds( l, mx, my ) and self.linkfunc then
         luatk.rerender()
         self.linkfunc( l.target )
         return true
      end
   end
end
function Markdown:released( _mx, _my )
   self.scrolling = false
end
function Markdown:setPos( pos )
   luatk.rerender()
   self.pos = pos * self.scrollh
   self.pos = math.max( 0, math.min( self.scrollh, self.pos ) )
end
function Markdown:wheelmoved( _mx, my )
   if my > 0 then
      self:setPos( (self.pos - 50) / self.scrollh )
   elseif my < 0 then
      self:setPos( (self.pos + 50) / self.scrollh )
   end
end

return luatk_markdown
