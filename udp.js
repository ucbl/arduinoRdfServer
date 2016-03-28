var coap        = require('coap')
  , server      = coap.createServer()

server.on('request', function(req, res) {
  res.end('Hello ' + req.url.split('/')[0] + '\n')
})

// the default CoAP port is 5683
server.listen(function() {
  var req = coap.request({
  	hostname: '10.0.0.2',
  	confirmable: false,
  	options: null
  })

  req.on('response', function(res) {
    res.pipe(process.stdout)
    res.on('end', function() {
      process.exit(0)
    })
  })

  req.end()
})