var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () { callback(this.responseText); };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  var url = 'https://saurel.me/pebble/' + pos.coords.latitude + '/' + pos.coords.longitude;
  xhrRequest(url, 'GET', function(responseText) { Pebble.sendAppMessage(JSON.parse(responseText)); });
}

function locationError(err) { console.log('Error requesting location!'); }

function getWeather() {
  navigator.geolocation.getCurrentPosition( locationSuccess, locationError, {timeout: 15000, maximumAge: 60000});
}

Pebble.addEventListener('ready', function(e) { getWeather(); });
Pebble.addEventListener('appmessage', function(e) { getWeather(); });
