//at watchface start
Pebble.addEventListener('ready',
	function(e) {
		
		//send settings to watch
		sendConfig();
	}
);

//at opening configuration
Pebble.addEventListener('showConfiguration', 
	function(e) {
		
		//send current config to url
		var config = {
                  seconds	: localStorage.getItem("seconds"),
                  weekday	: localStorage.getItem("weekday"),
                  date		: localStorage.getItem("date"),
                  month		: localStorage.getItem("month")
                };
		
		Pebble.openURL('http://googledrive.com/host/0B6y59waHe-KXaDZHcG5mblZRcTQ?' + JSON.stringify(config));
	}
);

//at finished configuration
Pebble.addEventListener('webviewclosed',
	function(e) {

		//get the variables
		var config = JSON.parse(e.response);
		
		//save variables locally
		localStorage.setItem("seconds", config.seconds);
		localStorage.setItem("weekday", config.weekday);
		localStorage.setItem("date",	config.date);
		localStorage.setItem("month",	config.month);
		
		console.log('saved variables');
		
		sendConfig();
	}
);


function sendConfig() {
	
	console.log('sendConfig()');
	
	//get from local storage
	var seconds = parseInt(localStorage.getItem("seconds"));
	var weekday = parseInt(localStorage.getItem("weekday"));
	var date	= parseInt(localStorage.getItem("date"));
	var month	= parseInt(localStorage.getItem("month"));
	
	//if invalid, use defaults
	if(isNaN(seconds))
		seconds = 1;
	if(isNaN(weekday))
		weekday = 1;
	if(isNaN(date))
		date = 0;
	if(isNaN(month))
		month = 0;
	
	
	
	//send to pebble
	Pebble.sendAppMessage({
		0	: seconds,
		1	: weekday,
		2	: date,
		3	: month
  });   
	
	console.log('done');
	
}