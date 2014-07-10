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
var weather4 = {
	"temperature": -461,
	"conditions": 0,
	"isDay": true,
	"lastUpdate": 0,
	"name": "",
	"country": "",
/*	"t":[{"temperature": -461,"conditions": 0,"isDay": true,"time":0},
		 {"temperature": -461,"conditions": 0,"isDay": true,"time":0},
		 {"temperature": -461,"conditions": 0,"isDay": true,"time":0},
		 {"temperature": -461,"conditions": 0,"isDay": true,"time":0}
		],*/
	"temperature0": -461,
	"conditions0": 0,
	"isDay0": true,
	"time0" : 0,

	"temperature6": -461,
	"conditions6": 0,
	"isDay6": true,
	"time6" : 0,
	
	"temperature12": -461,
	"conditions12": 0,
	"isDay12": true,
	"time12":0,
	
	"temperature18": -461,
	"conditions18": 0,
	"isDay18": true,
	"time18":0
}

var wlocation = {
	"getPosition": 1,
	"typePosition" : "",
	"requestWeather": 0,
	"countWeather":  1,
	"WeatherPeriod": 6
};
var prevMessages = {};
var coords;

var maxWeatherUpdateFreq = 10 * 60;

Date.prototype.myFormat = function(){
  var dd=this.getDate();
  if(dd<10)dd='0'+dd;
  var mm=this.getMonth()+1;
  if(mm<10)mm='0'+mm;
  var yyyy=this.getFullYear();
  var hh=this.getHours();
  if(hh<10)hh='0'+hh;
  var mn=this.getMinutes();
  if (mn<10) mn='0'+mn;
  var ss=this.getSeconds();
  if(ss<10) ss='0'+ss;
  return String(yyyy+"-"+mm+"-"+dd+" "+hh+":"+mn+":"+ss);
}

function fetchWeather() {
	var request="http://api.openweathermap.org/data/2.5/find?" +"q=" + wlocation["typePosition"] + "&cnt=5";
	if(wlocation["countWeather"]==4){
		request="http://api.openweathermap.org/data/2.5/forecast?" +"q=" + wlocation["typePosition"] + "&cnt=5";
	}
	if(wlocation["getPosition"]==1){
		window.navigator.geolocation.getCurrentPosition(
			function(pos) { coords = pos.coords; },
			function(err) { console.warn("location error (" + err.code + "): " + err.message); },
			{ "timeout": 45000
//			, "maximumAge": 60000 
			}
		);
		request="http://api.openweathermap.org/data/2.5/find?" + "lat=" + coords.latitude + "&lon=" + coords.longitude + "&cnt=1";
		if(wlocation["countWeather"]==4){
			request="http://api.openweathermap.org/data/2.5/forecast?" + "lat=" + coords.latitude + "&lon=" + coords.longitude + "&cnt=1"
		}
	}	
	var response;
	var req = new XMLHttpRequest();
//	console.log(request);
	req.open('GET', request, true);
	req.timeout = 30000;		 
	req.ontimeout = function() {
//		console.log("Error request timeout");
		Pebble.sendAppMessage({"setWeather":3});
	}
	req.onload = function(e) {
		if (req.readyState == 4) {
			if(req.status == 200) {
				response = JSON.parse(req.responseText);
				if (response && response.list && response.list.length > 0) {
					if(wlocation["countWeather"]==1){
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
					}	
					if(wlocation["countWeather"]==4){	
						var weatherResult = response;
						var now = new Date();
//						console.log("lat="+weatherResult.city.coord.lat+"  lon="+weatherResult.city.coord.lon);
						var hours=wlocation["WeatherPeriod"];//6;
						var bhours= now.getHours();
						now.setMinutes(0,0);
						bhours=bhours-(bhours%3);
						var wdat= new Array(5);
						var whours=new Array(5);
						var wtime=new Array(5);
						now.setHours(bhours);
						for (var i=0; i<5; i++){
							wdat[i]=now.myFormat(now);
							var sunCalc = SunCalc.getTimes(now, weatherResult.city.coord.lat, weatherResult.city.coord.lon);
							whours[i]= sunCalc.sunset > now && now > sunCalc.sunrise;
							wtime[i]=now.getTime();
							now.setHours(now.getHours()+hours);
//							console.log('i='+i+'  wdat[i]='+wdat[i]+' wtime[i]='+wtime[i]+ ' hours='+hours);
						}
				
						var cnt=weatherResult.list.length;
						var wtemp=new Array(5);
						var wcond=new Array(5);
						for(var i=0; i<cnt; i++){
							for(var k=0;k<5;k++){
								if (weatherResult.list[i].dt_txt==wdat[k]){
									wtemp[k]=weatherResult.list[i].main.temp;
									wcond[k]=weatherResult.list[i].weather[0].id;
//									console.log('i='+i+' k='+k+'  wtemp[k]='+wtemp[k]+' wcond[k]='+wcond[k]);
								}									
							}
						}
						sendWeather4(weather4 = {
							"lastUpdate": Math.round(now.getTime() / 1000),
							"name": unescape( encodeURIComponent(weatherResult.city.name)),
							"country": unescape( encodeURIComponent(weatherResult.city.country)),
							"temperature": wtemp[0],
							"conditions": wcond[0],
							"isDay": whours[0],
							"temperature0": wtemp[1],
							"conditions0": wcond[1],
							"isDay0": whours[1],
							"time0": wtime[1],
							"temperature6": wtemp[2],
							"conditions6": wcond[2],
							"isDay6": whours[2],
							"time6": wtime[2],
							"temperature12": wtemp[3],
							"conditions12": wcond[3],
							"isDay12": whours[3],
							"time12": wtime[3],
							"temperature18": wtemp[4],
							"conditions18": wcond[4],
							"isDay18": whours[4],
							"time18": wtime[4]
						});						
					}
				}else{
					if(response && response.list && response.list.length == 0) {
						Pebble.sendAppMessage({"setWeather":2});
					}else{
						Pebble.sendAppMessage({"setWeather":0});
					}
				}
			}
			else {
//				console.log("Error getting weather info (status " + req.status + ")");
				Pebble.sendAppMessage({"setWeather":5});
			}
		}
	}
	req.send(null);
}

