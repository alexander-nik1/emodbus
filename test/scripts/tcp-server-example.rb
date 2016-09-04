#!/usr/bin/ruby

require 'rmodbus'
# Create TCPServer (port - 8502, slvae ID - 1)
server = ModBus::TCPServer.new(8502,1)
# Initial data
server.coils = [0...65535]
server.discrete_inputs = [0...65535]
server.holding_registers = [0...65535]
server.input_registers = [0...65535]
# Enable debug logging
server.debug = true
# Enable logging
server.audit = true
# Start server
server.start
server.join