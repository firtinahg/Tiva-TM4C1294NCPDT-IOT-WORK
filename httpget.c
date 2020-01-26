
/*
 *  ======== httpget.c ========
 *  HTTP Client GET example application
 */
#include <string.h>
#include <stdlib.h>
/* XDCtools Header files */
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

/* TI-RTOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Idle.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/drivers/GPIO.h>
#include <ti/net/http/httpcli.h>
#include <ti/drivers/I2C.h>
#include <math.h>
/* Example/Board Header file */
#include "Board.h"

#include <sys/socket.h>

#define HOSTNAME "api.openweathermap.org"
#define REQUEST_URI "/data/2.5/forecast?id=524901&APPID=e7d4c1e2dc96c1b17ab826b1bf3d7ff0"
#define USER_AGENT "HTTPCli (ARM; TI-RTOS)"
#define SOCKETTEST_IP "10.10.96.97"
#define HOSTNAME2 "api.weatherstack.com"
#define REQUEST_URI2 "/current?access_key=7784f93c611c00e84ab044d0146b6cb5&query=Eskisehir"
#define TASKSTACKSIZE 4096
Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];
extern Semaphore_Handle semaphore0;
extern Semaphore_Handle semaphore1;
extern Semaphore_Handle semaphore2;
extern Semaphore_Handle semaphore3;
extern Event_Handle event0;
char tempstr[20];
char tempstr_2[20];
char press_str[20];
char press_str_2[20];
char Humidity_str[20];
char Humidity_str_2[20];
#define Board_BMP180_ADDR 0x77
int TEMP_VALUE_FOR_ACU;
uint16_t temperature_TMP006;
float temp, press, alt;

I2C_Handle i2c;
I2C_Params i2cParams;
I2C_Transaction i2cTransaction;

uint8_t txBuffer[4];
uint8_t rxBuffer[30];



/////////////////////////////////////////////////  Related Parts with BMP180 \\\\\\\\\\\\\\\\\\\\\\\\\

short AC1, AC2, AC3, B1, B2, MB, MC, MD; // calibration variables
unsigned short AC4, AC5, AC6;            // calibration variables
long UT, UP;                             //uncompensated temperature and pressure
float B3, B4, B6, B7, X1t, X1p, X2t, X2p, X3p, B5t, B5p, Altitude;

void BMP180_getPressureCalibration(void)
{
  txBuffer[0] = 0xAA;
  i2cTransaction.slaveAddress = Board_BMP180_ADDR;
  i2cTransaction.writeBuf = txBuffer;
  i2cTransaction.writeCount = 1;
  i2cTransaction.readBuf = rxBuffer;
  i2cTransaction.readCount = 22;

  if (I2C_transfer(i2c, &i2cTransaction))
  {
    //   System_printf("Calibration data acquired\n");

    AC1 = rxBuffer[0] << 8 | rxBuffer[1];
    AC2 = rxBuffer[2] << 8 | rxBuffer[3];
    AC3 = rxBuffer[4] << 8 | rxBuffer[5];
    AC4 = rxBuffer[6] << 8 | rxBuffer[7];
    AC5 = rxBuffer[8] << 8 | rxBuffer[9];
    AC6 = rxBuffer[10] << 8 | rxBuffer[11];
    B1 = rxBuffer[12] << 8 | rxBuffer[13];
    B2 = rxBuffer[14] << 8 | rxBuffer[15];
    MB = rxBuffer[16] << 8 | rxBuffer[17];
    MC = rxBuffer[18] << 8 | rxBuffer[19];
    MD = rxBuffer[20] << 8 | rxBuffer[21];
  }
}

void BMP180_startTemperatureAcquisition(void)
{
  txBuffer[0] = 0xf4;                              // control register
  txBuffer[1] = 0x2e;                              // temperature conversion command
  i2cTransaction.slaveAddress = Board_BMP180_ADDR; // 0x77
  i2cTransaction.writeBuf = txBuffer;              // transmit buffer
  i2cTransaction.writeCount = 2;                   // two bytes will be sent
  i2cTransaction.readBuf = rxBuffer;               // receive buffer
  i2cTransaction.readCount = 0;                    // we are expecting 2 bytes

  if (I2C_transfer(i2c, &i2cTransaction))
  {
    // System_printf("Temperature acquisition initiated\n");
  }
}

