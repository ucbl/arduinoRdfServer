var coap = require('coap'), 
  http = require('http'),
  url = require('url'),
  port = 8080;

server = http.createServer(function(httpReq, httpRes){
  var path=url.parse(httpReq.url).pathname

  switch(path){
    default:
      var coapReq = coap.request({
        hostname: '10.0.0.2',
        confirmable: false
      })

      coapReq.on('response', function(coapRes) {
        httpRes.writeHead(200)
        httpRes.write(coapRes.payload)
        console.log(coapRes.payload)
        httpRes.end()
      })

      coapReq.end() 
      break;
  } 
})

server.listen(port, function(){
  console.log("listening on "+port);
})
// the default CoAP port is 5683