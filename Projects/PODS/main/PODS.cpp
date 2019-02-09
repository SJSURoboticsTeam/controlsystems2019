#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <WiFi.h>
#include <math.h>
#include <iostream>
#include <string>
#include "PODS.h"
#include "Arduino.h"
#include "constants.h"
#include "driver/ledc.h"
#include "Servo_Control.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "RTOStasks.h"
#include "esp_intr_alloc.h"

using namespace std;
//portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
////////////////////////////////////////////////////////////////////////////////
//                               BEGIN CODE HERE                              //
////////////////////////////////////////////////////////////////////////////////


void initServer(AsyncWebServer* server, ParamsStruct* params) {
    //Create Access Point
    WiFi.softAP("ROAR", "testpassword");
    Serial.println();
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());


    AsyncEventSource events("/events");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

    /* XHR Example.
        - Param "name" is sent from mission control.
        - "name" is copied to the params object
        - params object is then passed to vSayHelloTask - see main.cpp
        - vSayHello task then accesses name directly.

        Note: for ANY parameters you want to use, you must add them to
        the paramsStruct struct located in Source.h first. 
    */
    server->on("/update_name", HTTP_POST, [=](AsyncWebServerRequest *request){
        strcpy(params->name, request->arg("name").c_str());
        request->send(200, "text/plain", "Success");
   
    });
    
    /*******************************
		start/stop PODS comand
    ********************************/
    server->on("/toggle_pod", HTTP_POST, [=](AsyncWebServerRequest *request){
         printf("XHR recieved \n");
         int x = atoi(request->arg("pod").c_str()); 
         string y = request->arg("state").c_str();

         std::cout << y << "\n";
         std::cout << x << "\n";
         bool z;

         if(y =="True" or y == "true" or "TRUE")
         {
         	 z = true;
         }
         else if(y == "false" or y == "False" or y == "FALSE")
         {
         	z = false;
         }
        startPOD(z,x);
         
         request->send(200, "text/plain", "pod toggled");

    });
       

        server->on("/stop_all", HTTP_POST, [=](AsyncWebServerRequest *request){
         
         for(int i = 0; i <=6; i++)
         {
         	startPOD(false,i);
         }
         
         request->send(200, "text/pain", "All POD suspended");

    });

    /*************************
		return data comand
    ***************************/
    server->on("/data", HTTP_POST, [=](AsyncWebServerRequest *request){
        String type = request->arg("type").c_str();
        int x = atoi(request->arg("id").c_str());
        int z = writeData(false, x, 0);
        char c[15];

        String data;
         itoa(z, c, 10);
         data = c;
        if(type == "cpm")
         	{
        		request->send(200, "text/plain", data);
        	}
    });

    /* SSE Example.
        - SSEs will be used to continuously send data that was
        not necessarily requested by mission control
        (e.g. temperature, something we should send periodically)

        - Once mission control declares the ESPs IP address at a certain 
        endpoint to be an EventSource, the ESP can trigger events on the web
        interface, which the web interface can attach event listeners to
        (similar to how we are attaching event listeners for when we recieve
        XHRs to /update_name above, allowing us to do things when we recieve an 
        XHR).
        - Below's example is an example of sending SSEs when mission control
        declares our ip address and endpoint (e.g. 192.168.4.1/events) to be
        an event source.
        - More info on this concept here: 
            https://developer.mozilla.org/en-US/docs/Web/API/EventSource
    */
    events.onConnect([](AsyncEventSourceClient *client) {
      if(client->lastId())
      {
        Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
      }
      // send event with message "hello!", id current millis
      // and set reconnect delay to 1 second
      client->send("hello!", NULL, millis(), 1000);
      delay(1000);
      client->send("hello!", NULL, millis(), 1000);
      delay(1000);
      client->send("hello!", NULL, millis(), 1000);
      delay(1000);
      client->send("hello!", NULL, millis(), 1000);
      delay(1000);
    });

    //Attach event source to the server.
    server->addHandler(&events);

    //Start server.
    server->begin();
}


void startPOD(bool start, int x)
{
	int state;
		if(start == true)
		{
			state = -1;
		}
		if(start == false)
		{
			state = -2;
		}
	
		printf("startPOD function\n x = %d \n", x);
		printf("state = %d \n", state );
		

		switch(x)
		{
		case 0: xTaskCreate(vGygerTask, "gyger1 data", 4060, (void*)0, 1, NULL); 
				printf("task %d toggled \n", x);
			break;
		case 1: xTaskCreate(vGygerTask, "gyger1 data", 4060, (void*)1, 1, NULL); 
				printf("task %d toggled \n", x);				
			break;
		case 2: xTaskCreate(vGygerTask, "gyger2 data", 4060, (void*)2, 1, NULL); 
				printf("task %d toggled \n", x);
			break;
		case 3: xTaskCreate(vGygerTask, "gyger3 data", 4060, (void*)3, 1, NULL); 
				printf("task %d toggled \n", x);
			break;
		case 4: xTaskCreate(vGygerTask, "gyger4 data", 4060, (void*)4, 1, NULL);
				printf("task %d toggled \n", x);
			break;
		case 5: xTaskCreate(vGygerTask, "gyger5 data", 4060, (void*)5, 1, NULL);
				printf("task %d toggled \n", x);
			break;
		case 6: xTaskCreate(vGygerTask, "gyger6 data", 4060, (void*)6, 1, NULL);
				printf("task %d toggled \n", x);
			break;
		default:
			break;
	}
	

	
}


