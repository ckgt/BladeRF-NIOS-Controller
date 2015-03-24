/*
 * blade_hardware.h
 *
 *  Created on: Mar 21, 2015
 *      Author: Cameron Karlsson
 */

#ifndef BLADE_HARDWARE_H_
#define BLADE_HARDWARE_H_


#include "simple_types.h"

// skip Altera includes if null hardware...
#ifndef BLADE_NULL_HARDWARE
 #include "system.h"
 #include "altera_avalon_spi.h"
 #include "altera_avalon_uart_regs.h"
 #include "altera_avalon_jtag_uart_regs.h"
 #include "altera_avalon_pio_regs.h"
#endif
 

//         _______           _      ___________ 
//        | ___ \ |         | |    | ___ \  ___|
//        | |_/ / | __ _  __| | ___| |_/ / |_   
//        | ___ \ |/ _` |/ _` |/ _ \    /|  _|  
//        | |_/ / | (_| | (_| |  __/ |\ \| |    
//        \____/|_|\__,_|\__,_|\___\_| \_\_|    
//   
// The goal of this file is to abstract some of the Altera
// macros/functions. A generic blade structure is created
// that contains the functions to access the underlying hardware.
// 
//                                         

// DEBUGGING NOTE:
// If not running on the nios, like on a pc or something...
// define BLADE_NULL_HARDWARE before this file.
//
// This will replace all hardware functions with print statements.
//





//when version id is moved to a qsys port these will be removed
#define FPGA_VERSION_ID         0x7777
#define FPGA_VERSION_MAJOR      0
#define FPGA_VERSION_MINOR      1
#define FPGA_VERSION_PATCH      3
#define FPGA_VERSION            (FPGA_VERSION_MAJOR | (FPGA_VERSION_MINOR << 8) | (FPGA_VERSION_PATCH << 16))

#define TIME_TAMER              TIME_TAMER_0_BASE
// Register offsets from the base
#define I2C                 BLADERF_OC_I2C_MASTER_0_BASE
#define OC_I2C_PRESCALER    0
#define OC_I2C_CTRL         2
#define OC_I2C_DATA         3
#define OC_I2C_CMD_STATUS   4

#define SI5338_I2C          (0xE0)
#define OC_I2C_ENABLE       (1<<7)
#define OC_I2C_STA          (1<<7)
#define OC_I2C_STO          (1<<6)
#define OC_I2C_WR           (1<<4)
#define OC_I2C_RD           (1<<5)
#define OC_I2C_TIP          (1<<1)
#define OC_I2C_RXACK        (1<<7)
#define OC_I2C_NACK         (1<<3)

#define DEFAULT_GAIN_CORRECTION 0x1000
#define GAIN_OFFSET 0
#define DEFAULT_PHASE_CORRECTION 0x0000
#define PHASE_OFFSET 16
#define DEFAULT_CORRECTION ( (DEFAULT_PHASE_CORRECTION << PHASE_OFFSET)|  (DEFAULT_GAIN_CORRECTION << GAIN_OFFSET))


#define LMS_READ            0
#define LMS_WRITE           (1<<7)
#define LMS_VERBOSE         1
#define kHz(x)              (x*1000)
#define MHz(x)              (x*1000000)
#define GHz(x)              (x*1000000000)




//================================================================
//================================================================
//=================== THE HARDWARE ACCESS FUNCTIONS ==============
//================================================================
//================================================================

void blade_initialize_hardware( void ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return;
#else
	// Set the prescaler for 400kHz with an 80MHz clock (prescaer = clock / (5*desired) - 1)
	IOWR_16DIRECT(I2C, OC_I2C_PRESCALER, 39 ) ;
	IOWR_8DIRECT(I2C, OC_I2C_CTRL, OC_I2C_ENABLE ) ;

	// Set the FX3 UART connection divisor to 14 to get 4000000bps UART (baud rate = clock/(divisor + 1))
	IOWR_ALTERA_AVALON_UART_DIVISOR(UART_0_BASE, 19) ;

	// Set the IQ Correction parameters to 0
	IOWR_ALTERA_AVALON_PIO_DATA(IQ_CORR_RX_PHASE_GAIN_BASE, DEFAULT_CORRECTION);
	IOWR_ALTERA_AVALON_PIO_DATA(IQ_CORR_TX_PHASE_GAIN_BASE, DEFAULT_CORRECTION);
	return;
#endif
}