void BMP180_startPressureAcquisition(void)
{
  txBuffer[0] = 0xf4;                              // control register
  txBuffer[1] = 0x34;                              // pressure conversion command
  i2cTransaction.slaveAddress = Board_BMP180_ADDR; // 0x77
  i2cTransaction.writeBuf = txBuffer;              // transmit buffer
  i2cTransaction.writeCount = 2;                   // two bytes will be sent
  i2cTransaction.readBuf = rxBuffer;               // receive buffer
  i2cTransaction.readCount = 0;                    // we are expecting 2 bytes

  if (I2C_transfer(i2c, &i2cTransaction))
  {
    //System_printf("Pressure acquisition initiated\n");
  }
}

float BMP180_getTemperature(void)
{
  float temp;

  txBuffer[0] = 0xf6;                              // temperature register
  i2cTransaction.slaveAddress = Board_BMP180_ADDR; // 0x77
  i2cTransaction.writeBuf = txBuffer;              // transmit buffer
  i2cTransaction.writeCount = 1;                   // two bytes will be sent
  i2cTransaction.readBuf = rxBuffer;               // receive buffer
  i2cTransaction.readCount = 2;                    // we are expecting 2 bytes

  if (I2C_transfer(i2c, &i2cTransaction))
  {
    //  System_printf("Temperature value acquired\n");
  }

  UT = rxBuffer[0] << 8 | rxBuffer[1]; //UT = raw temperature data
  //System_printf("Uncompansated Temperature : %d\n", UT);

  //compute temperature
  X1t = ((UT - AC6) * AC5) >> 15;
  X2t = (MC << 11) / (X1t + MD);
  B5t = X1t + X2t;
  temp = ((B5t + 8) / 16) / 10;

  return temp;
}

float BMP180_getPressure(void)
{
  float pressure;

  txBuffer[0] = 0xf6;                              // temperature register
  i2cTransaction.slaveAddress = Board_BMP180_ADDR; // 0x77
  i2cTransaction.writeBuf = txBuffer;              // transmit buffer
  i2cTransaction.writeCount = 1;                   // two bytes will be sent
  i2cTransaction.readBuf = rxBuffer;               // receive buffer
  i2cTransaction.readCount = 2;                    // we are expecting 2 bytes

  if (I2C_transfer(i2c, &i2cTransaction))
  {
    //  System_printf("Pressure value acquired\n");
  }

  UP = rxBuffer[0] << 8 | rxBuffer[1]; //UT = raw pressure data
                                       // System_printf("Uncompansated Pressure : %d\n", UP);

  //compute pressure
  B6 = B5t - 4000;
  X1p = (B2 * (B6 * B6 / 4096)) / 2048;
  X2p = AC2 * B6 / 2048;
  X3p = X1p = X2p;
  B3 = ((((long)AC1 * 4 + X3p)) + 2) / 4;
  X1p = AC3 * B6 / 8192;
  X2p = (B1 * (B6 * B6 / 4096)) / 65536;
  X3p = ((X1p + X2p) + 2) / 4;
  B4 = AC4 * (unsigned long)(X3p + 32768) / 32768;
  B7 = ((unsigned long)UP - B3) * (50000);
  if (B7 < 0x80000000)
  {
    pressure = (B7 * 2) / B4;
  }
  else
  {
    pressure = (B7 / B4) * 2;
  }
  X1p = (pressure / 256) * (pressure / 256);
  X1p = (X1p * 3038) / 65536;
  X2p = (-7357 * pressure) / 65536;
  pressure = pressure + (X1p + X2p + 3791) / 16;

  return pressure;
}

float BMP180_calculateAltitude(float pressure)
{
  float alt;

  // compute altitude; uses default sea level pressure; altitude will vary
  // depending on difference between default sea level pressure (101325 Pascal)
  // and the actual pressure
  //
  alt = 44330.0f * (1.0f - powf(pressure / 101325.0f, 1 / 5.255f));

  //  System_printf("Altitude calculated\n");
  return alt;
}

void initializeI2C()
{
  // Create I2C interface for sensor usage
  //
  I2C_Params_init(&i2cParams);
  i2cParams.bitRate = I2C_100kHz; // It can be I2C_400kHz orI2C_100kHz

  // Let's open the I2C interface
  //
  i2c = I2C_open(Board_I2C0, &i2cParams); // actually I2C7
  if (i2c == NULL)
  {
    // error initializing IIC
    //
    System_abort("Error Initializing I2C\n");
  }

  //  System_printf("I2C Initialized!\n");
}

void closeI2C(void)
{
  // close the interface
  //
  I2C_close(i2c);

  // System_printf("I2C interface closed\n");
}




