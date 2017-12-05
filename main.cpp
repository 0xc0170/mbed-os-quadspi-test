#include "mbed.h"
#include "cmsis_os.h"
#include "PinNames.h"
#include "QSPI.h"

// The below values are command codes defined in Datasheet for MX25R6435F Macronix Flash Memory
// Command for reading status register
#define QSPI_STD_CMD_RDSR                   0x05
// Command for writing status register
#define QSPI_STD_CMD_WRSR                   0x01
// Command for reading control register (supported only by some memories)
#define QSPI_STD_CMD_RDCR                   0x35
// Command for writing control register (supported only by some memories)
#define QSPI_STD_CMD_WRCR                   0x3E
// Command for setting Reset Enable (supported only by some memories)
#define QSPI_STD_CMD_RSTEN                  0x66
// Command for setting Reset (supported only by some memories)
#define QSPI_STD_CMD_RST                    0x99
// Command for setting WREN (supported only by some memories)
#define QSPI_STD_CMD_WREN                   0x06
// Command for Sector erase (supported only by some memories)
#define QSPI_STD_CMD_SECT_ERASE             0x20
// Read/Write commands
#define QSPI_PP_COMMAND_NRF_ENUM            (0x0) //This corresponds to Flash command 0x02
#define QSPI_READ2O_COMMAND_NRF_ENUM        (0x1) //This corresponds to Flash command 0x3B
#define QSPI_READ2IO_COMMAND_NRF_ENUM       (0x2) //This corresponds to Flash command 0xBB
#define QSPI_PP4IO_COMMAND_NRF_ENUM         (0x3) //This corresponds to Flash command 0x38
#define QSPI_READ4IO_COMMAND_NRF_ENUM       (0x4) //This corresponds to Flash command 0xEB

//#define DEBUG_ON 1
#ifdef DEBUG_ON
    #define VERBOSE_PRINT(x) printf x
#else    
    #define VERBOSE_PRINT(x)
#endif

#define DO_TEST( test )                                 \
    {                                                   \
        printf("\nExecuting test: %-40s :", #test );    \
        if( false == test() ) {                         \
            printf(" FAILED" );                         \
        } else {                                        \
            printf(" PASSED" );                         \
        }                                               \
    }                                                   \

QSPI *myQspi = NULL;
QSPI *myQspiOther = NULL;
#define _1_K_ (0x400)
#define _4_K_ (_1_K_ * 4)
    
bool InitializeFlashMem();
bool WaitForMemReady();
bool SectorErase(unsigned int flash_addr);
bool TestWriteReadSimple();
bool TestWriteReadBlockMultiplePattern();
bool TestWriteSingleReadMultiple();
bool TestWriteMultipleReadSingle();
bool TestWriteReadMultipleObjects();
bool TestWriteReadCustomCommands();
    
// main() runs in its own thread in the OS
int main() {
    myQspi = new QSPI((PinName)QSPI_PIN_IO0, (PinName)QSPI_PIN_IO1, (PinName)QSPI_PIN_IO2, (PinName)QSPI_PIN_IO3, (PinName)QSPI_PIN_SCK, (PinName)QSPI_PIN_CSN);        
    if(myQspi) {
        printf("\nCreated QSPI driver object succesfully");
    } else {
        printf("\nERROR: Failed creating QSPI driver object");
        return -1;
    }
    
    ///////////////////////////////////////////
    // Run tests in QUADSPI 1_1_1 mode
    ///////////////////////////////////////////
    printf("\n\nQSPI Config = 1_1_1");
    if(QSPI_STATUS_OK == myQspi->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_NONE, QSPI_CFG_BUS_SINGLE, 0, 0 )) {
        printf("\nConfigured QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring QSPI driver");
        return -1;
    }
    
    if( false == InitializeFlashMem()) {
        printf("\nUnable to initialize flash memory, tests failed\n");
        return -1;
    }
        
    DO_TEST( TestWriteReadSimple );
    DO_TEST( TestWriteReadBlockMultiplePattern );
    DO_TEST( TestWriteMultipleReadSingle );
    DO_TEST( TestWriteSingleReadMultiple );
            
    ///////////////////////////////////////////
    // Run tests in QUADSPI 1_1_4 mode
    ///////////////////////////////////////////
    printf("\n\nQSPI Config = 1_1_4");
    if(QSPI_STATUS_OK == myQspi->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_NONE, QSPI_CFG_BUS_QUAD, 0, 0 )) {
        printf("\nConfigured QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring QSPI driver");
        return -1;
    }
    
    if( false == InitializeFlashMem()) {
        printf("\nUnable to initialize flash memory, tests failed\n");
        return -1;
    }
    
    DO_TEST( TestWriteReadSimple );
    DO_TEST( TestWriteReadBlockMultiplePattern );
    DO_TEST( TestWriteMultipleReadSingle );
    DO_TEST( TestWriteSingleReadMultiple );
            
    ///////////////////////////////////////////
    // Run tests in QUADSPI 1_4_4 mode
    ///////////////////////////////////////////
    printf("\n\nQSPI Config = 1_4_4");
    if(QSPI_STATUS_OK == myQspi->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_QUAD, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_NONE, QSPI_CFG_BUS_QUAD, 0, 0 )) {
        printf("\nConfigured QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring QSPI driver");
        return -1;
    }
    
    if( false == InitializeFlashMem()) {
        printf("\nUnable to initialize flash memory, tests failed\n");
        return -1;
    }
    
    DO_TEST( TestWriteReadSimple );
    DO_TEST( TestWriteReadBlockMultiplePattern );
    DO_TEST( TestWriteMultipleReadSingle );
    DO_TEST( TestWriteSingleReadMultiple );
  
