// --------------------------- begin weather
var prefs = {
	"tempFormat": 1,
	"weatherUpdateFreq": 10 * 60,
	"statusbar": 0
};

var weather = {
	"temperature": -461,
	"conditions": 0,
	"lastUpdate": 0,
	"name": "",
	"country": ""
};
var wlocation = {
	"getPosition": 1,
	"typePosition" : "",
	"requestWeather": 0
};
var prevMessages = {};
var coords;

var maxWeatherUpdateFreq = 10 * 60;


function fetchWeather() {
//	if(Math.round(Date.now()/1000) - weather.lastUpdate >= maxWeatherUpdateFreq) {
	if(wlocation["getPosition"]==1){
		window.navigator.geolocation.getCurrentPosition(
			function(pos) { coords = pos.coords; },
			function(err) { console.warn("location error (" + err.code + "): " + err.message); },
			{ "timeout": 45000
//			, "maximumAge": 60000 
			}
		);
		

		var response;
		var req = new XMLHttpRequest();
		console.log("http://api.openweathermap.org/data/2.5/find?" +"lat=" + coords.latitude + "&lon=" + coords.longitude + "&cnt=1");
		req.open('GET', "http://api.openweathermap.org/data/2.5/find?" +
				 "lat=" + coords.latitude + "&lon=" + coords.longitude + "&cnt=1", true);
		req.timeout = 30000;		 
		req.ontimeout = function() {
			console.log("Error request timeout");
			Pebble.sendAppMessage({"setWeather":3});
		}
		req.onload = function(e) {
			if (req.readyState == 4) {
				if(req.status == 200) {
					response = JSON.parse(req.responseText);
					
					if (response && response.list && response.list.length > 0) {
						var weatherResult = response.list[0];
						var now = new Date();
						var sunCalc = SunCalc.getTimes(now, coords.latitude, coords.longitude);
						
						sendWeather(weather = {
							"temperature": weatherResult.main.temp,
							"conditions": weatherResult.weather[0].id,
							"isDay": sunCalc.sunset > now && now > sunCalc.sunrise,
							"lastUpdate": Math.round(now.getTime() / 1000),
							"name": unescape( encodeURIComponent(weatherResult.name)),
							"country": weatherResult.sys.country
						});
					}else{
						if(response && response.list && response.list.length == 0) {
							Pebble.sendAppMessage({"setWeather":2});
						}else{
							Pebble.sendAppMessage({"setWeather":0});
						}
					
					}
				}
				else {
					console.log("Error getting weather info (status " + req.status + ")");
					Pebble.sendAppMessage({"setWeather":4});
				}
			}
		}
		req.send(null);
	}
	if(wlocation["getPosition"]==0 && wlocation["typePosition"].length>0){
		var response;
		var req = new XMLHttpRequest();
		console.log("http://api.openweathermap.org/data/2.5/find?" +"q=" + wlocation["typePosition"] + "&cnt=1");
		req.open('GET', "http://api.openweathermap.org/data/2.5/find?" +"q=" + wlocation["typePosition"] + "&cnt=1", true);
		req.timeout = 30000;		 
		req.ontimeout = function() {
			console.log("Error request timeout");
			sendWeather(weather);
		}
		req.onload = function(e) {
			if (req.readyState == 4) {
				if(req.status == 200) {
					response = JSON.parse(req.responseText);
					
					if (response && response.list && response.list.length > 0) {
						var weatherResult = response.list[0];
						var now = new Date();
						var sunCalc = SunCalc.getTimes(now, weatherResult.coord.lat, weatherResult.coord.lon);
						
						sendWeather(weather = {
							"temperature": weatherResult.main.temp,
							"conditions": weatherResult.weather[0].id,
							"isDay": sunCalc.sunset > now && now > sunCalc.sunrise,
							"lastUpdate": Math.round(now.getTime() / 1000),
							"name": unescape( encodeURIComponent(weatherResult.name)),
							"country": weatherResult.sys.country
						});
					}else{
						if(response && response.list && response.list.length == 0) {
							Pebble.sendAppMessage({"setWeather":2});
						}else{
							Pebble.sendAppMessage({"setWeather":0});
						}
					
					}
				}
				else {
					console.log("Error getting weather info (status " + req.status + ")");
					Pebble.sendAppMessage({"setWeather":4});
				}
			}
		}
		req.send(null);		
	}
//	}
//	else {
//		console.warn("Weather update requested too soon; loading from cache (" + (new Date()).toString() + ")");
//		sendWeather(weather);
//	}
}