void si5338_complete_transfer( uint8_t check_rxack ) {
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return;
#else
    if( (IORD_8DIRECT(I2C, OC_I2C_CMD_STATUS)&OC_I2C_TIP) == 0 ) {
        while( (IORD_8DIRECT(I2C, OC_I2C_CMD_STATUS)&OC_I2C_TIP) == 0 ) { } ;
    }
    while( IORD_8DIRECT(I2C, OC_I2C_CMD_STATUS)&OC_I2C_TIP )
    { } ;
    while( check_rxack && (IORD_8DIRECT(I2C, OC_I2C_CMD_STATUS)&OC_I2C_RXACK) )
    { } ;
    return;
#endif
}

void si5338_read( uint8_t addr, uint8_t *data ) {
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return;
#else
    // Set the address to the Si5338
    IOWR_8DIRECT(I2C, OC_I2C_DATA, SI5338_I2C ) ;
    IOWR_8DIRECT(I2C, OC_I2C_CMD_STATUS, OC_I2C_STA | OC_I2C_WR ) ;
    si5338_complete_transfer( 1 ) ;

    IOWR_8DIRECT(I2C, OC_I2C_DATA, addr ) ;
    IOWR_8DIRECT(I2C, OC_I2C_CMD_STATUS, OC_I2C_WR | OC_I2C_STO ) ;
    si5338_complete_transfer( 1 ) ;

    // Next transfer is a read operation, so '1' in the read/write bit
    IOWR_8DIRECT(I2C, OC_I2C_DATA, SI5338_I2C | 1 ) ;
    IOWR_8DIRECT(I2C, OC_I2C_CMD_STATUS, OC_I2C_STA | OC_I2C_WR ) ;
    si5338_complete_transfer( 1 ) ;

    IOWR_8DIRECT(I2C, OC_I2C_CMD_STATUS, OC_I2C_RD | OC_I2C_NACK | OC_I2C_STO ) ;
    si5338_complete_transfer( 0 ) ;

    *data = IORD_8DIRECT(I2C, OC_I2C_DATA) ;
    return;
#endif
}

void si5338_write( uint8_t addr, uint8_t data ) {
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return;
#else
    // Set the address to the Si5338
    IOWR_8DIRECT(I2C, OC_I2C_DATA, SI5338_I2C) ;
    IOWR_8DIRECT(I2C, OC_I2C_CMD_STATUS, OC_I2C_STA | OC_I2C_WR ) ;
    si5338_complete_transfer( 1 ) ;

    IOWR_8DIRECT(I2C, OC_I2C_DATA, addr) ;
    IOWR_8DIRECT(I2C, OC_I2C_CMD_STATUS, OC_I2C_CMD_STATUS | OC_I2C_WR ) ;
    si5338_complete_transfer( 1 ) ;

    IOWR_8DIRECT(I2C, OC_I2C_DATA, data ) ;
    IOWR_8DIRECT(I2C, OC_I2C_CMD_STATUS, OC_I2C_WR | OC_I2C_STO ) ;
    si5338_complete_transfer( 0 ) ;

    return;
#endif
}

// Trim DAC write
void dac_write( uint16_t val ) {
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return;
#else
//    alt_printf( "DAC Writing: %x\n", val ) ;
    uint8_t data[3] ;
    data[0] = 0x28, data[1] = 0, data[2] = 0 ;
    alt_avalon_spi_command( SPI_1_BASE, 0, 3, data, 0, 0, 0 ) ;
    data[0] = 0x08, data[1] = (val>>8)&0xff, data[2] = val&0xff  ;
    alt_avalon_spi_command( SPI_1_BASE, 0, 3, data, 0, 0, 0) ;
    return;
#endif
}