////////////////////////////////////////////////////////////////////////////////////////////////////
// The Macronix Flash part on NRF52840_DK does not support Dual Mode writes. The only testing we can
// is to do a single line write and Dual-line reads which is covered by TestWriteReadCustomCommands.
// So, for now keep this tests disabled using the below DUAL_MODE_READ_ENABLED flag.    
////////////////////////////////////////////////////////////////////////////////////////////////////
//#define DUAL_MODE_READ_ENABLED    
#ifdef DUAL_MODE_READ_ENABLED    
    ///////////////////////////////////////////
    // Run tests in QUADSPI 1_1_2 mode
    ///////////////////////////////////////////
    printf("\n\nQSPI Config = 1_1_2");
    if(QSPI_STATUS_OK == myQspi->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_NONE, QSPI_CFG_BUS_DUAL, 0, 0 )) {
        printf("\nConfigured QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring QSPI driver");
        return -1;
    }
    
    if( false == InitializeFlashMem()) {
        printf("\nUnable to initialize flash memory, tests failed\n");
        return -1;
    }
    
    DO_TEST( TestWriteReadSimple );
    DO_TEST( TestWriteReadBlockMultiplePattern );
    DO_TEST( TestWriteMultipleReadSingle );
    DO_TEST( TestWriteSingleReadMultiple );
        
    ///////////////////////////////////////////
    // Run tests in QUADSPI 1_2_2 mode
    ///////////////////////////////////////////
    printf("\n\nQSPI Config = 1_2_2");
    if(QSPI_STATUS_OK == myQspi->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_DUAL, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_NONE, QSPI_CFG_BUS_DUAL, 0, 0 )) {
        printf("\nConfigured QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring QSPI driver");
        return -1;
    }
    
    if( false == InitializeFlashMem()) {
        printf("\nUnable to initialize flash memory, tests failed\n");
        return -1;
    }
    
    DO_TEST( TestWriteReadSimple );
    DO_TEST( TestWriteReadBlockMultiplePattern );
    DO_TEST( TestWriteMultipleReadSingle );
    DO_TEST( TestWriteSingleReadMultiple );
