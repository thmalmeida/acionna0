#include "spi_driver.hpp"

SPI_Driver::SPI_Driver(void) {
	init();
}
void SPI_Driver::init(void) {

	spi_bus_config_t bus_cfg;
	bus_cfg.miso_io_num = 24;
	bus_cfg.mosi_io_num = 25;
	bus_cfg.sclk_io_num = 26;
	bus_cfg.quadwp_io_num = -1;
	bus_cfg.quadhd_io_num = -1;
	// bus_cfg.max_transfer_sz = 100; // PARALLEL_LINES * 320 * 2 + 8 // verify it!

	spi_device_interface_config_t dev_cfg;
	dev_cfg.command_bits = 0;
	dev_cfg.address_bits = 0;
	dev_cfg.dummy_bits = 0;
	dev_cfg.clock_speed_hz = 5000000;			// 5 MHz?
	dev_cfg.duty_cycle_pos = 128;				// 50% duty cycle
	dev_cfg.mode = 0;							// SPI mode 0
	dev_cfg.spics_io_num = 27;                  // CS pin number;
	dev_cfg.cs_ena_posttrans = 3;				// keep the CS low 3 cycles after transaction to stop slave from missing the last bit when CS has less propagation delay than CLK;
	dev_cfg.queue_size = 3;
	// dev_cfg.pre_cb =                         // callback specify

	spi_host_device_t spi_host_id_ = spi_host_device_t::SPI1_HOST;

	// SPI Bus initialize
	ESP_ERROR_CHECK(spi_bus_initialize(spi_host_id_, &bus_cfg, SPI_DMA_CH_AUTO));

	// Register a device connected to the bus
	ESP_ERROR_CHECK(spi_bus_add_device(spi_host_id_, &dev_cfg, &spi_handle_));

}
uint8_t SPI_Driver::transfer(uint8_t data) {

	spi_transaction_t t;
	memset(&t, 0, sizeof(t));		// zero out the transaction structure
	t.length = 8;					// 8 bit data size;
	t.tx_buffer = &data;			// The data to transfer
	// t.tx_data = data;
	t.user = (void*)0;				// D/C needs to be set to 0
	// if (keep_cs_active) {
	t.flags = SPI_TRANS_CS_KEEP_ACTIVE;   //Keep CS active after data transfer
	// t.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA;
	// }

	// When using SPI_TRANS_CS_KEEP_ACTIVE, bus must be locked/acquired
	ESP_ERROR_CHECK(spi_device_acquire_bus(spi_handle_, portMAX_DELAY));	// acquire bus
	ESP_ERROR_CHECK(spi_device_polling_transmit(spi_handle_, &t)); 			// Transmit!
	spi_device_release_bus(spi_handle_);									// Release bus

	// return *t.rx_buffer;
	return 0;
}
uint8_t SPI_Driver::transfer(uint8_t *data, size_t length) {
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));	// zero out the transaction structure
	t.length = 8*length;		// 8 bit data size * length;
	t.tx_buffer = data;
	t.user = (void*)0;
	t.flags = SPI_TRANS_CS_KEEP_ACTIVE;

	ESP_ERROR_CHECK(spi_device_acquire_bus(spi_handle_, portMAX_DELAY));	// acquire bus
	ESP_ERROR_CHECK(spi_device_polling_transmit(spi_handle_, &t)); 			// Transmit!
	spi_device_release_bus(spi_handle_);									// Release bus

	return 0;
}