function sendWeather(weather) {
//	console.warn("send Weather :Location "+weather.name);
//	console.log("Weather = " + JSON.stringify(weather));
	Pebble.sendAppMessage(mergeObjects({
		"temperature": Math.round(weather.temperature * 100),
		"conditions": weather.conditions + (weather.isDay ? 1000 : 0),
		"weathername": weather.name,
		"country": weather.country
	}, {"setWeather": 1}));
}
function sendWeather4(weather4) {
	var offsetHours = new Date().getTimezoneOffset() ;
	offsetHours=offsetHours*(-1);
//	console.warn("send Weather4 :Location "+weather4.name);
//	console.log("Weather4 = " + JSON.stringify(weather4));
	Pebble.sendAppMessage(mergeObjects({
		"weathername": weather4.name,
		"country": weather4.country,
		"temperature": Math.round(weather4.temperature * 100),
		"conditions": weather4.conditions + (weather4.isDay ? 1000 : 0),
		"temperature0": Math.round(weather4.temperature0 * 100),
		"conditions0": weather4.conditions0 + (weather4.isDay0 ? 1000 : 0),
		"time0": weather4.time0/1000 + offsetHours*60,
		"temperature6": Math.round(weather4.temperature6 * 100),
		"conditions6": weather4.conditions6 + (weather4.isDay6 ? 1000 : 0),
		"time6": weather4.time6/1000 + offsetHours*60,
		"temperature12": Math.round(weather4.temperature12 * 100),
		"conditions12": weather4.conditions12 + (weather4.isDay12 ? 1000 : 0),
		"time12": weather4.time12/1000 + offsetHours*60,
		"temperature18": Math.round(weather4.temperature18 * 100),
		"conditions18": weather4.conditions18 + (weather4.isDay18 ? 1000 : 0),
		"time18": weather4.time18/1000 + offsetHours*60
	}, {"setWeather": 4}));
}

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

Pebble.addEventListener("ready", function(e) {
	prevMessages = {};
});

Pebble.addEventListener("showConfiguration", function(e) {
	Pebble.openURL("http://pebblewatch.pw/tcw/setup/index.php" + '?_=' + new Date().getTime()+"&version=24" );
});

Pebble.addEventListener("appmessage", function(e) {
	wlocation["requestWeather"] = 0;
	//for(var key in e.payload)
	//	console.log("-" + key + "-: " + e.payload[key]+ " mt:"+e.payload.message_type);
	//console.log("Received message: type " + e.payload.message_type)
	switch(e.payload.message_type) {
	case 100:
		saveBatteryValue(e);
		break;
	case 103:
		sendTimezoneToWatch();
		break;
	case 21:
//		console.log(" 21 fetch Weather	 ");
		fetchWeather();	
	}

	if(e.payload["requestWeather"] == 1) {
//		wlocation["get_weather"] = 1;
		for(var key in wlocation){
			if(e.payload[key] !== "undefined"){ wlocation[key] = e.payload[key];}
//			console.log("	 " + key + ": " + e.payload[key]);
		}
//		fetchWeather();
	}  
	if(e.payload["setPrefs"] == 1) {
		for(var key in prefs)
			if(e.payload[key] !== "undefined") { prefs[key] = e.payload[key]; }
	}
//    console.log(" gv " + wlocation["requestWeather"]+ " gp "+wlocation["getPosition"]+ " tp "+ wlocation["typePosition"]+ " count "+wlocation["countWeather"]);
	if (wlocation["requestWeather"] ==1){
//		console.log(" fetch Weather	 ");
		fetchWeather();
	}
});

function saveBatteryValue(e) {
//  console.log("Battery: " + e.payload.send_batt_percent + "%, Charge: " + e.payload.send_batt_charging + ", Plugged: " + e.payload.send_batt_plugged);
 // TODO - actually store these in localStorage along with a date object in some useful manner
}

function sendTimezoneToWatch() {
	var offsetHours = new Date().getTimezoneOffset() / 60;
	// 5 means GMT-5, -5 means GMT+5 ... -12 through +14 are the valid options
	Pebble.sendAppMessage({ message_type: 103, timezone_offset: offsetHours },
		function(e) {
//			console.log("Sent TZ message (" + offsetHours + ") with transactionId=" + e.data.transactionId);
		},
		function(e) {
//			console.log("Unable to deliver TZ message with transactionId=" + e.data.transactionId + " Error is: " + e.error.message);
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
	return output;
	}
}


function b64_to_utf8( str ) {
	var	str1=str.replace(/ +/g, '+');
	return decodeURIComponent(escape(Base64.decode( str1 )));
}

Pebble.addEventListener("webviewclosed", function(e) {
	var options = JSON.parse(b64_to_utf8(e.response));  
//	console.log(JSON.stringify(options));
	var transactionId = Pebble.sendAppMessage(mergeObjects(options, {"setPrefs": 1}),//{"setPrefs": 1}, options,
		function(e) {
//			console.log("Successfully delivered message with transactionId=" + e.data.transactionId);
		},
		function(e) {
//			console.log("Unable to deliver message with transactionId=" + e.data.transactionId  + " Error is: " + e.error.message);
		}
	);

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
 