#endif //DUAL_MODE_READ_ENABLED    
    
    printf("\n\nCustom commands test uses Dual-Mode QSPI to access the flash memory" );
    DO_TEST( TestWriteReadCustomCommands );
    
    if( true == TestWriteReadMultipleObjects()) {
        printf("\nExecuting test: %-40s : PASSED", "TestWriteReadMultipleObjects" );
    } else {
        printf("\nExecuting test: %-40s : FAILED", "TestWriteReadMultipleObjects" );        
    }
    
    if(NULL != myQspi)    
        delete myQspi;
    if(NULL != myQspiOther)
        delete myQspiOther;
    
    printf("\nDone...\n");
}

bool TestWriteReadSimple()
{
    int result = 0;
    char tx_buf[] = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x10, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x2F };    
    char rx_buf[16];    
    size_t buf_len = sizeof(tx_buf);
    
    uint32_t flash_addr = 0x1000;
    if( false == SectorErase(flash_addr)) {
        printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr);
        return false;
    }
    
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    result = myQspi->write( flash_addr, tx_buf, &buf_len );
    if( ( result != QSPI_STATUS_OK ) || buf_len != sizeof(tx_buf) ) {
        printf("\nERROR: Write failed");
    }
        
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    memset( rx_buf, 0, sizeof(rx_buf) );
    result = myQspi->read( flash_addr, rx_buf, &buf_len );
    if( result != QSPI_STATUS_OK ) {
        printf("\nERROR: Read failed");
        return false;
    }
    if( buf_len != sizeof(rx_buf) ) {
        printf( "\nERROR: Unable to read the entire buffer" );
        return false;
    }
    if(0 != (memcmp( rx_buf, tx_buf, sizeof(rx_buf)))) {
        printf("\nERROR: Buffer contents are invalid"); 
        return false;
    }
    
    return true;
}

bool TestWriteReadBlockMultiplePattern()
{
    char *test_tx_buf = NULL;
    char *test_rx_buf = NULL;
    char *test_tx_buf_aligned = NULL;
    uint32_t flash_addr = 0;
    int result = 0;
    size_t buf_len = 0;
    char pattern_buf[] = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x10, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x2F };    
            
    test_tx_buf = NULL;
    test_tx_buf = (char *)malloc( _1_K_ * 2 ); //Alloc 2k to get a 1K boundary
    if(test_tx_buf == NULL) {
        printf("\nERROR: tx buf alloc failed");
        return -1;
    }
    test_tx_buf_aligned = (char *)((((uint32_t)test_tx_buf) + _1_K_) & 0xFFFFFC00);
    
    test_rx_buf = NULL;
    test_rx_buf = (char *)malloc( _1_K_ ); //Alloc 2k to get a 1K boundary
    if(test_rx_buf == NULL) {
        printf("\nERROR: rx buf alloc failed");
        return false;
    }
    
    flash_addr = 0x2000;
    for(int i=0; i < 16; i++) {
        if( false == SectorErase(flash_addr)) {
            printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr);
            return false;
        }
        
        if( false == WaitForMemReady()) {
            printf("\nDevice not ready, tests failed\n");
            return false;
        }
        
        memset( test_tx_buf_aligned, pattern_buf[i], _1_K_ );
        buf_len = _1_K_; //1k 
        result = myQspi->write( flash_addr, test_tx_buf_aligned, &buf_len );
        if( ( result != QSPI_STATUS_OK ) || buf_len != _1_K_ ) {
            printf("\nERROR: Write failed");
        }
        
        if( false == WaitForMemReady()) {
            printf("\nERROR: Device not ready, tests failed\n");
            return false;
        }
        
        memset( test_rx_buf, 0, _1_K_ );
        buf_len = _1_K_; //1k
        result = myQspi->read( flash_addr, test_rx_buf, &buf_len );
        if( result != QSPI_STATUS_OK ) {
            printf("\nERROR: Read failed");
            return false;
        }
        if( buf_len != _1_K_ ) {
            printf( "\nERROR: Unable to read the entire buffer" );
            return false;
        }
        if(0 != (memcmp( test_rx_buf, test_tx_buf_aligned, _1_K_))) {
            printf("\nERROR: Buffer contents are invalid"); 
            return false;
        }
        
        flash_addr += 0x1000;
    }
    
    free(test_rx_buf);
    free(test_tx_buf);
    
    return true;
}

