#ifndef WEBPAGES_H
#define WEBPAGES_H
const char index_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
   <head>
      <title>SmartSocket - Welcome</title>
      <style>
         .center{
         width:250px; 
         height:350px; 
         margin: 0 auto;
         }
      </style>
   </head>
   <body>
      <div class="center">
         <h2>Welcome!</h2>
         <h3>Enter your WiFi's credentials</h3>
         <form action="/process-info" method="get">
            <label for="label_ssid">SSID:</label><br>
            <input type="text" id="input_ssid" name="input_ssid"><br>
            <label for="label_pswd">Password:</label><br>
            <input type="text" id="input_pswd" name="input_pswd"><br><br>
            <input type="submit" value="Submit">
         </form>
      </div>
   </body>
</html>

)=====";

const char process_info_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
   <head>
      <script>
         window.alert("Device restarting");
      </script>
   </head>
</html>

)=====";

#endif
