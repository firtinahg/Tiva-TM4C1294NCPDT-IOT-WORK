# httpget_TIVA4C
IOT Work with TIVA TM4C

In this work you can find out an IOT implementation with TI TivaWare TM4C board.

What this program do?

    First of all this program is suitable for Code Composer Studio platform.It's written in CCS 9.2.0 , please be careful about this matter.
This program aiming to creating a system which requirements listed as below ;

-Program has to be a timer.This timer trigger the semaphore every 15 seconds to executing the next task.
-First task is TaskForTMP006 , this task pending for semaphore0.Semaphore0 sending by TimerISR.
-TaskForTMP006 created for getting temperature value from tmp006 sensor.
-Second task is TaskForBMP180 , this task pending for semaphore1.Semaphore1 sending by TaskForTMP006.
-TaskForBMP180 created for getting pressure value from BMP180 sensor.
-**This sensors available on BOOSTXL SENSHUB.If you have not this part , this program couldn't work.**
-To get the temperature and pressure values from the internet , system use the ethernet connection.So , you have to connect the Tiva tm4c1294ncpdt port with to the Wi-Fi modem LAN port.

To get more information about usage of program please contact me.
fhg971 at gmail dot com



