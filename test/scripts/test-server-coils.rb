#!/usr/bin/ruby

require 'rmodbus'

ModBus::RTUClient.new('/dev/ttyS0', 115200) do |cl|
	
	cl.read_retries = 1
	cl.read_retry_timeout = 1
	
	cl.with_slave(16) do |slave|
		
		NTimes = 1000
		
		NTimes.times do |i|
			
			nCoils = rand(0x7B0) + 1
			startCoil = rand(65535 - nCoils)
			endCoil = startCoil + nCoils
			
			v = rand((1 << nCoils) - 1);
			
			ivalue = ("%0#{nCoils}d" % v.to_s(2)).split('').map{ |e| e.to_i }
		
			slave.coils[startCoil...endCoil] = ivalue
			
			rpos = rand(nCoils)
			
			ivalue[rpos] = rand(2)
			
			slave.coils[startCoil + rpos] = ivalue[rpos]
			
			ovalue = slave.coils[startCoil...endCoil];
			
			if(ovalue != ivalue)
				printf "Error: startCoil:0x%04X, nCoils:0x%04X, endCoil:0x%04X\n", startCoil, nCoils, endCoil
				puts "<< #{ivalue.to_s}"
				puts ">> #{ovalue.to_s}"
			end
			
			if(i != 0 && (i % (NTimes / 100)) == 0)
				puts "Progress: #{i/ (NTimes / 100)}%"
			end
		end
	end
end