// Transverter write
void adf4351_write( uint32_t val ) {
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return;
#else
    union {
        uint32_t val;
        uint8_t byte[4];
    } sval;

    uint8_t t;
    sval.val = val;

    t = sval.byte[0];
    sval.byte[0] = sval.byte[3];
    sval.byte[3] = t;

    t = sval.byte[1];
    sval.byte[1] = sval.byte[2];
    sval.byte[2] = t;

    alt_avalon_spi_command( SPI_1_BASE, 1, 4, (uint8_t*)&sval.val, 0, 0, 0 ) ;
    return;
#endif
}

// SPI Read
void lms_spi_read( uint8_t address, uint8_t *val ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return;
#else
	//alt_printf("lms_spi_read(%x, %x)\n", address, *val);
    uint8_t rv ;
    if( address > 0x7f )
    {
//        alt_printf( "Invalid read address: %x\n", address ) ;
    } else {
        alt_avalon_spi_command( SPI_0_BASE, 0, 1, &address, 0, 0, ALT_AVALON_SPI_COMMAND_MERGE ) ;
        rv = alt_avalon_spi_command( SPI_0_BASE, 0, 0, 0, 1, val, 0 ) ;
        if( rv != 1 )
        {
//            alt_putstr( "SPI data read did not work :(\n") ;
        }
    }
    if( LMS_VERBOSE )
    {
//        alt_printf( "r-addr: %x data: %x\n", address, *val ) ;
    }
    return ;
#endif
}

// SPI Write
void lms_spi_write( uint8_t address, uint8_t val ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return;
#else
	//alt_printf("lms_spi_write(%x, %x)\n", address, val);
    //if( LMS_VERBOSE )
    //{
	//   alt_printf( "w-addr: %x data: %x\n", address, val ) ;
    //}
    /*if( address > 0x7f )
    {
        alt_printf( "Invalid write address: %x\n", address ) ;
    } else*/ {
        uint8_t data[2] = { address |= LMS_WRITE, val } ;
        alt_avalon_spi_command( SPI_0_BASE, 0, 2, data, 0, 0, 0 ) ;
    }
    return ;
#endif
}

// FX3 UART connection to HOST
void host_uart_write(uint8_t val) {
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	DLOG(" --> %d\n", val);
	return;
#else
	while (!(IORD_ALTERA_AVALON_UART_STATUS(UART_0_BASE) & ALTERA_AVALON_UART_STATUS_TRDY_MSK));
	IOWR_ALTERA_AVALON_UART_TXDATA(UART_0_BASE,  val);
	DLOG("write: %x\n",val);
#endif
}
// returns true if data is available.
int host_uart_hasdata( void ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return 0;
#else
	return IORD_ALTERA_AVALON_UART_STATUS(UART_0_BASE) & ALTERA_AVALON_UART_STATUS_RRDY_MSK;
#endif
}
// returns a byte only if data is available.
uint8_t host_uart_read( void ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return 0;
#else
	uint8_t val = IORD_ALTERA_AVALON_UART_RXDATA(UART_0_BASE);
	//DLOG("read: %x\n",val);
	return val;
#endif
}


// ======================================================================
// ======================= GPIO DATA FUNCTIONS ==========================
// ======================================================================

// ---------------------- READ ---------------------
#define BLADE_HW_READ_FNAME( device ) 	blade_##device##_read
#define BLADE_HW_READ_PROTO( device ) 	uint32_t blade_##device##_read(uint8_t addr, uint8_t data)
// These macros generate the function header for the read property accessors below.
// This will save time if the function signiture needs to change.