Void timerISR(UArg arg0)
{ //Clock ticks/////////////////////////////////////////

  Semaphore_post(semaphore0);
}



/////////////////////////////////////////////////Related PARTS with BMP180 \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\


Void TaskForTMP006(UArg arg0, UArg arg1)
{

  while (1)
  {

    Semaphore_pend(semaphore0, BIOS_WAIT_FOREVER);


    uint8_t txBuffer[6];
    uint8_t rxBuffer[6];
    I2C_Handle i2c;
    I2C_Params i2cParams;
    I2C_Transaction i2cTransaction;

    // Create I2C interface for sensor usage
    //
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz; // It can be I2C_400kHz orI2C_100kHz

    // Let's open the I2C interface
    //
    i2c = I2C_open(Board_I2C_TMP, &i2cParams); // Board_I2C_TMP is actually I2C7
    if (i2c == NULL)
    {
      // error initializing IIC
      //
      System_abort("Error Initializing I2C\n");
    }

    //   System_printf("I2C Initialized!\n");

    // Point to the T ambient register and read its 2 bytes (actually 14 bits)
    // register number is 0x01.
    //
    txBuffer[0] = 0x01;                 // Ambient temperature register
    i2cTransaction.slaveAddress = 0x41; // For SENSHUB it is 0x41
    i2cTransaction.writeBuf = txBuffer; // transmit buffer
    i2cTransaction.writeCount = 1;      // only one byte will be sent
    i2cTransaction.readBuf = rxBuffer;  // receive buffer
    i2cTransaction.readCount = 2;       // we are expecting 2 bytes

    // carry out the I2C transfer. The received 16 bits is in big endian format since IIC
    // protocol sends the most significant byte first (i.e. rxBuffer[0]) and then
    // least significant byte (i.e. rxBuffer[1]).
    //
    // Remember that temperature register is 14 bits and we need to shift right 2 bits
    // to get a number. We need to divide it by 32 to get the temperature value.
    //
    if (I2C_transfer(i2c, &i2cTransaction))
    {

      // 14 bit to 16 bit conversion since least 2 bits are 0s
      //
      temperature_TMP006 = (rxBuffer[0] << 6) | (rxBuffer[1] >> 2);

      // This time we are going to check whether the number is negative or not.
      // If it is negative, we will sign extend to 16 bits.
      //
      if (rxBuffer[0] & 0x80)
      {
        temperature_TMP006 |= 0xF000;
      }

      // We need to divide by 32 to get the actual temperature value.
      // Check with the TMP006 datasheet
      //
      temperature_TMP006 /= 32;
      System_printf("TMP006 temperature value : %d\n ", temperature_TMP006);
    }
    else
    {

      // no response from TMP006. Is it there?
      //
      System_printf("I2C Bus fault\n");
    }

    // flush everything to the console
    //
    System_flush();

    // close the interface
    //
    I2C_close(i2c);

    Semaphore_post(semaphore1);
    Event_post(event0 , Event_Id_01);

  }

}                                                                     // |||| \\
                     /////////////////////////////////////////////// TaskFor TMP006//////////////

//////////////////////////////////////////////////TASK FOR BMP180 \\\\\\\\\\\\

float temp, press, alt;

Void TaskForBMP180(UArg arg0, UArg arg1)
{

  while (1)
  {
    Semaphore_pend(semaphore1, BIOS_WAIT_FOREVER);

    initializeI2C();

    // get pressure calibration data
    //
    BMP180_getPressureCalibration();

    // start temperature acquisition
    //
    BMP180_startTemperatureAcquisition();

    // wait for 5 mseconds for the acquisition
    //
    Task_sleep(5);

    // get the uncompensated temperature value
    //
    temp = BMP180_getTemperature();

    // start pressure acquisition
    //
    BMP180_startPressureAcquisition();
    System_flush();

    // wait for 5 mseconds for the acquisition
    //
    Task_sleep(5);

    // get the uncompensated pressure value
    // The sea level pressure is 101325 pascal
    //
    press = BMP180_getPressure();

    // get the altitude
    //
    alt = BMP180_calculateAltitude(press);

    System_printf("BMP180 Temperature: %d\nBMP180  Pressure: %d\n    ", (int)temp, (int)press );

    I2C_close(i2c);

    System_flush();

    Semaphore_post(semaphore2);
    Event_post(event0, Event_Id_02);
  }
}
//// |||| \\\\\
/////////////////////////////////////////////////  TASK FOR BMP 180 ////////////////////////////////////

