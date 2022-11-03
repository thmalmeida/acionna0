#ifndef WELL_HPP
#define WELL_HPP

#include <adc.hpp>
#include <gpio.hpp>
#include "pinout.hpp"

enum class states_level {
	low,
	high
};

// Well properties
class well {
public:
	// well variables
//	uint16_t levelSensorLL, levelSensorML, levelSensorHL;
//	uint16_t levelSensorLL_d, levelSensorML_d, levelSensorHL_d;

	// well();

	// member functions declarations
	well() : water_level_{{WATER_LEVEL1},{WATER_LEVEL2},{WATER_LEVEL3},{WATER_LEVEL4}}
	{
//		 pins directions for drive switches;
		for(std::size_t i = 0; i < water_level_count_; i++)
		{
			water_level_[i].pull(GPIO_PULLUP_ONLY);
			water_level_[i].mode(GPIO_MODE_INPUT);
//			water_level_[i].enable_interrupt(GPIO_INTR_ANYEDGE);
//			water_level_[i].register_interrupt(gpio_input_interrput, nullptr);
		}

	}
	// ~well();

	states_level state_L1() const // noexcept // const -> não modifica nada na classe
	{										// noexcept para dizer que não gera exceção
		return state_L1_;
	}
	states_level state_L2() const // noexcept // const -> não modifica nada na classe
	{										// noexcept para dizer que não gera exceção
		return state_L2_;
	}
	states_level state_L3() const // noexcept // const -> não modifica nada na classe
	{										// noexcept para dizer que não gera exceção
		return state_L3_;
	}
	states_level state_L4() const // noexcept // const -> não modifica nada na classe
	{										// noexcept para dizer que não gera exceção
		return state_L4_;
	}
	states_level read_level_sensor_(int l_number)
	{
		if(water_level_[l_number-1].read())
			return states_level::low;
		else
			return states_level::high;
	}
	void update()
	{
		state_L1_ = read_level_sensor_(1);
		state_L2_ = read_level_sensor_(2);
		state_L3_ = read_level_sensor_(3);
		state_L4_ = read_level_sensor_(4);
	}

private:
	GPIO_Basic water_level_[4];
	const std::size_t water_level_count_ = sizeof(water_level_) / sizeof(water_level_[0]);

	states_level state_L1_ = states_level::low;
	states_level state_L2_ = states_level::low;
	states_level state_L3_ = states_level::low;
	states_level state_L4_ = states_level::low;
};
#endif