BLADE_HW_READ_PROTO( GDEV_GPIO ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return 0;
#else
	return (IORD_ALTERA_AVALON_PIO_DATA(PIO_0_BASE)) >> (addr * 8);
#endif
}
BLADE_HW_READ_PROTO( GDEV_IQ_CORR_RX_GAIN ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return 0;
#else
	return (IORD_ALTERA_AVALON_PIO_DATA(IQ_CORR_RX_PHASE_GAIN_BASE)) >> (addr * 8);
#endif
}
BLADE_HW_READ_PROTO( GDEV_IQ_CORR_RX_PHASE ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return 0;
#else
	return (IORD_ALTERA_AVALON_PIO_DATA(IQ_CORR_RX_PHASE_GAIN_BASE)) >> ((addr + 2) * 8);
#endif
}
BLADE_HW_READ_PROTO( GDEV_IQ_CORR_TX_GAIN ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return 0;
#else
	return (IORD_ALTERA_AVALON_PIO_DATA(IQ_CORR_TX_PHASE_GAIN_BASE)) >> (addr * 8);
#endif
}
BLADE_HW_READ_PROTO( GDEV_IQ_CORR_TX_PHASE ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return 0;
#else
	return (IORD_ALTERA_AVALON_PIO_DATA(IQ_CORR_TX_PHASE_GAIN_BASE)) >> ((addr + 2) * 8);
#endif
}
BLADE_HW_READ_PROTO( GDEV_FPGA_VERSION ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return (FPGA_VERSION >> (addr * 8));
#else
	return (FPGA_VERSION >> (addr * 8));
#endif
}
BLADE_HW_READ_PROTO( GDEV_TIME_TAMER ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return 0;
#else
	return IORD_8DIRECT(TIME_TAMER, addr);
#endif
}
BLADE_HW_READ_PROTO( GDEV_VCTXCO ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return 0;
#else
	// not implemented
	return 0;
#endif
}
BLADE_HW_READ_PROTO( GDEV_XB_LO ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return 0;
#else
	// not implemented
	return 0;
#endif
}
BLADE_HW_READ_PROTO( GDEV_EXPANSION ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return 0;
#else
	return (IORD_ALTERA_AVALON_PIO_DATA(PIO_1_BASE)) >> (addr * 8);
	// couldn't resist
	//return (0xdeadbeef) >> (addr * 8);
#endif
}
BLADE_HW_READ_PROTO( GDEV_EXPANSION_DIR ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return 0;
#else
	return (IORD_ALTERA_AVALON_PIO_DATA(PIO_2_BASE)) >> (addr * 8);
#endif
}


// ---------------- WRITE -----------------------------
#define BLADE_HW_WRITE_FNAME( device ) 	blade_##device##_write
#define BLADE_HW_WRITE_PROTO( device ) 	void blade_##device##_write(uint32_t addr, uint32_t data)

#define SPLIT_WRITE(reg, shift)   \
	uint32_t split_write = IORD_ALTERA_AVALON_PIO_DATA(reg); \
    split_write &= 0xffff << (16 - shift);           \
    split_write |= (data) << shift;                 \
	IOWR_ALTERA_AVALON_PIO_DATA(reg, split_write); \

BLADE_HW_WRITE_PROTO( GDEV_GPIO ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	DLOG("  -->addr: %d;  data:%d \n", addr, data);
	return;
#else
	IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, data);
#endif
}
BLADE_HW_WRITE_PROTO( GDEV_IQ_CORR_RX_GAIN ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	DLOG("  -->addr: %d;  data:%d \n", addr, data);
	return;
#else
	SPLIT_WRITE(IQ_CORR_RX_PHASE_GAIN_BASE, 0);
#endif
}
BLADE_HW_WRITE_PROTO( GDEV_IQ_CORR_RX_PHASE ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	DLOG("  -->addr: %d;  data:%d \n", addr, data);
	return;
#else
	SPLIT_WRITE(IQ_CORR_RX_PHASE_GAIN_BASE, 16);
#endif
}
BLADE_HW_WRITE_PROTO( GDEV_IQ_CORR_TX_GAIN ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	DLOG("  -->addr: %d;  data:%d \n", addr, data);
	return;
#else
	SPLIT_WRITE(IQ_CORR_TX_PHASE_GAIN_BASE, 0);
#endif
}
BLADE_HW_WRITE_PROTO( GDEV_IQ_CORR_TX_PHASE ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	DLOG("  -->addr: %d;  data:%d \n", addr, data);
	return;
#else
	SPLIT_WRITE(IQ_CORR_TX_PHASE_GAIN_BASE, 16);
#endif
}
BLADE_HW_WRITE_PROTO( GDEV_FPGA_VERSION ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	return;
#else
	// not implemented
	return;
#endif
}
BLADE_HW_WRITE_PROTO( GDEV_TIME_TAMER ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	DLOG("  -->addr: %d;  data:%d \n", addr, data);
	return;
#else
	IOWR_8DIRECT(TIME_TAMER, addr, 1);
#endif
}
BLADE_HW_WRITE_PROTO( GDEV_VCTXCO ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	DLOG("  -->addr: %d;  data:%d \n", addr, data);
	return;
#else
	dac_write(data);
#endif
}
BLADE_HW_WRITE_PROTO( GDEV_XB_LO ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	DLOG("  -->addr: %d;  data:%d \n", addr, data);
	return;
#else
	adf4351_write(data);
#endif
}
BLADE_HW_WRITE_PROTO( GDEV_EXPANSION ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	DLOG("  -->addr: %d;  data:%d \n", addr, data);
	return;
#else
	IOWR_ALTERA_AVALON_PIO_DATA(PIO_1_BASE, data);
#endif
}
BLADE_HW_WRITE_PROTO( GDEV_EXPANSION_DIR ){
#ifdef BLADE_NULL_HARDWARE
	DSHOW_CALL(__FUNCTION__);
	DLOG("  -->addr: %d;  data:%d \n", addr, data);
	return;
#else
	IOWR_ALTERA_AVALON_PIO_DATA(PIO_2_BASE, data);
#endif
}

