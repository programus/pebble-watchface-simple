// listen kit ready
Pebble.addEventListener('ready', function (e) {
  console.log("PabbleKit JS ready!");
  
  getWeather();
});

// listen app message
Pebble.addEventListener('appmessage', function (e) {
  console.log("App message received.");
  
  getWeather();
});

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  
  xhr.open(type, url);
  xhr.send();
};

// get weather
function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationFailed,
    {
      timeout: 15000,
      maximumAge: 60000
    }
  );
}

function locationSuccess(pos) {
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' +
    pos.coords.latitude + '&lon=' + pos.coords.longitude;
  
  xhrRequest(url, 'GET', function (responseText) {
    var json = JSON.parse(responseText);
    console.log(json);
    
    var temperature = Math.round(json.main.temp - 273.15);
    console.log("Temperatur: " + temperature);
    
    var conditions = json.weather[0].main;
    console.log("Conditions: " + conditions);
    
    var dictionary = {
      'KEY_TEMPERATURE': temperature,
      'KEY_CONDITIONS': conditions
    };
    
    Pebble.sendAppMessage(dictionary, function (e) {
      console.log('Weather info sent to Pebble successfully!');
    }, function (e) {
      console.log('Weather info sent to Pebble failed.');
    });
  });
}

function locationFailed(err) {
  console.log("Error requesting location.");
}