void printError(char *errString, int code)
{
  System_printf("Error! code = %d, desc = %s\n", code, errString);
  BIOS_exit(code);
}

void sendData2Server(char *serverIP, int serverPort, char *data, int size)
{
  int sockfd;
  struct sockaddr_in serverAddr;

  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd == -1)
  {
    System_printf("Socket not created");
    BIOS_exit(-1);
  }

  memset(&serverAddr, 0, sizeof(serverAddr)); /* clear serverAddr structure */
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(serverPort); /* convert port # to network order */
  inet_pton(AF_INET, serverIP, &(serverAddr.sin_addr));

  int connStat = connect(sockfd, (struct sockaddr *)&serverAddr, /* connecting….*/
                         sizeof(serverAddr));
  if (connStat < 0)
  {
    System_printf("Error while connecting to server\n");
    if (sockfd > 0)
      close(sockfd);
    BIOS_exit(-1);
  }

  int numSend = send(sockfd, data, size, 0); /* send data to the server*/
  if (numSend < 0)
  {
    System_printf("Error while sending data to server\n");
    if (sockfd > 0)
      close(sockfd);
    BIOS_exit(-1);
  }

  if (sockfd > 0)
    close(sockfd);
}

Void socketTask(UArg arg0, UArg arg1)
{
  while (1)
  {
    // wait for the semaphore that httpTask() will signal
    // when temperature string is retrieved from api.openweathermap.org site
    //
    Event_pend(event0, Event_Id_03  , Event_Id_NONE ,  BIOS_WAIT_FOREVER);

    GPIO_write(Board_LED0, 1); // turn on the LED
System_printf("Socket started\n");
System_flush();
    // connect to SocketTest program on the system with given IP/port
    // send hello message whihc has a length of 5.
    //
 String TMP006_TEMPERATURE_VALUE = (String)temperature_TMP006;
 String MY_TEMP_FROM_ACU = (String) TEMP_VALUE_FOR_ACU;
 String MY_TEMP_FROM_WEAT =(String) tempstr_2;
 String MY_PRESS_FROM_WEAT = (String) press_str_2;
 String MY_PRESS_FORM_ACU =(String) press_str;
int my_new_temp_is = (int) temp;
int my_press_value =(int) press;
//String BMP180_TEMP = (String)my_new_temp_is ;
String BMP180_PRESS =(String) my_press_value;
String MY_BIG_STRING="";
strcpy(MY_BIG_STRING,TMP006_TEMPERATURE_VALUE)  ;
strcpy(MY_BIG_STRING,MY_PRESS_FORM_ACU)  ;
strcpy(MY_BIG_STRING,MY_TEMP_FROM_ACU)  ;
strcpy(MY_BIG_STRING,MY_PRESS_FROM_WEAT)  ;
strcpy(MY_BIG_STRING,MY_TEMP_FROM_WEAT)  ;
strcpy(MY_BIG_STRING,BMP180_PRESS)  ;


sendData2Server(SOCKETTEST_IP, 5011, MY_BIG_STRING, strlen(MY_BIG_STRING));

   /* sendData2Server(SOCKETTEST_IP, 5011, my_temp_for_acu, strlen(my_temp_for_acu));
    sendData2Server(SOCKETTEST_IP, 5011, tempstr_2, strlen(tempstr_2));


    sendData2Server(SOCKETTEST_IP, 5011, press_str, strlen(press_str));

    sendData2Server(SOCKETTEST_IP, 5011, press_str_2, strlen(press_str_2));

    sendData2Server(SOCKETTEST_IP, 5011, Humidity_str, strlen(Humidity_str));

    sendData2Server(SOCKETTEST_IP, 5011, my_tmp_is, strlen(my_tmp_is));

    sendData2Server(SOCKETTEST_IP, 5011, my_new_temp_is_1, strlen(my_new_temp_is_1));

    sendData2Server(SOCKETTEST_IP, 5011,my_press_value_1, strlen(my_press_value_1));

*/
    GPIO_write(Board_LED0, 0); // turn off the LED

    // wait for 5 seconds (5000 ms)
    //
    Task_sleep(1000);
  }
}

/*
 *  ======== httpTask ========
 *  Makes a HTTP GET request
 */

