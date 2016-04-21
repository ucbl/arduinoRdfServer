var coap = require('coap'), 
  http = require('http'),
  url = require('url'),
  port = 8080;

server = http.createServer(function(httpReq, httpRes){

  var payload=""
  var chunkNum=0
  httpReq.on('data', function(data){
    payload+=data
  })
  httpReq.on('end',function(){
  
    var coapReq = coap.request({
      hostname: '10.0.0.2',
      confirmable: false,
      pathname: url.parse(httpReq.url).pathname,
      method: httpReq.method
    })

    switch(httpReq.method){
      case 'GET':
        // block2 is implemented
        coapReq.end()
        coapReq.on('response', function(coapRes) {
          httpRes.writeHead(200)
          httpRes.write(coapRes.payload)
          console.log(coapRes.payload)
          httpRes.end()

        })
        break
      case 'POST':
        //block1 is not implemented
        coapReq.setOption('Block1', new Buffer([genBlock1Buff(payload, chunkNum)]))
        var payloadChunk = genBlock1Payload(payload, chunkNum)
        coapReq.write(payloadChunk)
        coapReq.end()
        chunkNum++;
        // workaround by individually sending messages with no ack
        var interval = setInterval(function(){
          if(chunkNum>=(payload.length/16)) 
            clearInterval(interval)
          else{
            if(chunkNum>=(payload.length/16)-1){
              var lastCoapReq = coap.request({
                hostname: '10.0.0.2',
                confirmable: false,
                pathname: url.parse(httpReq.url).pathname,
                method: httpReq.method
              })
              lastCoapReq.setOption('Block1', new Buffer([genBlock1Buff(payload, chunkNum)]))
              var payloadChunk = genBlock1Payload(payload, chunkNum)
              lastCoapReq.write(payloadChunk)
              lastCoapReq.end()
              lastCoapReq.on('response', function(coapRes){
                if(chunkNum>=(payload.length/16)-1){
                  httpRes.writeHead(201)
                  httpRes.write(coapRes.payload)
                  httpRes.end()
                }
              })
            } else {
              var coapReq = coap.request({
                hostname: '10.0.0.2',
                confirmable: false,
                pathname: url.parse(httpReq.url).pathname,
                method: httpReq.method
              })
              coapReq.setOption('Block1', new Buffer([genBlock1Buff(payload, chunkNum)]))
              var payloadChunk = genBlock1Payload(payload, chunkNum)
              coapReq.write(payloadChunk)
              coapReq.end()

            }
            chunkNum++
          }
        },50)
 
        break
      default:
        console.log("undef method")
        break
    }
  
  })
})


server.listen(port, function(){
  console.log("listening on "+port);
})


function genBlock1Buff(payload, chunkNum){
  // only for 16B PL size
  var buff = new Buffer([0x0])
  if(payload.length>=16)
    if(chunkNum<(payload.length/16)-1){
      buff |= 1 << 3 //more
      buff |= (chunkNum&0xf) << 4
    }
    else{
      buff |= 0 << 3 //more
      buff |= (chunkNum&0xf) << 4
    }
  return buff
}

function genBlock1Payload(payload, chunkNum){
  if(payload.length<16)
    return payload
  else {
    if(chunkNum<(payload.length/16)-1){
      return payload.slice(chunkNum*16, (chunkNum+1)*16)
    }
    else
      return payload.slice(chunkNum*16,payload.length-1)
  }
}