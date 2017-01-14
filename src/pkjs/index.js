var xhrRequest = function (url, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () { callback(this.responseText); };
  xhr.open('GET', url);
  xhr.send();
};

function locationSuccess(pos) {
  var url = 'https://saurel.me/pebble/' + pos.coords.latitude + '/' + pos.coords.longitude;
  xhrRequest(url, function(responseText) { Pebble.sendAppMessage(JSON.parse(responseText)); });
}

function locationError(err) {
  var url = 'https://saurel.me/pebble/';
  xhrRequest(url, function(responseText) { Pebble.sendAppMessage(JSON.parse(responseText)); });
}

function getLocation() {
  navigator.geolocation.getCurrentPosition( locationSuccess, locationError, {timeout: 15000, maximumAge: 60000});
}

Pebble.addEventListener('ready', function(e) { getLocation(); });
Pebble.addEventListener('appmessage', function(e) { getLocation(); });
