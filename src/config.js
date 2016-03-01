Pebble.addEventListener("ready", function(){
  //console.log("PebbleKit JS ready");
});

Pebble.addEventListener("showConfiguration", function(){
  var url = "http://briwestervelt.github.io/rings_config";
  
  //console.log("Opening the config page: " + url);
  
  Pebble.openURL(url);
});

Pebble.addEventListener("webviewclosed", function(e){
  var configData = JSON.parse(decodeURIComponent(e.response));
  //console.log("Config returned: " + JSON.stringify(configData));
  
  if(configData.backgroundColor || configData.foregroundColor){
    Pebble.sendAppMessage(
      {
        backgroundColor: parseInt(configData.backgroundColor, 16),
        foregroundColor: parseInt(configData.foregroundColor, 16),
        batteryLine: configData.batteryLine,
        staticLine: configData.staticLine,
        noLine: configData.noLine,
        showDate: configData.showDate,
        bluetoothVibes: configData.bluetoothVibes
      },
      function(){
        //console.log('data send success');
      },
      function(){
        //console.log('data send failure');
      }
    );
  }
});