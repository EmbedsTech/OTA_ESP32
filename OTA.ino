#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>

const char* ssid = "ESP32_AP";
const char* password = "12345678";

WebServer server(80);

void setupAP() {
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void handleRoot() {
  server.send(200, "text/html",
    "<!DOCTYPE html>"
    "<html>"
    "<head>"
    "<style>"
    "body {"
    "  font-family: Arial, sans-serif;"
    "  background-color: #87CEFA;;" /* Blue background */
    "  display: flex;"
    "  justify-content: center;"
    "  align-items: center;"
    "  height: 100vh;"
    "  margin: 0;"
    "}"
    ".box {"
    "  background-color: white;"
    "  padding: 100px;"
    "  border-radius: 20px;"
    "  box-shadow: 0px 4px 10px rgba(0, 0, 0, 0.2);"
    "  text-align: center;"
    "  width: 400px;" /* Wider box */
    "}"
    "input[type='file'] {"
    "  display: block;"
    "  margin: 15px auto;"
    "  padding: 10px;"
    "  border: 1px solid #ccc;"
    "  border-radius: 5px;"
    "  width: 80%;"
    "  text-align: center;"
    "}"
    "input[type='submit'] {"
    "  background-color: #007BFF;" /* Blue button */
    "  color: white;"
    "  border: none;"
    "  padding: 10px 20px;"
    "  text-transform: uppercase;"
    "  border-radius: 5px;"
    "  cursor: pointer;"
    "  width: 80%;" /* Matches file input width */
    "}"
    "input[type='submit']:hover {"
    "  background-color: #0056b3;" /* Darker blue on hover */
    "}"
    ".progress-bar {"
    "  width: 100%;"
    "  background-color: #ddd;"
    "  border-radius: 10px;"
    "  overflow: hidden;"
    "  margin-top: 15px;"
    "  height: 10px;"
    "}"
    ".progress-bar div {"
    "  height: 10px;"
    "  width: 0%;"
    "  background-color: #007BFF;" /* Blue progress bar */
    "  transition: width 0.2s;" /* Smooth movement */
    "}"
    "#message {"
    "  margin-top: 20px;"
    "  font-size: 16px;"
    "  color: green;"
    "}"
    "</style>"
    "</head>"
    "<body>"
    "<div class='box'>"
    "  <h3>Firmware Update</h3>"
    "  <form method='POST' action='/update' enctype='multipart/form-data' id='upload_form'>"
    "    <input type='file' name='update' required>"
    "    <br>"
    "    <input type='submit' value='Update'>"
    "  </form>"
    "  <div class='progress-bar'>"
    "    <div id='progress-bar'></div>"
    "  </div>"
    "  <div id='message'></div>"
    "</div>"
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<script>"
    "$('form').submit(function(e) {"
    "  e.preventDefault();"
    "  var form = $('#upload_form')[0];"
    "  var data = new FormData(form);"
    "  $.ajax({"
    "    url: '/update',"
    "    type: 'POST',"
    "    data: data,"
    "    contentType: false,"
    "    processData: false,"
    "    xhr: function() {"
    "      var xhr = new window.XMLHttpRequest();"
    "      xhr.upload.addEventListener('progress', function(e) {"
    "        if (e.lengthComputable) {"
    "          var percentComplete = (e.loaded / e.total) * 100;"
    "          $('#progress-bar').css('width', percentComplete + '%');"
    "        }"
    "      }, false);"
    "      return xhr;"
    "    },"
    "    success: function() {"
    "      $('#message').html('Update Successful!');" /* Success message */
    "    },"
    "    error: function() {"
    "      $('#message').html('Update Failed!').css('color', 'red');" /* Error message */
    "    }"
    "  });"
    "});"
    "</script>"
    "</body>"
    "</html>");
}


void handleUpdate() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Update: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("Update Success: %u bytes\n", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
  }
}

void handleUpdateResult() {
  if (Update.hasError()) {
    server.send(200, "text/plain", "Update Failed!");
  } else {
    server.send(200, "text/plain", "Update Successful! Rebooting...");
    delay(1000);
    ESP.restart();
  }
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/update", HTTP_POST, handleUpdateResult, handleUpdate);
  server.begin();
  Serial.println("HTTP server started");
}

void setup() {
  Serial.begin(115200);
  setupAP();
  setupWebServer();
}

void loop() {
  server.handleClient();
}
