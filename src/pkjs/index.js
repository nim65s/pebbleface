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
  var url = 'http://api.openweathermap.org/data/2.5/weather?units=metric&lang=fr&lat=' +
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
      var conditions = json.weather[0].description;
      console.log('Conditions are ' + conditions);

      // Vent https://fr.wikipedia.org/wiki/%C3%89chelle_de_Beaufort
      var windspeed = Math.round(Math.cbrt(Math.pow(json.wind.speed * 3.6, 2) / 9));
      var winddir;
      // http://climate.umn.edu/snow_fence/components/winddirectionanddegreeswithouttable3.htm
      switch (true) {
          case (348.75 < json.wind.deg || json.wind.deg <=  11.25): winddir = ' ↑ '; break;
          case ( 11.25 < json.wind.deg && json.wind.deg <=  33.75): winddir = 'NNE'; break;
          case ( 33.75 < json.wind.deg && json.wind.deg <=  56.25): winddir = ' ↗ '; break;
          case ( 56.25 < json.wind.deg && json.wind.deg <=  78.75): winddir = 'ENE'; break;
          case ( 78.75 < json.wind.deg && json.wind.deg <= 101.25): winddir = ' → '; break;
          case (101.25 < json.wind.deg && json.wind.deg <= 123.75): winddir = 'ESE'; break;
          case (123.75 < json.wind.deg && json.wind.deg <= 146.25): winddir = ' ↘ '; break;
          case (146.25 < json.wind.deg && json.wind.deg <= 168.75): winddir = 'SSE'; break;
          case (168.75 < json.wind.deg && json.wind.deg <= 191.25): winddir = ' ↓ '; break;
          case (191.25 < json.wind.deg && json.wind.deg <= 213.75): winddir = 'SSW'; break;
          case (213.75 < json.wind.deg && json.wind.deg <= 236.25): winddir = ' ↙ '; break;
          case (236.25 < json.wind.deg && json.wind.deg <= 258.75): winddir = 'WSW'; break;
          case (258.75 < json.wind.deg && json.wind.deg <= 281.25): winddir = ' ← '; break;
          case (281.25 < json.wind.deg && json.wind.deg <= 303.75): winddir = 'WNW'; break;
          case (303.75 < json.wind.deg && json.wind.deg <= 326.25): winddir = ' ↖ '; break;
          case (326.25 < json.wind.deg && json.wind.deg <= 348.75): winddir = 'NNW'; break;
      }

      var dictionary = {
          'TEMPERATURE': temperature,
          'CONDITIONS': conditions,
          'WINDDIR': winddir,
          'WIND': windspeed
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