// ======================================================================
// ======================================================================
// ======================================================================

typedef enum {
		GDEV_UNKNOWN = 0,
	    GDEV_GPIO,
	    GDEV_IQ_CORR_RX_GAIN,
	    GDEV_IQ_CORR_RX_PHASE,
	    GDEV_IQ_CORR_TX_GAIN,
	    GDEV_IQ_CORR_TX_PHASE,
	    GDEV_FPGA_VERSION,
	    GDEV_TIME_TAMER,
	    GDEV_VCTXCO,
	    GDEV_XB_LO,
	    GDEV_EXPANSION,
	    GDEV_EXPANSION_DIR
} GPIO_Device_Type;

typedef struct {
	const GPIO_Device_Type index;							// Device array index & device ID
	const uint8_t 		address;							// Specified function address.
	const uint8_t 		parameter_bytecount;				// Number of bytes used for the parameter.
	const bool 			requires_collected_write_bytes;		// If the bytes are collected before calling write.
	uint32_t (* const read)(uint8_t addr, uint8_t data); 
	void     (* const write)(uint32_t addr, uint32_t data);
} blade_device_procedure;

// this is to format the functions, check the 'blade' structure below.
const blade_device_procedure blade_function_table[] = {
		// table_index/name, 	addr,size,collectwrite,read(),write()
		{GDEV_UNKNOWN, 			-1, 0,  	false, NULL, NULL},	// unknown in here because it is number 0, so it has to be index 0
		{GDEV_GPIO,           	0,  4,  	true, BLADE_HW_READ_FNAME(GDEV_GPIO), 				BLADE_HW_WRITE_FNAME(GDEV_GPIO) },
		{GDEV_IQ_CORR_RX_GAIN,  4,  2,  	true, BLADE_HW_READ_FNAME(GDEV_IQ_CORR_RX_GAIN), 	BLADE_HW_WRITE_FNAME(GDEV_IQ_CORR_RX_GAIN) },
		{GDEV_IQ_CORR_RX_PHASE, 6,  2,  	true, BLADE_HW_READ_FNAME(GDEV_IQ_CORR_RX_PHASE), 	BLADE_HW_WRITE_FNAME(GDEV_IQ_CORR_RX_PHASE) },
		{GDEV_IQ_CORR_TX_GAIN,  8,  2,  	true, BLADE_HW_READ_FNAME(GDEV_IQ_CORR_TX_GAIN), 	BLADE_HW_WRITE_FNAME(GDEV_IQ_CORR_TX_GAIN)  },
		{GDEV_IQ_CORR_TX_PHASE, 10, 2,  	true, BLADE_HW_READ_FNAME(GDEV_IQ_CORR_TX_PHASE), 	BLADE_HW_WRITE_FNAME(GDEV_IQ_CORR_TX_PHASE) },
		{GDEV_FPGA_VERSION,  	12, 4,  	true, BLADE_HW_READ_FNAME(GDEV_FPGA_VERSION), 		BLADE_HW_WRITE_FNAME(GDEV_FPGA_VERSION) },
		{GDEV_TIME_TAMER,    	16, 16, 	false, BLADE_HW_READ_FNAME(GDEV_TIME_TAMER), 		BLADE_HW_WRITE_FNAME(GDEV_TIME_TAMER) },
		{GDEV_VCTXCO,        	34, 2,  	true, BLADE_HW_READ_FNAME(GDEV_VCTXCO), 			BLADE_HW_WRITE_FNAME(GDEV_VCTXCO) },
		{GDEV_XB_LO,         	36, 4,  	true, BLADE_HW_READ_FNAME(GDEV_XB_LO), 				BLADE_HW_WRITE_FNAME(GDEV_XB_LO) },
		{GDEV_EXPANSION,     	40, 4,  	true, BLADE_HW_READ_FNAME(GDEV_EXPANSION), 			BLADE_HW_WRITE_FNAME(GDEV_EXPANSION) },
		{GDEV_EXPANSION_DIR, 	44, 4,  	true, BLADE_HW_READ_FNAME(GDEV_EXPANSION_DIR), 		BLADE_HW_WRITE_FNAME(GDEV_EXPANSION_DIR) }

};

