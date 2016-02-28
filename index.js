var http = require("http");
var url = require("url");
var fs = require("fs");

switch(process.argv[2]){
  case 'dev': //dev environment
    var port = 80;
    break;
  default:
    var port = process.env.PORT;
}

var server = http.createServer(function(request, response){
  var path = url.parse(request.url).pathname;
  switch(path){
    case '/docs.jsonld':
      fs.readFile(__dirname + path, function(error, data){
        if(error){
          response.writeHead(404);
          response.write("/docs not found - 404");
          response.end();
        } else {
          response.writeHead(200, {'Content-Type': 'application/ld+json'});
          response.write(data, "utf8");
          response.end();
        }
      });
      break;
    default:
      response.writeHead(404);
      response.write(path + " --- not found 404 ---");
      response.end();
      break;
  }
});

server.listen(port);