function sendWeather(weather) {
	console.warn("send Weather :Location "+weather.name);
	Pebble.sendAppMessage(mergeObjects({
		"temperature": Math.round(weather.temperature * 100),
		"conditions": weather.conditions + (weather.isDay ? 1000 : 0),
		"weathername": weather.name,
		"country": weather.country
	}, {"setWeather": 1}));
}

//function sendPreferences(prefs) {
//	Pebble.sendAppMessage(mergeObjects(prefs, {"setPrefs": 1}));
//}

function mergeObjects(a, b) {
	for(var key in b)
		a[key] = b[key];
	return a;
}

function queryify(obj) {
	var queries = [];
	for(var key in obj) { queries.push(key + "=" + obj[key]) };
	return "?" + queries.join("&");
}

// -------------------------------------- end weather

Pebble.addEventListener("ready", function(e) {
  //console.log("Connect! " + e.ready);
  // begin weather
  prevMessages = {};
  // end weather
});

Pebble.addEventListener("showConfiguration", function(e) {
//  console.log("Configuration window launching...");
  Pebble.openURL("http://pebblewatch.pw/2/2.0.12.html" + '?_=' + new Date().getTime() );
});

Pebble.addEventListener("appmessage", function(e) {
//console.warn("App message:");
wlocation["requestWeather"] = 0;
for(var key in e.payload)
	console.log("-" + key + "-: " + e.payload[key]+ " mt:"+e.payload.message_type);
  //console.log("Received message: type " + e.payload.message_type)
  switch(e.payload.message_type) {
  case 100:
    saveBatteryValue(e);
    break;
  case 103:
    sendTimezoneToWatch();
    break;
  case 21:
	console.log(" 21 fetch Weather	 ");
	fetchWeather();	
  }
//  if(e.payload["getPosition"]) {
//	location.getPosition=e.payload["getPosition"];
//  }
//  if(e.payload["typePosition"]) {
//	location.typePosition=e.payload["typePosition"];
//  }
  
	if(e.payload["requestWeather"] == 1) {
		console.log(" fetch Weather	 ");
//		wlocation["get_weather"] = 1;
		for(var key in wlocation){
			if(e.payload[key] !== "undefined"){ wlocation[key] = e.payload[key];}
			console.log("	 " + key + ": " + e.payload[key]);
		}
//		fetchWeather();
	}  
//	if(e.payload["getPosition"]
  // begin weather
	if(e.payload["setPrefs"] == 1) {
		for(var key in prefs)
			if(e.payload[key] !== "undefined") { prefs[key] = e.payload[key]; }
	}
//	else {
//		console.warn("Received unknown app message:");
//		for(var key in e.payload)
//			console.log("	 " + key + ": " + e.payload[key]);
//	}  
  // end weather
    console.log(" gv " + wlocation["requestWeather"]+ " gp "+wlocation["getPosition"]+ " tp "+ wlocation["typePosition"]);
	if (wlocation["requestWeather"] ==1){
		fetchWeather();
	}
});

function saveBatteryValue(e) {
  console.log("Battery: " + e.payload.send_batt_percent + "%, Charge: " + e.payload.send_batt_charging + ", Plugged: " + e.payload.send_batt_plugged);
 // TODO - actually store these in localStorage along with a date object in some useful manner
}

function sendTimezoneToWatch() {
  var offsetHours = new Date().getTimezoneOffset() / 60;
  // 5 means GMT-5, -5 means GMT+5 ... -12 through +14 are the valid options
  Pebble.sendAppMessage({ message_type: 103, timezone_offset: offsetHours },
    function(e) {
      console.log("Sent TZ message (" + offsetHours + ") with transactionId=" + e.data.transactionId);
    },
    function(e) {
      console.log("Unable to deliver TZ message with transactionId=" + e.data.transactionId
        + " Error is: " + e.error.message);
    }
  );
}

/** 
 *
 * Base64 encode/decode
 * http://www.webtoolkit.info
 *
 **/   
 