bool TestWriteMultipleReadSingle()
{
    char *test_tx_buf = NULL;
    char *test_rx_buf = NULL;
    char *test_tx_buf_aligned = NULL;
    char *test_rx_buf_aligned = NULL;
    char *tmp = NULL;
    uint32_t flash_addr = 0;
    int result = 0;
    size_t buf_len = 0;
    char pattern_buf[] = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x10, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x2F };  
    unsigned int start_addr = 0x2000;
            
    test_tx_buf = NULL;
    test_tx_buf = (char *)malloc( _1_K_ * 5 ); //Alloc 5k to get a 1K boundary
    if(test_tx_buf == NULL) {
        printf("\nERROR: tx buf alloc failed");
        return -1;
    }
    test_tx_buf_aligned = (char *)((((uint32_t)test_tx_buf) + _1_K_) & 0xFFFFFC00);
    
    test_rx_buf = NULL;
    test_rx_buf = (char *)malloc( _1_K_ * 5 ); //Alloc 5k to get a 1K boundary
    if(test_rx_buf == NULL) {
        printf("\nERROR: rx buf alloc failed");
        return false;
    }
    test_rx_buf_aligned = (char *)((((uint32_t)test_rx_buf) + _1_K_) & 0xFFFFFC00);
    
    flash_addr = start_addr;
    if( false == SectorErase(flash_addr)) {
        printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr);
        return false;
    }
    
    if( false == WaitForMemReady()) {
        printf("\nDevice not ready, tests failed\n");
        return false;
    }
    
    tmp = test_tx_buf_aligned;
    for( int i=0; i < 4; i++) {
        memset( tmp, pattern_buf[i], _1_K_ );
        buf_len = _1_K_; //1k 
        result = myQspi->write( flash_addr, tmp, &buf_len );
        if( ( result != QSPI_STATUS_OK ) || buf_len != _1_K_ ) {
            printf("\nERROR: Write failed");
        }
        
        if( false == WaitForMemReady()) {
            printf("\nERROR: Device not ready, tests failed\n");
            return false;
        }
        flash_addr += _1_K_;
        tmp += _1_K_;
    }
    
    memset( test_rx_buf_aligned, 0, _4_K_ );
    buf_len = _4_K_; //1k
    flash_addr = start_addr;
    result = myQspi->read( flash_addr, test_rx_buf_aligned, &buf_len );
    if( result != QSPI_STATUS_OK ) {
        printf("\nERROR: Read failed");
        return false;
    }
    if( buf_len != _4_K_ ) {
        printf( "\nERROR: Unable to read the entire buffer" );
        return false;
    }
    if(0 != (memcmp( test_rx_buf_aligned, test_tx_buf_aligned, _4_K_))) {
        printf("\nERROR: Buffer contents are invalid"); 
        return false;
    } 
    
    free(test_rx_buf);
    free(test_tx_buf);
    
    return true;
}

