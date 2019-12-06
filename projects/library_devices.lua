library_devices = {
	["DCS A-10C 1.1.0.9"] = {
		["type"] = "simulation_host",
		["variables"] = {
			["ias"] = {
				["desc"] = "Indicated Airspeed",
				["type"] = "float",
				["default"] = 0,
				["source"] = function() end
			}
		}
		
	}

	["Arduino Uno"] = {
		["type"] = "controller",
		["pins"] = {
			["D0"] = {
				iotype = "digital",
				iomode = "both",
				pullup = true,
				pwm = false,
				spi = nil,
				serial = "rx",
				i2c = nil,
				interrupt = false,
				reserved = false,
				order=0
			},
			["D1"] = { iotype="digital", iomode="both", pullup=true, serial="tx", order=1 },
			["D2"] = { iotype="digital", iomode="both", pullup=true, order=2 },
			["D3"] = { iotype="digital", iomode="both", pullup=true, pwm=true, order=3 },
			["D4"] = { iotype="digital", iomode="both", pullup=true, order=4 },
			["D5"] = { iotype="digital", iomode="both", pullup=true, pwm=true, order=5 },
			["D6"] = { iotype="digital", iomode="both", pullup=true, pwm=true, order=6 },
			["D7"] = { iotype="digital", iomode="both", pullup=true, order=7 },
			["D8"] = { iotype="digital", iomode="both", pullup=true, order=8 },
			["D9"] = { iotype="digital", iomode="both", pullup=true, pwm=true, order=9 },
			["D10"] = { iotype="digital", iomode="both", pullup=true, pwm=true, order=10 },
			["D11"] = { iotype="digital", iomode="both", pullup=true, pwm=true, order=11 },
			["D12"] = { iotype="digital", iomode="both", pullup=true, order=12 },
			["D13"] = { iotype="digital", iomode="both", pullup=true, order=13 },
			["A0"] = { iotype="analog", iomode="both", order=14 },
			["A1"] = { iotype="analog", iomode="both", order=15 },
			["A2"] = { iotype="analog", iomode="both", order=16 },
			["A3"] = { iotype="analog", iomode="both", order=17 },
			["A4"] = { iotype="analog", iomode="both", order=18 },
			["A5"] = { iotype="analog", iomode="both", order=19 }
		}
	}
}