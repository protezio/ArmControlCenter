#include <Arduino.h>


String WEB_ACTIONS =  "<a class='w3-bar-item w3-button' href='/'><i class='fa fa-home'></i> Home</a>"
                      "<a class='w3-bar-item w3-button' href='/chart'><i class='fa fa-line-chart'></i> Chart</a>"
                      "<a class='w3-bar-item w3-button' href='/configure'><i class='fa fa-cog'></i> Configure</a>"
                    //  "<a class='w3-bar-item w3-button' href='/configureweather'><i class='fa fa-cloud'></i> Weather</a>"
                      "<a class='w3-bar-item w3-button' href='/systemreset' onclick='return confirm(\"Do you want to reset to default settings?\")'><i class='fa fa-undo'></i> Reset Settings</a>"
                     // "<a class='w3-bar-item w3-button' href='/readsystem'><i class='fa fa-copy'></i> Read System</a>"
                      "<a class='w3-bar-item w3-button' href='/forgetwifi' onclick='return confirm(\"Do you want to forget to WiFi connection?\")'><i class='fa fa-wifi'></i> Forget WiFi</a>"
                      "<a class='w3-bar-item w3-button' href='/update'><i class='fa fa-wrench'></i> Firmware Update</a>"
                      ;

String CHANGE_FORM =  "<form class='w3-container' action='/updateconfig' method='get'><h2>Planton Config:</h2>"
                      "<br>"
                      "<hr>"
                      "<p>Refresh (seconds) <select class='w3-option w3-padding' name='refresh'> %OPTIONS%</select></p>"
                      "<hr>"
                      "<h3>Timing chart:</h3>"
                      "<div class='w3-container'>"
                      "<table class='w3-table w3-bordered'>"
                      "<tr>"
                      "<th>ON/OFF</th>"
                      "<th>START [hh:mm]</th>"
                      "<th>STOP  [hh:mm]</th>"
                      "</tr>"

                      "<tr>"
                      "<td><input name='cb_lun' class='w3-check' type='checkbox' %LUN%><label> Monday</label></td>"
                      "<td><input class='w3-input w3-border' type='text' name='startLun' value='%start_lun%' maxlength='5'></td>"
                      "<td><input class='w3-input w3-border' type='text' name='stopLun' value='%stop_lun%'></td>"
                      "</tr>"

                      "<tr>"
                      "<td><input name='cb_mar' class='w3-check' type='checkbox' %MAR%><label> Tuesday</label></td>"
                      "<td><input class='w3-input w3-border' type='text' name='startMar' value='%start_mar%' maxlength='5'></td>"
                      "<td><input class='w3-input w3-border' type='text' name='stopMar' value='%stop_mar%' maxlength='5'></td>"
                      "</tr>"

                      "<tr>"
                      "<td><input name='cb_mer' class='w3-check' type='checkbox' %MER%><label> Wednesday</label></td>"
                      "<td><input class='w3-input w3-border' type='text' name='startMer' value='%start_mer%' maxlength='5'></td>"
                      "<td><input class='w3-input w3-border' type='text' name='stopMer' value='%stop_mer%' maxlength='5'></td>"
                      "</tr>"

                      "<tr>"
                      "<td><input name='cb_gio' class='w3-check' type='checkbox' %GIO%><label> Thursday</label></td>"
                      "<td><input class='w3-input w3-border' type='text' name='startGio' value='%start_gio%' maxlength='5'></td>"
                      "<td><input class='w3-input w3-border' type='text' name='stopGio' value='%stop_gio%' maxlength='5'></td>"
                      "</tr>"

                      "<tr>"
                      "<td><input name='cb_ven' class='w3-check' type='checkbox' %VEN%><label> Friday</label></td>"
                      "<td><input class='w3-input w3-border' type='text' name='startVen' value='%start_ven%' maxlength='5'></td>"
                      "<td><input class='w3-input w3-border' type='text' name='stopVen' value='%stop_ven%' maxlength='5'></td>"
                      "</tr>"

                      "<tr>"
                      "<td><input name='cb_sab' class='w3-check' type='checkbox' %SAB%><label> Saturday</label></td>"
                      "<td><input class='w3-input w3-border' type='text' name='startSab' value='%start_sab%' maxlength='5'></td>"
                      "<td><input class='w3-input w3-border' type='text' name='stopSab' value='%stop_sab%' maxlength='5'></td>"
                      "</tr>"

                      "<tr>"
                      "<td><input name='cb_dom' class='w3-check' type='checkbox' %DOM%><label> Sunday</label></td>"
                      "<td><input class='w3-input w3-border' type='text' name='startDom' value='%start_dom%' maxlength='5'></td>"
                      "<td><input class='w3-input w3-border' type='text' name='stopDom' value='%stop_dom%' maxlength='5'></td>"
                      "</tr>"

                      "</table>"
                      "</div>"  
                      "<hr>"
                      "<button class='w3-button w3-block w3-green w3-section w3-padding' type='submit' onclick='return confirm(\"Data Saved\")'>SAVE</button></form>"
                      "<hr>"
                      "<br>"
                      ; 


String THEME_FORM =  ""; /*"<p>Theme Color <select class='w3-option w3-padding' name='theme'>%THEME_OPTIONS%</select></p>"//;
                      "<p><label>UTC Time Offset</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='utcoffset' value='%UTCOFFSET%' maxlength='12'></p><hr>"
                      "<p><input name='isBasicAuth' class='w3-check w3-margin-top' type='checkbox' %IS_BASICAUTH_CHECKED%> Use Security Credentials for Configuration Changes</p>"
                      "<p><label>User ID (for this interface)</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='userid' value='%USERID%' maxlength='20'></p>"
                      "<p><label>Password </label><input class='w3-input w3-border w3-margin-bottom' type='password' name='stationpassword' value='%STATIONPASSWORD%'></p>"*/

String GRAPH = "";                  

String COLOR_THEMES = "<option>red</option>"
                      "<option>pink</option>"
                      "<option>purple</option>"
                      "<option>deep-purple</option>"
                      "<option>indigo</option>"
                      "<option>blue</option>"
                      "<option>light-blue</option>"
                      "<option>cyan</option>"
                      "<option>teal</option>"
                      "<option>green</option>"
                      "<option>light-green</option>"
                      "<option>lime</option>"
                      "<option>khaki</option>"
                      "<option>yellow</option>"
                      "<option>amber</option>"
                      "<option>orange</option>"
                      "<option>deep-orange</option>"
                      "<option>blue-grey</option>"
                      "<option>brown</option>"
                      "<option>grey</option>"
                      "<option>dark-grey</option>"
                      "<option>black</option>"
                      "<option>w3schools</option>";