bool TestWriteSingleReadMultiple()
{
    char *test_tx_buf = NULL;
    char *test_rx_buf = NULL;
    char *test_tx_buf_aligned = NULL;
    char *test_rx_buf_aligned = NULL;
    char *tmp = NULL;
    uint32_t flash_addr = 0;
    int result = 0;
    size_t buf_len = 0;
    char pattern_buf[] = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x10, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x2F };  
    unsigned int start_addr = 0x2000;
            
    test_tx_buf = NULL;
    test_tx_buf = (char *)malloc( _1_K_ * 5 ); //Alloc 5k to get a 1K boundary
    if(test_tx_buf == NULL) {
        printf("\nERROR: tx buf alloc failed");
        return -1;
    }
    test_tx_buf_aligned = (char *)((((uint32_t)test_tx_buf) + _1_K_) & 0xFFFFFC00);
    
    test_rx_buf = NULL;
    test_rx_buf = (char *)malloc( _1_K_ * 5 ); //Alloc 5k to get a 1K boundary
    if(test_rx_buf == NULL) {
        printf("\nERROR: rx buf alloc failed");
        return false;
    }
    test_rx_buf_aligned = (char *)((((uint32_t)test_rx_buf) + _1_K_) & 0xFFFFFC00);
    
    flash_addr = start_addr;
    if( false == SectorErase(flash_addr)) {
        printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr);
        return false;
    }
    
    if( false == WaitForMemReady()) {
        printf("\nDevice not ready, tests failed\n");
        return false;
    }
    
    tmp = test_tx_buf_aligned;
    for( int i=0; i < 4; i++) {
        memset( tmp, pattern_buf[i], _1_K_ );
        tmp += _1_K_;
    }
    
    buf_len = _4_K_; //4k 
    result = myQspi->write( flash_addr, test_tx_buf_aligned, &buf_len );
    if( ( result != QSPI_STATUS_OK ) || buf_len != _4_K_ ) {
        printf("\nERROR: Write failed");
    }
    
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    memset( test_rx_buf_aligned, 0, _4_K_ );
    
    buf_len = _1_K_; //1k
    flash_addr = start_addr;
    tmp = test_rx_buf_aligned;
    for( int i=0; i < 4; i++) {
        result = myQspi->read( flash_addr, tmp, &buf_len );
        if( result != QSPI_STATUS_OK ) {
            printf("\nERROR: Read failed");
            return false;
        }
        if( buf_len != _1_K_ ) {
            printf( "\nERROR: Unable to read the entire buffer" );
            return false;
        }
        tmp += _1_K_;
        flash_addr += _1_K_;
    }
    if(0 != (memcmp( test_rx_buf_aligned, test_tx_buf_aligned, _4_K_))) {
        printf("\nERROR: Buffer contents are invalid"); 
        return false;
    } 
    
    free(test_rx_buf);
    free(test_tx_buf);
    
    return true;
}

bool TestWriteReadMultipleObjects()
{
    unsigned int flash_addr1 = 0x2000;
    unsigned int flash_addr2 = 0x4000;
    
    myQspiOther = new QSPI((PinName)QSPI_PIN_IO0, (PinName)QSPI_PIN_IO1, (PinName)QSPI_PIN_IO2, (PinName)QSPI_PIN_IO3, (PinName)QSPI_PIN_SCK, (PinName)QSPI_PIN_CSN);        
    if(myQspiOther) {
        printf("\nCreated 2nd QSPI driver object succesfully");
    } else {
        printf("\nERROR: Failed creating 2nd QSPI driver object");
        return -1;
    }
    
    ////////////////////////////////////////////////////
    // Configure myQspiOther object to do 1_1_1 mode
    ////////////////////////////////////////////////////
    if( QSPI_STATUS_OK == myQspiOther->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_NONE, QSPI_CFG_BUS_SINGLE, 0, 0 )) {
        printf("\nConfigured 2nd QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring 2nd QSPI object");
        return -1;
    }
    
    //////////////////////////////////////////////
    // Configure myQspi object to do 1_4_4 mode
    //////////////////////////////////////////////
    printf("\n\nQSPI Config = 1_4_4");
    if(QSPI_STATUS_OK == myQspi->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_QUAD, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_NONE, QSPI_CFG_BUS_QUAD, 0, 0 )) {
        printf("\nConfigured QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring QSPI driver");
        return -1;
    }
    
    for(int i=0; i < 10; i++) {
        int result = 0;
        char tx_buf[] = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x10, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x2F };    
        char rx_buf[16];    
        size_t buf_len = sizeof(tx_buf);
        
        if( false == SectorErase(flash_addr1)) {
            printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr1);
            return false;
        }
        
        // Wait for FlashMem to be ready
        if( false == WaitForMemReady()) {
            printf("\nERROR: Device not ready, tests failed\n");
            return false;
        }
        
        result = myQspi->write( flash_addr1, tx_buf, &buf_len );
        if( (result != QSPI_STATUS_OK) || buf_len != sizeof(tx_buf) ) {
            printf("\nERROR: Write failed");
        }
            
        if( false == WaitForMemReady()) {
            printf("\nERROR: Device not ready, tests failed\n");
            return false;
        }
        
        memset( rx_buf, 0, sizeof(rx_buf) );
        result = myQspi->read( flash_addr1, rx_buf, &buf_len );
        if(result != QSPI_STATUS_OK) {
            printf("\nERROR: Read failed");
            return false;
        }
        if( buf_len != sizeof(rx_buf) ) {
            printf( "\nERROR: Unable to read the entire buffer" );
            return false;
        }
        if(0 != (memcmp( rx_buf, tx_buf, sizeof(rx_buf)))) {
            printf("\nERROR: Buffer contents are invalid"); 
            return false;
        }
        
        //Now use other object to deal with other part of memory
        if( false == SectorErase(flash_addr2)) {
            printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr2);
            return false;
        }
        
        // Wait for FlashMem to be ready
        if( false == WaitForMemReady()) {
            printf("\nERROR: Device not ready, tests failed\n");
            return false;
        }
        
        result = myQspiOther->write( flash_addr2, tx_buf, &buf_len );
        if( (result != QSPI_STATUS_OK) || buf_len != sizeof(tx_buf) ) {
            printf("\nERROR: Write failed");
        }
            
        if( false == WaitForMemReady()) {
            printf("\nERROR: Device not ready, tests failed\n");
            return false;
        }
        
        memset( rx_buf, 0, sizeof(rx_buf) );
        result = myQspiOther->read( flash_addr2, rx_buf, &buf_len );
        if(result != QSPI_STATUS_OK) {
            printf("\nERROR: Read failed");
            return false;
        }
        if( buf_len != sizeof(rx_buf) ) {
            printf( "\nERROR: Unable to read the entire buffer" );
            return false;
        }
        if(0 != (memcmp( rx_buf, tx_buf, sizeof(rx_buf)))) {
            printf("\nERROR: Buffer contents are invalid"); 
            return false;
        }
        osDelay(100);
    }    
    
    return true;
}

