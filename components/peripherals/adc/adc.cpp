#include "adc.hpp"
//ADC_Basic::ADC_Basic(adc_channel_t channel)
//{
//
//}
//void ADC_Basic::check_efuse_(void)
//{
//}
//uint32_t ADC_Basic::voltage_converter(uint32_t d12)
//{
	/*
  4.5 V___	    922___	1.2 MPa___	 12 Bar___	 120 m.c.a.___		  4096 ___       3.16 V___
		|			|			|			|				|				|				|
		|			|			|			|				|				|				|
		|			|			|			|				|				|				|
	out_|		Pd__|		  __|			|			Pa__|		   d12__|			 v__|
		|			|			|			|				|				|				|
		|			|			|			|				|				|				|
		|			|			|			|				|				|				|
		|	_	   _|_		   _|_		   _|_			   _|_			   _|_			   _|_
	0.5 V			103			0 MPa		0 Bar		0 m.c.a.		0				0 V

	we are trying to convert

  1.1 V___	   2048___	  5.0 V___	  4.5 V___	 120 m.c.a.___		  4096 ___       3.16 V___
		|			|			|			|				|				|				|
		|			|			|			|				|				|				|
		|			|			|			|				|				|				|
	out_|		Pd__|		Vi__|			|			Pa__|		   d12__|			 v__|
		|			|			|			|				|				|				|
		|			|			|			|				|				|				|
		|			|			|	 		|				|				|				|
		|	_	  0_|_		0 V_|_	 0.5 V _|_			   _|_			   _|_			   _|_
	0.0 V											0 m.c.a.		0				0 V

	(out-0.5)/(4.5-0.5) = 1024

	(out-0.0)/(5-0) = (x-0)/(1024-0)

	(Pd - 103)/(922-103) = (Pa - 0)/(120 - 0)
	Pa = 120.0*Pd/(1024.0);

	(xs - 0) = temp - (0)
	(255 - 0)  +50 - (0)

	Direct Conversion
	xs = 255*(temp+0)/51
	tempNow_XS = (uint8_t) 255.0*(tempNow+0.0)/51.0;

	Inverse Conversion
	temp = (TempMax*xs/255) - TempMin
	tempNow = (uint8_t) ((sTempMax*tempNow_XS)/255.0 - sTempMin);

    (d12-0)/(4096-0) = (v-0)/(3.16-0)
    v = d12/4096*3162

    */

//	0,703089

//	uint32_t voltage = 0;

//	voltage = d12*DEFAULT_VREF/2047;
//	printf("Raw0: %d\n", d12);
//	voltage = 1000.0*5.0*(d12/2048.0);
//	printf("Raw0: %d\tVoltage0:%d\n", d12, voltage);
//	return voltage;
//}
//uint32_t ADC_Basic::read(void)
//{
//	uint32_t adc_reading = 0;
//
//	for (int i = 0; i < NO_OF_SAMPLES; i++)		//Multisampling
//	{
//		if (unit == ADC_UNIT_1)					// Sample ADC1
//		{
//			adc_reading += adc1_get_raw((adc1_channel_t)channel_);
//		}
//		else
//		{
//			int raw;
//			adc2_get_raw((adc2_channel_t)channel_, width, &raw);
//			adc_reading += raw;
//		}
//	}
//	adc_reading /= NO_OF_SAMPLES;
//
//	return adc_reading;
//}
//void ADC_Basic::run_example(void)
//{
//	uint32_t adc_readed = read();
//	// Convert adc_reading to voltage in mV
//	uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_readed, adc_chars);
//	uint32_t voltage2 = voltage_converter(adc_readed);
//
//	printf("adc_read: %d\tVoltage: %dmV, Voltage2 %dmV\n", adc_readed, voltage, voltage2);
//}

//static void print_char_val_type(esp_adc_cal_value_t val_type)
//{
//    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
//        printf("Characterized using Two Point Value\n");
//    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
//        printf("Characterized using eFuse Vref\n");
//    } else {
//        printf("Characterized using Default Vref!!!\n");
//    }
//}

//ADC_Basic::ADC_Basic

//unsigned int GPIO_Basic::driver_instaled = 0;
//
//GPIO_Basic::GPIO_Basic(gpio_num_t num, gpio_mode_t mode) : level(0){
//	this->num = num;
//	gpio_set_direction(num,mode);
//}
//
//void GPIO_Basic::mode(gpio_mode_t mode){
//	gpio_set_direction(num,mode);
//}
//
//void GPIO_Basic::write(int level){
//	gpio_set_level(num, level);
//}
//
//void GPIO_Basic::toggle(){
//	level = !level;
//	write(level);
//}
//
//int GPIO_Basic::read(){
//	level = gpio_get_level(num);
//	return level;
//}
//
//void GPIO_Basic::reset() noexcept
//{
//	gpio_reset_pin(num);
//}
//
//void GPIO_Basic::pull(gpio_pull_mode_t mode){
//	gpio_set_pull_mode(num, mode);
//}
//
//void GPIO_Basic::strength(gpio_drive_cap_t cap){
//	gpio_set_drive_capability(num, cap);
//}
//
//void GPIO_Basic::hold(bool hold){
//	if(hold){
//		gpio_hold_en(num);
//	} else {
//		gpio_hold_dis(num);
//	}
//}
//
//void GPIO_Basic::deep_sleep_hold(bool hold){
//	if(hold){
//		gpio_deep_sleep_hold_en();
//	} else {
//		gpio_deep_sleep_hold_dis();
//	}
//}
//
//void GPIO_Basic::register_interrupt(gpio_isr_t handler, void* isr_args){
//	if(driver_instaled == 0){
//		gpio_install_isr_service(GPIO_INTERRUPT_FLAGS);
//		driver_instaled++;
//	}
//
//	this->isr_args = isr_args;
//	gpio_isr_handler_add(num, handler,this);
//}
//
//void GPIO_Basic::unregister_interrupt(){
//	if(driver_instaled == 0){
//		return;
//	}
//
//	gpio_isr_handler_remove(num);
//	disable_interrupt();
//
//	if(--driver_instaled){
//		gpio_uninstall_isr_service();
//	}
//}
//
//void GPIO_Basic::enable_interrupt(gpio_int_type_t type){
//	gpio_set_intr_type(num, type);
//	gpio_intr_enable(num);
//}
//
//void GPIO_Basic::disable_interrupt(){
//	gpio_intr_disable(num);
//}
//
//void* GPIO_Basic::get_isr_args(){
//	return isr_args;
//}
//
//GPIO_Basic::~GPIO_Basic(){
//	unregister_interrupt();
//}
