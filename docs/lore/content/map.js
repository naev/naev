//= require js/_sigma.js
//= require js/_graphology.umd.js

function starmapLabel( context, data, settings ) {
   if (!data.label) return;

   const size = data.labelSize || settings.labelSize;
   const font = settings.labelFont;
   const weight = settings.labelWeight;
   const color = data.labelColor || settings.labelColor.color;

   context.fillStyle = color;
   context.font = `${weight} ${size}px ${font}`;

   context.fillText(data.label, data.x + data.size + 3, data.y + size / 3);
}

/**
 * Draw an hovered node.
 * - if there is no label => display a shadow on the node
 * - if the label box is bigger than node size => display a label box that contains the node with a shadow
 * - else node with shadow and the label box
 */
function starmapHover( context, data, settings ) {
   const size = data.labelSize || settings.labelSize;
   const font = settings.labelFont;
   const weight = settings.labelWeight;

   data = { ...data, label: data.label || data.hoverLabel };

   context.font = `${weight} ${size}px ${font}`;

   // Then we draw the label background
   context.fillStyle = '#000';
   context.shadowOffsetX = 0;
   context.shadowOffsetY = 0;
   context.shadowBlur = 8;
   context.shadowColor = '#FFF';

   const PADDING = 2;

   if (typeof data.label === 'string') {
      const textWidth = context.measureText(data.label).width;
      const boxWidth = Math.round(textWidth + 5);
      const boxHeight = Math.round(size + 2 * PADDING);
      const radius = Math.max(data.size, size / 2) + PADDING;

      const angleRadian = Math.asin(boxHeight / 2 / radius);
      const xDeltaCoord = Math.sqrt(
         Math.abs(Math.pow(radius, 2) - Math.pow(boxHeight / 2, 2))
      );

      context.beginPath();
      context.moveTo(data.x + xDeltaCoord, data.y + boxHeight / 2);
      context.lineTo(data.x + radius + boxWidth, data.y + boxHeight / 2);
      context.lineTo(data.x + radius + boxWidth, data.y - boxHeight / 2);
      context.lineTo(data.x + xDeltaCoord, data.y - boxHeight / 2);
      context.arc(data.x, data.y, radius, angleRadian, -angleRadian);
      context.closePath();
      context.fill();
   } else {
      context.beginPath();
      context.arc(data.x, data.y, data.size + PADDING, 0, Math.PI * 2);
      context.closePath();
      context.fill();
   }

   context.shadowOffsetX = 0;
   context.shadowOffsetY = 0;
   context.shadowBlur = 0;

   // And finally we draw the label
   starmapLabel( context, data, settings );
}
