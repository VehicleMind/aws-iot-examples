// Debug mode - Uncomment it to activate debug mode
// #define DEBUG

// #define MEMS_PACKET_DATA_SIZE 32 // 8 (long timestamp) + 12 (float[3] acc) + 4 (float pitch) + 4 (float roll) + 4 (float yaw) 
// DMP
// #define MEMS_PACKET_DATA_SIZE 48 // 8 (long timestamp) + 1 (byte pid class) + 1 (byte length) + 3 * (2 (short pid) + 2 (bytes type&length)) + 26 (byte value)

// #define GPS_PACKET_DATA_SIZE 22 // 8 (long timestamp) + 4 (int lat) + 4 (int lng) + 2 (short alt) + 1 (byte speed) + 1 (byte sat) + 2 (short heading)

#define MEMS_PACKET_DATA_SIZE 64 // 8 (long timestamp) + 1 (byte pid class) + 1 (byte length) + 4 * (2 (short pid) + 2 (bytes type&length)) + 38 (byte value)
#define GPS_PACKET_DATA_SIZE 48 // 8 (long timestamp) + 1 (byte pid class) + 1 (byte length) + 6 * (2 (short pid) + 2 (bytes type&length)) + 14 (byte value)
#define OBD_PACKET_DATA_SIZE 18 // 8 (long timestamp) + 1 (byte pid class) + 1 (byte length) + 2 (short pid) + 2 (bytes type&length) + X (byte value)
#define PACKET_BUFFER_SIZE 250 // Tweeked to some value that sends without failure

// mems
#define MEMS_DISABLED 0
#define MEMS_ACC 1
#define MEMS_9DOF 2
#define MEMS_DMP 3

#define ENABLE_ORIENTATION 1

// motion detection
#define WAKEUP_MOTION_THRESHOLD 30 /* for wakeup on movement */
#define CALIBRATION_TIME 3000 /* ms */

// gps
#define GPS_SERIAL_BAUDRATE 115200L

// obd
#define OBD_NONE 0
#define OBD_UART 1
#define OBD_SPI 2