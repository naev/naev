//= require js/_jquery.js
//= require js/_bootstrap.js
//= require js/_debounce.js
//= require js/_fluidbox.js

function spoilers () {
   return localStorage.getItem('spoilers') === "true";
}

function spoilers_update () {
   if (spoilers()) {
      $('.spoiler').removeClass('d-none');
      $('.nospoiler').addClass('d-none');
   }
   else {
      $('.spoiler').addClass('d-none');
      $('.nospoiler').removeClass('d-none');
   }
}

$(document).ready( function() {
   $(".use-fluidbox").fluidbox();

   $('input#spoilers').attr('checked', spoilers());
   spoilers_update();

   const toastElList = document.querySelectorAll('.toast')
   const toastList = [...toastElList].map(toastEl => new bootstrap.Toast(toastEl))

   $('input#spoilers').change( function () {
      var checked = $(this).is(':checked');
      localStorage.setItem('spoilers', checked);
      spoilers_update();
      if (checked) {
         bootstrap.Toast.getOrCreateInstance( document.getElementById('toast-spoiler') ).show();
         bootstrap.Toast.getOrCreateInstance( document.getElementById('toast-nospoiler') ).hide();
      }
      else {
         bootstrap.Toast.getOrCreateInstance( document.getElementById('toast-spoiler') ).hide();
         bootstrap.Toast.getOrCreateInstance( document.getElementById('toast-nospoiler') ).show();
      }
   } );

});