bool TestWriteReadCustomCommands()
{
    int result = 0;
    char tx_buf[] = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x10, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x2F };    
    char rx_buf[16];    
    size_t buf_len = sizeof(tx_buf);
    
    uint32_t flash_addr = 0x1000;
    
    //Try 1-1-2 mode using custom commands
    if( false == SectorErase(flash_addr)) {
        printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr);
        return false;
    }
    
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    result = myQspi->write( QSPI_PP_COMMAND_NRF_ENUM, flash_addr, 0, tx_buf, &buf_len );
    if( (result != QSPI_STATUS_OK) || buf_len != sizeof(tx_buf) ) {
        printf("\nERROR: Write failed");
    }
        
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    memset( rx_buf, 0, sizeof(rx_buf) );
    result = myQspi->read( QSPI_READ2O_COMMAND_NRF_ENUM, flash_addr, 0, rx_buf, &buf_len );
    if(result != QSPI_STATUS_OK) {
        printf("\nERROR: Read failed");
        return false;
    }
    if( buf_len != sizeof(rx_buf) ) {
        printf( "\nERROR: Unable to read the entire buffer" );
        return false;
    }
    if(0 != (memcmp( rx_buf, tx_buf, sizeof(rx_buf)))) {
        printf("\nERROR: Buffer contents are invalid"); 
        return false;
    }
    
    //Try 1-2-2 mode using custom commands
    if( false == SectorErase(flash_addr)) {
        printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr);
        return false;
    }
    
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    result = myQspi->write( QSPI_PP_COMMAND_NRF_ENUM, flash_addr, 0, tx_buf, &buf_len );
    if( (result != QSPI_STATUS_OK) || buf_len != sizeof(tx_buf) ) {
        printf("\nERROR: Write failed");
    }
        
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    memset( rx_buf, 0, sizeof(rx_buf) );
    result = myQspi->read( QSPI_READ2IO_COMMAND_NRF_ENUM, flash_addr, 0, rx_buf, &buf_len );
    if(result != QSPI_STATUS_OK) {
        printf("\nERROR: Read failed");
        return false;
    }
    if( buf_len != sizeof(rx_buf) ) {
        printf( "\nERROR: Unable to read the entire buffer" );
        return false;
    }
    if(0 != (memcmp( rx_buf, tx_buf, sizeof(rx_buf)))) {
        printf("\nERROR: Buffer contents are invalid"); 
        return false;
    }
    
    return true;
}