// Looks up the procedure from the given uart_cmd address.
// This implements the old lookup table functionality so
// no changes to the libbladerf host code need to me made.
blade_device_procedure blade_find_gpio_procedure(uint8_t addr){

	int i = 0;
	int entries = sizeof(blade_function_table)/sizeof(blade_device_procedure);
	for (i = 0; i < entries; ++i) {
		blade_device_procedure proc = blade_function_table[i];

		if (	proc.address <= addr
				&&
				(proc.address + proc.parameter_bytecount) > addr
		)
		{
			// decreasing the index by the start address is to be done elsewhere (in the router)
			return proc;
		}
	}
	return blade_function_table[0];
}


//      ____  __          __        ____            _         
//     / __ )/ /___  ____/ /____   / __ \_____   __(_)_________
//    / __  / / __ `/ __  / ___/  / / / / ___/| / / / ___/ ___/
//   / /_/ / / /_/ / /_/ /  __/  / /_/ /  __/ |/ / / /__/  __/
//  /_____/_/\__,_/\__,_/\___/  /_____/\___/|___/_/\___/\___/ 
//                                                          
//
//
// For some sugar.
// 
// blade.gpio[GDEV_EXPANSION].read( 0, &out );
// blade.gpio[GDEV_EXPANSION].write( 0, 0xdeadbeef );
// - or -
// blade.devices.lms.spi_read( addr, &out );
// 

typedef struct {
	void (* const spi_read)( uint8_t address, uint8_t *val );
	void (* const spi_write)( uint8_t address, uint8_t val );
} lms_functions;

typedef struct {
	void (* const read)( uint8_t addr, uint8_t *data );
	void (* const write)( uint8_t addr, uint8_t data );
} si5338_functions;

typedef struct {
	void (* const write)( uint16_t val );
} dac_functions;

typedef struct {
	void (* const write)( uint32_t val );
} adf4351_functions;
typedef struct {
	uint8_t (* const uart_read)( void );
	void    (* const uart_write)( uint8_t val);
	int     (* const uart_hasdata)( void );
} host_uart_functions;
typedef struct {
	const lms_functions 		lms;
	const si5338_functions 		si5338;
	const dac_functions 		dac;
	const adf4351_functions 	adf4351;
	const host_uart_functions 	host;
} blade_device_list;

const struct {
	void (* const initialize)( void );
	const blade_device_list 		devices;
	const int 						gpio_count;
	const blade_device_procedure* 	gpio;
} blade = {
	blade_initialize_hardware,
	{	// devices
		{ // lms
			lms_spi_read,
			lms_spi_write
		},
		{ // si5338
			si5338_read,
			si5338_write
		},
		{ // dac
			dac_write
		},
		{ // adf4351
			adf4351_write
		},
		{ // host uart
			host_uart_read,
			host_uart_write,
			host_uart_hasdata
		}
	},
	sizeof(blade_function_table)/sizeof(blade_device_procedure),
	blade_function_table
};


#endif /* BLADE_HARDWARE_H_ */
