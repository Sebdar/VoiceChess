#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <unistd.h>

int main()
{
    int ArduFD = 0, test = 1; char lecture[5];
    ArduFD = wiringPiI2CSetup(0x08);
    wiringPiI2CWriteReg16(ArduFD, 8, 801);

    while(1)
    {   test = wiringPiI2CReadReg8(ArduFD, 5);
        lseek(ArduFD, -2, SEEK_END);

        read(ArduFD, lecture, 5);
        printf("Lecture | %d |%s\n",test, lecture);
        sleep(1);
    }

    return 0;
}