bool InitializeFlashMem()
{
    bool ret_status = true;
    char status_value[2];
    
    //Read the Status Register from device
    if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_RDSR, // command to send
                              0,                 // do not transmit
                              NULL,              // do not transmit
                              status_value,                 // just receive two bytes of data
                              2)) {   // store received values in status_value
        VERBOSE_PRINT(("\nReading Status Register Success: value = 0x%02X:0x%02X\n", status_value[0], status_value[1]));
    } else {
        printf("\nERROR: Reading Status Register failed\n");
        ret_status = false;
    }
    
    if(ret_status)
    {
        //Send Reset Enable
        if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_RSTEN, // command to send
                                  0,                 // do not transmit
                                  NULL,              // do not transmit
                                  0,                 // just receive two bytes of data
                                  NULL)) {   // store received values in status_value
            VERBOSE_PRINT(("\nSending RSTEN Success\n"));
        } else {
            printf("\nERROR: Sending RSTEN failed\n");
            ret_status = false;
        }
        
        if(ret_status)
        {
            //Send Reset
            if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_RST, // command to send
                                      0,                 // do not transmit
                                      NULL,              // do not transmit
                                      status_value,                 // just receive two bytes of data
                                      2)) {   // store received values in status_value
                VERBOSE_PRINT(("\nSending RST Success\n"));
            } else {
                printf("\nERROR: Sending RST failed\n");
                ret_status = false;
            }
            
            if(ret_status)
            {
                status_value[0] |= 0x40;
                //Write the Status Register to set QE enable bit
                if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_WRSR, // command to send
                                          status_value,                 
                                          1,      
                                          NULL,                 
                                          0)) {   // store received values in status_value
                    VERBOSE_PRINT(("\nWriting Status Register Success\n"));
                } else {
                    printf("\nERROR: Writing Status Register failed\n");
                    ret_status = false;
                }
            }
        }
    }
    
    return ret_status;
}

bool WaitForMemReady()
{
    char status_value[2];
    int retries = 0;
    
    do
    {
        retries++;
        //Read the Status Register from device
        if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_RDSR, // command to send
                                  0,                 // do not transmit
                                  NULL,              // do not transmit
                                  status_value,                 // just receive two bytes of data
                                  2)) {   // store received values in status_value
            VERBOSE_PRINT(("\nReadng Status Register Success: value = 0x%02X:0x%02X\n", status_value[0], status_value[1]));
        } else {
            printf("\nERROR: Reading Status Register failed\n");
        }
    } while( (status_value[0] & 0x1) != 0 && retries <10000 );
    
    if((status_value[0] & 0x1) != 0) return false;
    return true;
}

bool SectorErase(unsigned int flash_addr)
{
    char addrbytes[3] = {0};
    
    addrbytes[2]=flash_addr & 0xFF;
    addrbytes[1]=(flash_addr >> 8) & 0xFF;
    addrbytes[0]=(flash_addr >> 16) & 0xFF;
            
    //Send WREN
    if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_WREN, // command to send
                              0,                 // do not transmit
                              NULL,              // do not transmit
                              0,                 // just receive two bytes of data
                              NULL)) {   // store received values in status_value
        VERBOSE_PRINT(("\nSending WREN command success\n"));
    } else {
        printf("\nERROR: Sending WREN command failed\n");
        return false;
    }
    
    if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_SECT_ERASE, // command to send
                              addrbytes,                 // do not transmit
                              3,              // do not transmit
                              0,                 // just receive two bytes of data
                              NULL)) {   // store received values in status_value
        VERBOSE_PRINT(("\nSending SECT_ERASE command success\n"));
    } else {
        printf("\nERROR: Readng SECT_ERASE command failed\n");
        return false;
    }
    
    return true;
}