Void httpTask(UArg arg0, UArg arg1)
{
  bool moreFlag = false;
  char data[64], *s1, *s2;
  int ret, temp_received = 0, len;
  struct sockaddr_in addr;

  HTTPCli_Struct cli;
  HTTPCli_Field fields[3] = {
      {HTTPStd_FIELD_NAME_HOST, HOSTNAME},
      {HTTPStd_FIELD_NAME_USER_AGENT, USER_AGENT},
      {NULL, NULL}};

  while (1)
  {
    Semaphore_pend(semaphore2, BIOS_WAIT_FOREVER);

    System_printf("Sending a HTTP GET request to '%s'\n", HOSTNAME);
    System_flush();

    HTTPCli_construct(&cli);

    HTTPCli_setRequestFields(&cli, fields);
    System_flush();

    ret = HTTPCli_initSockAddr((struct sockaddr *)&addr, HOSTNAME, 0);
    System_flush();
    if (ret < 0)
    {
      printError("httpTask: address resolution failed", ret);
    }

    ret = HTTPCli_connect(&cli, (struct sockaddr *)&addr, 0, NULL);
    System_flush();
    if (ret < 0)
    {
      printError("httpTask: connect failed", ret);
    }

    ret = HTTPCli_sendRequest(&cli, HTTPStd_GET, REQUEST_URI, false);
    if (ret < 0)
    {
      printError("httpTask: send failed", ret);
    }

    ret = HTTPCli_getResponseStatus(&cli);
    if (ret != HTTPStd_OK)
    {
      printError("httpTask: cannot get status", ret);
    }

    System_printf("HTTP Response Status Code: %d\n", ret);

    ret = HTTPCli_getResponseField(&cli, data, sizeof(data), &moreFlag);
    if (ret != HTTPCli_FIELD_ID_END)
    {
      printError("httpTask: response field processing failed", ret);
    }

    len = 0;
    do
    {
      ret = HTTPCli_readResponseBody(&cli, data, sizeof(data), &moreFlag);
      if (ret < 0)
      {
        printError("httpTask: response body processing failed", ret);
      }
      else
      {
        // string is read correctly
        // find "temp:" string
        //
        s1 = strstr(data, "temp");
        if (s1)
        {
          if (temp_received)
            continue; // temperature is retrieved before, continue
          // is s1 is not null i.e. "temp" string is found
          // search for comma
          s2 = strstr(s1, ",");
          if (s2)
          {
            *s2 = 0;                 // put end of string
            strcpy(tempstr, s1 + 6); // copy the string
            temp_received = 1;
          }
        }
        s1 = strstr(data, "pressure");
        if (s1)
        {

          // is s1 is not null i.e. "temp" string is found
          // search for comma
          s2 = strstr(s1, ",");
          if (s2)
          {
            *s2 = 0;                    // put end of string
            strcpy(press_str, s1 + 10); // copy the string

            temp_received = 1;
          }
        }
        s1 = strstr(data, "humidity");
        if (s1)
        {

          // is s1 is not null i.e. "temp" string is found
          // search for comma
          s2 = strstr(s1, ",");
          if (s2)
          {
            *s2 = 0;                       // put end of string
            strcpy(Humidity_str, s1 + 10); // copy the string

            temp_received = 1;
          }
        }
      }

      len += ret; // update the total string length received so far
    } while (moreFlag);


    System_printf("Recieved %d bytes of payload\n", len);

    System_flush(); // write logs to console

    int TEMP_VALUE_FOR_ACU = atoi(tempstr);
    TEMP_VALUE_FOR_ACU = TEMP_VALUE_FOR_ACU - 274 ;

System_printf("OpenWeather Temperature: %d\nOpenWeather Pressure: %s\nOpenWeather Humidity: %s\n ", TEMP_VALUE_FOR_ACU, press_str, Humidity_str);
System_flush();
    HTTPCli_disconnect(&cli); // disconnect from openweathermap

       Task_sleep(1000);
       bool moreFlag = false;
         char data[100], *s1, *s2;
         int ret, temp_received = 0, len;
         struct sockaddr_in addr;

         HTTPCli_Struct cli;
         HTTPCli_Field fields[3] = {
             {HTTPStd_FIELD_NAME_HOST, HOSTNAME2},
             {HTTPStd_FIELD_NAME_USER_AGENT, USER_AGENT},
             {NULL, NULL}};


    System_printf("Sending a HTTP GET request to '%s'\n", HOSTNAME2);
       System_flush();

       HTTPCli_construct(&cli);

       HTTPCli_setRequestFields(&cli, fields);
       System_flush();

       ret = HTTPCli_initSockAddr((struct sockaddr *)&addr, HOSTNAME2, 0);
       System_flush();
       if (ret < 0)
       {
         printError("httpTask: address resolution failed", ret);
       }

       ret = HTTPCli_connect(&cli, (struct sockaddr *)&addr, 0, NULL);
       System_flush();
       if (ret < 0)
       {
         printError("httpTask: connect failed", ret);
       }

       ret = HTTPCli_sendRequest(&cli, HTTPStd_GET, REQUEST_URI2, false);
       if (ret < 0)
       {
         printError("httpTask: send failed", ret);
       }

       ret = HTTPCli_getResponseStatus(&cli);
       if (ret != HTTPStd_OK)
       {
         printError("httpTask: cannot get status", ret);
       }

       System_printf("HTTP Response Status Code: %d\n", ret);

       ret = HTTPCli_getResponseField(&cli, data, sizeof(data), &moreFlag);
       if (ret != HTTPCli_FIELD_ID_END)
       {
         printError("httpTask: response field processing failed", ret);
       }

       len = 0;
       do
       {
         ret = HTTPCli_readResponseBody(&cli, data, sizeof(data), &moreFlag);
         if (ret < 0)
         {
           printError("httpTask: response body processing failed", ret);
         }
         else
         {
           // string is read correctly
           // find "temperature:" string
           //
           s1 = strstr(data, "temperature");
           if (s1)
           {
             if (temp_received)
               continue; // temperature is retrieved before, continue
             // is s1 is not null i.e. "temp" string is found
             // search for comma
             s2 = strstr(s1, ",");
             if (s2)
             {
               *s2 = 0;                    // put end of string
               strcpy(tempstr_2, s1 + 13); // copy the string
               temp_received = 1;
             }
           }
           s1 = strstr(data, "pressure");


           if (s1)
           {

             // is s1 is not null i.e. "temp" string is found
             // search for comma
             s2 = strstr(s1, ",");
             if (s2)
             {
               *s2 = 0;                      // put end of string
               strcpy(press_str_2, s1 + 10); // copy the string

               temp_received = 1;
             }
           }
           s1 = strstr(data, "humidity");
           if (s1)
           {

             // is s1 is not null i.e. "temp" string is found
             // search for comma
             s2 = strstr(s1, ",");
             if (s2)
             {
               *s2 = 0;                         // put end of string
               strcpy(Humidity_str_2, s1 + 10 ); // copy the string

               temp_received = 1;
             }
           }
         }


         len += ret; // update the total string length received so far
       } while (moreFlag);

       System_printf("Recieved %d bytes of payload\n", len);
       System_printf("WeatherStack Temperature: %s\nWeatherStcak Pressure: %s\n ", tempstr_2, press_str_2);
       System_flush(); // write logs to console

       HTTPCli_disconnect(&cli); // disconnect from openweathermap


           Semaphore_post(semaphore3); // activate socketTask
    Event_post(event0 , Event_Id_03);
       Task_sleep(5000); // sleep 5 seconds


  }
  HTTPCli_destruct(&cli);
}