void sealPODS(int x)
{

uint32_t servo1_frequency = 50; //hz
uint32_t servo1_gpio_pin = servoLidPin(x);
uint32_t servo1_timer = 0;
uint32_t servo1_channel = 0;
float servo1_min = 5;
float servo1_max = 10;

		//servo object for PODS door
	Servo servo1(servo1_gpio_pin,servo1_channel,servo1_timer, 
		servo1_frequency, servo1_max, servo1_min);

	servo1.SetPositionPercent(getPercent(-90));


}

/**************************
	rotates a servo to despens an amount of fluid
****************************/
void dispenseFluid(int x)
{

uint32_t servo1_frequency = 50; //hz
uint32_t servo1_gpio_pin = servoInoculationPin(x);
uint32_t servo1_timer = 0;
uint32_t servo1_channel = 0;
float servo1_min = 5;
float servo1_max = 10;
int up = 90;
int down = -90;


/*
	if(x == 7)
	{
		//servo object for sterilization fluid
	
	Servo servo2(inoculation_servo7_pin,servo1_channel,servo1_timer, 
					servo1_frequency, servo1_max, servo1_min);
	}


	*/
		//servo object for inoculation fluid
	Servo servo1(servo1_gpio_pin,servo1_channel,servo1_timer, 
		servo1_frequency, servo1_max, servo1_min);

	servo1.SetPositionPercent(getPercent(down));

/*
	if(x == 7)
	{
		servo2.SetPositionPercent(getPercent(down));
	}

*/
	vTaskDelay(10000 / portTICK_PERIOD_MS);

/*
	if(x == 7)
	{
		servo2.SetPositionPercent(getPercent(up));
	}
*/
	servo1.SetPositionPercent(getPercent(up));

}


double getPercent(int angle)
{
	//servo 5-10% 
	int percent = map(angle, -90, 90, 0, 100);

	return percent;
}




int servoInoculationPin(int x)
{
			switch(x)
	{
		case 0: return inoculation_servo0_pin;
			break;
		case 1: return inoculation_servo1_pin;
			break;
		case 2: return inoculation_servo2_pin;
			break;
		case 3: return inoculation_servo3_pin;
			break;
		case 4: return inoculation_servo4_pin;
			break;
		case 5: return inoculation_servo5_pin;
			break;
		case 6: return inoculation_servo6_pin;
			break;
		default: return -1;
			break;
	}
}

int servoLidPin(int x)
{
			switch(x)
	{
		case 0: return lid_servo_pin0;
			break;
		case 1: return lid_servo_pin1;
			break;
		case 2: return lid_servo_pin2;
			break;
		case 3: return lid_servo_pin3;
			break;
		case 4: return lid_servo_pin4;
			break;
		case 5: return lid_servo_pin5;
			break;
		case 6: return lid_servo_pin6;
			break;
		default: return -1;
			break;
		}
}

/*********************************
	interupt functions
********************************/




int gygerPin(int id)
{
	switch(id)
	{
		case 0: return gyger0_pin;
			break;
		case 1: return gyger1_pin;
			break;
		case 2: return gyger2_pin;
			break;
		case 3: return gyger3_pin;
			break;
		case 4: return gyger4_pin;
			break;
		case 5: return gyger5_pin;
			break;
		case 6: return gyger6_pin;
			break;
		default: 
			break;
	}

	return -1;
}

//type: true = write data  false = request data
int writeData(bool type, int id, int val)
{

	if(type == true)
	{
			switch (id)
		{
		case 0:  cpm0 = val;
			break;
		case 1:  cpm1 = val;
			break;
		case 2:  cpm2 = val;
			break;
		case 3:  cpm3 = val;
			break;
		case 4:  cpm4 = val;
			break;
		case 5:  cpm5 = val;
			break;
		case 6: cpm6 = val;
			break;
		default:  
			break;
		}


	}

	
	if(type == false)
	{
			switch (id)
		{
		case 0: return cpm0;
			break;
		case 1: return cpm1;
			break;
		case 2: return cpm2;
			break;
		case 3: return cpm3;
			break;
		case 4: return cpm4;
			break;
		case 5: return cpm5;
			break;
		case 6: return cpm6;
			break;
		default: 
			break;
		}
	}

	return -1;
}



/****************************
	interupt function
*****************************/
void emissionCount(void* pin_num)
{
	int id;
	switch((int)pin_num)
	{
		case gyger0_pin: 
				id = 0; 
			break;
		case gyger1_pin: 
				id = 1; 
			break;
		case gyger2_pin: 
				id = 2; 
			break;
		case gyger3_pin: 
				id = 3; 
			break;
		case gyger4_pin: 
				id = 4; 
			break;
		case gyger5_pin: 
				id = 5;
		 	break;
		case gyger6_pin: 
				id = 6;
			 break;
		default: id = -1; break;
	}
	
		//eCount0++;
	xQueueSendFromISR(xQueueISR, (void*) &id, pdFALSE);
	
	    int yield = 0;
    //portENTER_CRITICAL_ISR(&mux);
   // xSemaphoreGiveFromISR(xGygerSemaphore0, &yield);
    portYIELD_FROM_ISR();
}
/////////////////////////////////////////////////////////////////////////////////                               CODE ENDS HERE                               //
////////////////////////////////////////////////////////////////////////////////
