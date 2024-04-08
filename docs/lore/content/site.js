//= require js/_jquery.js
//= require js/_bootstrap.js
//= require js/_debounce.js
//= require js/_fluidbox.js

function spoilers () {
   return localStorage.getItem('spoilers') === "true";
}

function spoilers_update () {
   if (spoilers()) {
      $('.spoiler').show();
      $('.nospoiler').hide();
   }
   else {
      $('.spoiler').hide();
      $('.nospoiler').show();
   }
}

$(document).ready( function() {
   $(".use-fluidbox").fluidbox();

   $('input#spoilers').attr('checked', spoilers());
   spoilers_update();

   $('input#spoilers').change( function () {
      var checked = $(this).is(':checked');
      localStorage.setItem('spoilers', checked);
      spoilers_update();
   } );
});