var Base64 = {
   _keyStr : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",
   //метод для кодировки в base64 на javascript 
    encode : function (input) {
      var output = "";
      var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
      var i = 0
      input = Base64._utf8_encode(input);
         while (i < input.length) {
       chr1 = input.charCodeAt(i++);
        chr2 = input.charCodeAt(i++);
        chr3 = input.charCodeAt(i++);
       enc1 = chr1 >> 2;
        enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
        enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
        enc4 = chr3 & 63;
       if( isNaN(chr2) ) {
           enc3 = enc4 = 64;
        }else if( isNaN(chr3) ){
          enc4 = 64;
        }
       output = output +
        this._keyStr.charAt(enc1) + this._keyStr.charAt(enc2) +
        this._keyStr.charAt(enc3) + this._keyStr.charAt(enc4);
     }
      return output;
    },
 
   //метод для раскодировки из base64 
    decode : function (input) {
      var output = "";
      var chr1, chr2, chr3;
      var enc1, enc2, enc3, enc4;
      var i = 0;
     input = input.replace(/[^A-Za-z0-9\+\/\=]/g, "");
     while (i < input.length) {
       enc1 = this._keyStr.indexOf(input.charAt(i++));
        enc2 = this._keyStr.indexOf(input.charAt(i++));
        enc3 = this._keyStr.indexOf(input.charAt(i++));
        enc4 = this._keyStr.indexOf(input.charAt(i++));
       chr1 = (enc1 << 2) | (enc2 >> 4);
        chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
        chr3 = ((enc3 & 3) << 6) | enc4;
       output = output + String.fromCharCode(chr1);
       if( enc3 != 64 ){
          output = output + String.fromCharCode(chr2);
        }
        if( enc4 != 64 ) {
          output = output + String.fromCharCode(chr3);
        }
   }
//   output = Base64._utf8_decode(output);
     return output;
   },
   // метод для кодировки в utf8 
    _utf8_encode : function (string) {
      string = string.replace(/\r\n/g,"\n");
      var utftext = "";
      for (var n = 0; n < string.length; n++) {
        var c = string.charCodeAt(n);
       if( c < 128 ){
          utftext += String.fromCharCode(c);
        }else if( (c > 127) && (c < 2048) ){
          utftext += String.fromCharCode((c >> 6) | 192);
          utftext += String.fromCharCode((c & 63) | 128);
        }else {
          utftext += String.fromCharCode((c >> 12) | 224);
          utftext += String.fromCharCode(((c >> 6) & 63) | 128);
          utftext += String.fromCharCode((c & 63) | 128);
        }
     }
      return utftext;
 
    },
 
    //метод для раскодировки из urf8 
    _utf8_decode : function (utftext) {
      var string = "";
      var i = 0;
      var c = c1 = c2 = 0;
      while( i < utftext.length ){
        c = utftext.charCodeAt(i);
       if (c < 128) {
          string += String.fromCharCode(c);
          i++;
        }else if( (c > 191) && (c < 224) ) {
          c2 = utftext.charCodeAt(i+1);
          string += String.fromCharCode(((c & 31) << 6) | (c2 & 63));
          i += 2;
        }else {
          c2 = utftext.charCodeAt(i+1);
          c3 = utftext.charCodeAt(i+2);
          string += String.fromCharCode(((c & 15) << 12) | ((c2 & 63) << 6) | (c3 & 63));
          i += 3;
        }
     }
     return string;
    }
 }


function b64_to_utf8( str ) {
//  console.log("utf_enc "+base64.decode(dtr));
  return decodeURIComponent(escape(Base64.decode( str )));
}

Pebble.addEventListener("webviewclosed", function(e) {
//  console.log("Configuration closed");
//  console.log(Base64.decode(e.response));
//  var options = JSON.parse(decodeURIComponent(e.response));
  var options = JSON.parse(b64_to_utf8(e.response));  
//  utf8enc(options);
//    var options = JSON.parse(b64_to_utf8(e.response));
//  console.log("Options = " + JSON.stringify(options));
  var transactionId = Pebble.sendAppMessage(mergeObjects(options, {"setPrefs": 1}),//{"setPrefs": 1}, options,
    function(e) {
      console.log("Successfully delivered message with transactionId=" + e.data.transactionId);
    },
    function(e) {
      console.log("Unable to deliver message with transactionId=" + e.data.transactionId
        + " Error is: " + e.error.message);
    }
  );
  // begin weather
 // 	if(e && e.response) {
//		var newPrefs = JSON.parse(e.response);
//		for(var key in prefs) {
//			if(newPrefs[key] !== "undefined")
//				prefs[key] = parseInt(newPrefs[key]);
//		}
//		
//		sendPreferences(prefs);
//	}
  // end weather
});




