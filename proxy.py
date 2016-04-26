from twisted.web import server, resource
from twisted.internet import reactor
from twisted.python import log
import sys
import txthings.coap as coap
import txthings.resource as res

class MySite(server.Site):
    def getResourceFor(self, request):
        request.setHeader("Content-Type", "application/ld+json")
        request.setHeader("Connection","close")
        return server.Site.getResourceFor(self, request)

class HttpServer(resource.Resource):

	isLeaf = True

	def __init__ (self, arduinoAgent):
		self.arduinoAgent = arduinoAgent

	def render_GET(self, request):
		#send the requests to the arduino
		self.arduinoAgent.requestResource(request, self)
		#store the request in the scope to finish the response
		self.request = request
		return server.NOT_DONE_YET

	def render_POST(self, request):
		self.arduinoAgent.postResource(request, self)
		self.request = request
		return server.NOT_DONE_YET

	#callback function when a CoAP message is received
	def _delayedResponse(self, result):
		self.request.write(result.payload)
		self.request.finish()

class CoapAgent():
	def __init__(self, protocol):
		self.protocol = protocol

	def requestResource(self, httpRequest, httpServer):
		request = coap.Message(code=coap.GET)
		request.opt.uri_path = (httpRequest.path[1:],)
		request.remote = ("10.0.0.2", coap.COAP_PORT)
		d = coapProtocol.request(request)
		d.addCallback(httpServer._delayedResponse)
		d.addErrback(self.noResponse)

	def postResource(self, httpRequest, httpServer):
		request = coap.Message(code=coap.POST, payload=httpRequest.content.getvalue())
		request.opt.uri_path = (httpRequest.path[1:],)
		request.remote = ('10.0.0.2', coap.COAP_PORT)
		d = coapProtocol.request(request)
		d.addCallback(httpServer._delayedResponse)
		d.addErrback(self.noResponse)

	def noResponse(self, failure):
		print 'Failed to fetch resource:'
		print failure
		reactor.stop()

log.startLogging(sys.stdout)

endpoint = res.Endpoint(None)
coapProtocol = coap.Coap(endpoint)
arduinoAgent = CoapAgent(coapProtocol)
reactor.listenUDP(61616, coapProtocol)

entryPoint = MySite(HttpServer(arduinoAgent))
reactor.listenTCP(8080, entryPoint)
reactor.run()

