var myAPIKey = 'f22e41a3d6bc8bb2b821d3b304952f13';

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  // Construct URL
  var url = 'http://api.openweathermap.org/data/2.5/weather?units=metric&lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=' + myAPIKey;

  console.log('ur is ' + url);

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Temperature in Kelvin requires adjustment
      var temperature = json.main.temp;
      console.log('Temperature is ' + temperature);

      // Conditions
      var conditions = json.weather[0].main;
      console.log('Conditions are ' + conditions);

      var dictionary = {
          'TEMPERATURE': temperature,
          'CONDITIONS': conditions
      };

      Pebble.sendAppMessage(dictionary,
          function(e) {
              console.log('Weather info sent to Pebble successfully !');
          },
          function(e) {
              console.log('Error sending weather info to Pebble :/');
          }
      );
    }
  );
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function(e) {
    console.log('PebbleKit JS ready!');
    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
  }
);