// begin weather
/*
 (c) 2011-2013, Vladimir Agafonkin
 SunCalc is a JavaScript library for calculating sun/mooon position and light phases.
 https://github.com/mourner/suncalc
*/

(function () { "use strict";
 
 // shortcuts for easier to read formulas
 
 var PI		= Math.PI,
 sin	= Math.sin,
 cos	= Math.cos,
 tan	= Math.tan,
 asin = Math.asin,
 atan = Math.atan2,
 acos = Math.acos,
 rad	= PI / 180;
 
 // sun calculations are based on http://aa.quae.nl/en/reken/zonpositie.html formulas
 
 
 // date/time constants and conversions
 
 var dayMs = 1000 * 60 * 60 * 24,
 J1970 = 2440588,
 J2000 = 2451545;
 
 function toJulian(date) {
 return date.valueOf() / dayMs - 0.5 + J1970;
 }
 function fromJulian(j) {
 return new Date((j + 0.5 - J1970) * dayMs);
 }
 function toDays(date) {
 return toJulian(date) - J2000;
 }
 
 
 // general calculations for position
 
 var e = rad * 23.4397; // obliquity of the Earth
 
 function getRightAscension(l, b) {
 return atan(sin(l) * cos(e) - tan(b) * sin(e), cos(l));
 }
 function getDeclination(l, b) {
 return asin(sin(b) * cos(e) + cos(b) * sin(e) * sin(l));
 }
 function getAzimuth(H, phi, dec) {
 return atan(sin(H), cos(H) * sin(phi) - tan(dec) * cos(phi));
 }
 function getAltitude(H, phi, dec) {
 return asin(sin(phi) * sin(dec) + cos(phi) * cos(dec) * cos(H));
 }
 function getSiderealTime(d, lw) {
 return rad * (280.16 + 360.9856235 * d) - lw;
 }
 
 
 // general sun calculations
 
 function getSolarMeanAnomaly(d) {
 return rad * (357.5291 + 0.98560028 * d);
 }
 function getEquationOfCenter(M) {
 return rad * (1.9148 * sin(M) + 0.02 * sin(2 * M) + 0.0003 * sin(3 * M));
 }
 function getEclipticLongitude(M, C) {
 var P = rad * 102.9372; // perihelion of the Earth
 return M + C + P + PI;
 }
 function getSunCoords(d) {
 
 var M = getSolarMeanAnomaly(d),
 C = getEquationOfCenter(M),
 L = getEclipticLongitude(M, C);
 
 return {
 dec: getDeclination(L, 0),
 ra: getRightAscension(L, 0)
 };
 }
 
 
 var SunCalc = {};
 
 
 // calculates sun position for a given date and latitude/longitude
 
 SunCalc.getPosition = function (date, lat, lng) {
 
 var lw	 = rad * -lng,
 phi = rad * lat,
 d	 = toDays(date),
 
 c	= getSunCoords(d),
 H	= getSiderealTime(d, lw) - c.ra;
 
 return {
 azimuth: getAzimuth(H, phi, c.dec),
 altitude: getAltitude(H, phi, c.dec)
 };
 };
 
 
 // sun times configuration (angle, morning name, evening name)
 
 var times = [
				[-0.83, 'sunrise',			 'sunset'			 ],
				[ -0.3, 'sunriseEnd',		 'sunsetStart' ],
				[		-6, 'dawn',					 'dusk'				 ],
				[	 -12, 'nauticalDawn',	 'nauticalDusk'],
				[	 -18, 'nightEnd',			 'night'			 ],
				[		 6, 'goldenHourEnd', 'goldenHour'	 ]
				];
 
 // adds a custom time to the times config
 
 SunCalc.addTime = function (angle, riseName, setName) {
 times.push([angle, riseName, setName]);
 };
 
 
 // calculations for sun times
 
 var J0 = 0.0009;
 
 function getJulianCycle(d, lw) {
 return Math.round(d - J0 - lw / (2 * PI));
 }
 function getApproxTransit(Ht, lw, n) {
 return J0 + (Ht + lw) / (2 * PI) + n;
 }
 function getSolarTransitJ(ds, M, L) {
 return J2000 + ds + 0.0053 * sin(M) - 0.0069 * sin(2 * L);
 }
 function getHourAngle(h, phi, d) {
 return acos((sin(h) - sin(phi) * sin(d)) / (cos(phi) * cos(d)));
 }
 
 
 // calculates sun times for a given date and latitude/longitude
 
 SunCalc.getTimes = function (date, lat, lng) {
 
 var lw	 = rad * -lng,
 phi = rad * lat,
 d	 = toDays(date),
 
 n	= getJulianCycle(d, lw),
 ds = getApproxTransit(0, lw, n),
 
 M = getSolarMeanAnomaly(ds),
 C = getEquationOfCenter(M),
 L = getEclipticLongitude(M, C),
 
 dec = getDeclination(L, 0),
 
 Jnoon = getSolarTransitJ(ds, M, L);
 
 
 // returns set time for the given sun altitude
 function getSetJ(h) {
 var w = getHourAngle(h, phi, dec),
 a = getApproxTransit(w, lw, n);
 
 return getSolarTransitJ(a, M, L);
 }
 
 
 var result = {
 solarNoon: fromJulian(Jnoon),
 nadir: fromJulian(Jnoon - 0.5)
 };
 
 var i, len, time, angle, morningName, eveningName, Jset, Jrise;
 
 for (i = 0, len = times.length; i < len; i += 1) {
 time = times[i];
 
 Jset = getSetJ(time[0] * rad);
 Jrise = Jnoon - (Jset - Jnoon);
 
 result[time[1]] = fromJulian(Jrise);
 result[time[2]] = fromJulian(Jset);
 }
 
 return result;
 };
 
 
 // moon calculations, based on http://aa.quae.nl/en/reken/hemelpositie.html formulas
 
 function getMoonCoords(d) { // geocentric ecliptic coordinates of the moon
 
 var L = rad * (218.316 + 13.176396 * d), // ecliptic longitude
 M = rad * (134.963 + 13.064993 * d), // mean anomaly
 F = rad * (93.272 + 13.229350 * d),	// mean distance
 
 l	= L + rad * 6.289 * sin(M), // longitude
 b	= rad * 5.128 * sin(F),			// latitude
 dt = 385001 - 20905 * cos(M);	// distance to the moon in km
 
 return {
 ra: getRightAscension(l, b),
 dec: getDeclination(l, b),
 dist: dt
 };
 }
 
 SunCalc.getMoonPosition = function (date, lat, lng) {
 
 var lw	 = rad * -lng,
 phi = rad * lat,
 d	 = toDays(date),
 
 c = getMoonCoords(d),
 H = getSiderealTime(d, lw) - c.ra,
 h = getAltitude(H, phi, c.dec);
 
 // altitude correction for refraction
 h = h + rad * 0.017 / tan(h + rad * 10.26 / (h + rad * 5.10));
 
 return {
 azimuth: getAzimuth(H, phi, c.dec),
 altitude: h,
 distance: c.dist
 };
 };
 
 
 // calculations for illuminated fraction of the moon,
 // based on http://idlastro.gsfc.nasa.gov/ftp/pro/astro/mphase.pro formulas
 
 SunCalc.getMoonFraction = function (date) {
 
 var d = toDays(date),
 s = getSunCoords(d),
 m = getMoonCoords(d),
 
 sdist = 149598000, // distance from Earth to Sun in km
 
 phi = acos(sin(s.dec) * sin(m.dec) + cos(s.dec) * cos(m.dec) * cos(s.ra - m.ra)),
 inc = atan(sdist * sin(phi), m.dist - sdist * cos(phi));
 
 return (1 + cos(inc)) / 2;
 };
 
 
 // export as AMD module / Node module / browser variable
 
 if (typeof define === 'function' && define.amd) {
 define(SunCalc);
 } else if (typeof module !== 'undefined') {
 module.exports = SunCalc;
 } else {
 window.SunCalc = SunCalc;
 }
 
 }());
 
