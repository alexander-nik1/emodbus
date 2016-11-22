#!/usr/bin/ruby

require 'rmodbus'

# Create TCPServer (port - 8502, slvae ID - 1)
server = ModBus::TCPServer.new(8502, 255)

# Initial data
# server.coils = Array.new(0x10000) { |i| i=0 }
# server.discrete_inputs = [0...65536]
server.holding_registers = [0...65536]
# server.input_registers = [0...65536]

# Enable debug logging
server.debug = true

# Enable logging
server.audit = true

# Thread.new do
# 	loop do
# 		server.holding_registers[0] += 1
# 		sleep 1
# 	end
# end

# Start server
server.start
server.join