/*
 *  ======== netIPAddrHook ========
 *  This function is called when IP Addr is added/deleted
 */
void netIPAddrHook(unsigned int IPAddr, unsigned int IfIdx, unsigned int fAdd)
{

  static Task_Handle taskHandle1, taskHandle2 ;
  Task_Params taskParams;
  Error_Block eb;

  // Create a HTTP task when the IP address is added
  if (fAdd && !taskHandle1 && !taskHandle2  )
  {
    Error_init(&eb);

    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.priority = 1;
    taskHandle1 = Task_create((Task_FuncPtr)httpTask, &taskParams, &eb);

    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.priority = 1;
    taskHandle2 = Task_create((Task_FuncPtr)socketTask, &taskParams, &eb);


    if (taskHandle1 == NULL || taskHandle2 == NULL)
    {
      printError("netIPAddrHook: Failed to create HTTP and Socket Tasks\n", -1);
    }
  }
}

/*
 *  ======== main ========
 */

int main(void)
{
  /* Call board init functions */

  Board_initGeneral();
  Board_initGPIO();
  Board_initEMAC();
  Board_initI2C();
  /* Turn on user LED */
  GPIO_write(Board_LED0, Board_LED_ON);

  System_printf("Starting the HTTP GET example\nSystem provider is set to "
                "SysMin. Halt the target to view any SysMin contents in ROV.\n");
  /* SysMin will only print to the console when you call flush or exit */
  System_flush();

  /* Start BIOS */
  BIOS_start();

  return (0);
}
