/*********************************************************************************************************
*this node subscribes to (joy) topic from the TeleopeTurtle node (one of ROS example packages)          *
*then publish it as the standerd Twist message on topic (/cmd_vel),                                     * 
********************************************************************************************************/
#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <sensor_msgs/Joy.h>
#include <stdio.h>
#include <stdint.h>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <std_msgs/Bool.h>
#include <std_msgs/Int16.h>
#include <stdbool.h>
#include <unistd.h>
#include "modbus.cpp"


/* Creating class to publish and subscribe the topics at a time */
class RemoteTeleop {

    private:
   /* Creating publisher and subscriber variables */
    ros::Publisher pub;
    ros::Subscriber joystick_subscriber;
    ros::Subscriber button1_subscriber;
    ros::Subscriber button2_subscriber;
     /* Flag variable of light and debug purpose of pwm variable */
      int front_light_flag=0;
      int front_fog_light_flag=0;
      float l=0;
      float r=0;
      uint16_t lPwm = 0;
      uint16_t rPwm = 0;
    public:
  
        
     
      RemoteTeleop(ros::NodeHandle *nh) {
         /* Publish topics publishing to Arduino Controller */
        pub = nh->advertise<geometry_msgs::Twist>("/linear/angular", 100);  // linear and angular value publish topic
		/* Subscriber topics subscribing the data from ROS Mobile App  */
        joystick_subscriber = nh->subscribe("/cmd_vel", 2, &RemoteTeleop::callback_joy, this);// Linear and angular value subscribing
        button1_subscriber = nh->subscribe("btn1topic", 2, &RemoteTeleop::callback_button1, this);// front top light data subscribing
        button2_subscriber = nh->subscribe("btn2topic", 2, &RemoteTeleop::callback_button2, this);// front fog light data subscribing

  float mapPwm(float x, float out_min, float out_max);// Mapping pwm function prototype
     
           
    }
 // Map x value from [0 .. 1] to [out_min .. out_max]
   float mapPwm(float x, float out_min, float out_max)
    {
       return x * (out_max - out_min) + out_min;
    }
 // callback function of Joystick ROS MOBILE App and publishing to arduino rosserial node	
    void callback_joy(const geometry_msgs::Twist& msg) {    
    
  ROS_INFO("Linear:%lf , Angular:%lf\n",msg.linear.x,msg.angular.z);// printing linear/angular information

  if(msg.linear.x>0 || msg.linear.x<0)
  {
     pub.publish(msg);// Publish linear and angular
	 /* Below codes ard debugging purpose */
     l = (msg.linear.x - msg.angular.z) / 2;
         r = (msg.linear.x + msg.angular.z) / 2;
   lPwm = mapPwm(fabs(l), 80,120);
         rPwm = mapPwm(fabs(r), 80,120);
  ROS_INFO("left PWM: %d \n",lPwm);
  ROS_INFO("right PWM: %d \n",rPwm);
  if(l<0)
    ROS_INFO("left reverse\n");
  else
    ROS_INFO("left forward\n");
  if(r<0)
    ROS_INFO("right reverse\n");
  else
    ROS_INFO("right forward\n");  
/* Both linear and angular is less than zero enable reverse light */
   if(l<0&&r<0){
/*Enable Realy no. 5 for Rear light(Reverse light)*/
      modbus(1,MOD8I8O_W_R_OUTPUT_BIT5,PIN_SET,1);
   }
   else{
/*Disable Realy no. 5 for Rear light(Reverse light)*/
      modbus(1,MOD8I8O_W_R_OUTPUT_BIT5,PIN_CLR,1);
   }
  }
  // Both value comes zero else part will execute and publish zero data to arduino
  else  {
     modbus(1,MOD8I8O_W_R_OUTPUT_BIT5,PIN_CLR,1);
     pub.publish(msg);// Publish linear and angular as zero to arduino
     sleep(0.5);
     modbus(1,MOD8I8O_W_R_OUTPUT_BIT7,PIN_SET,1);
     modbus(1,MOD8I8O_W_R_OUTPUT_BIT8,PIN_SET,1);
     sleep(1);
     modbus(1,MOD8I8O_W_R_OUTPUT_BIT7,PIN_CLR,1);
     modbus(1,MOD8I8O_W_R_OUTPUT_BIT8,PIN_CLR,1);
     ROS_INFO("STOP");
  }
         
    }
/* Callback function of front top light and ROS MOBILE App and publishing to arduino rosserial node */
   void callback_button1(const std_msgs::Bool& msg) 
   {
          
     bool light_1=false;
     std_msgs::Bool value; 
      light_1= msg.data; // ROS Mobile data stored in boolean variable
     if(light_1==true)
     {
       front_light_flag++;// light variable true means count the front light flag
     } 
   /* light variable true at the same time front light flag is one 
     publish light on command send to arduino */
     if(front_light_flag==1 && light_1==true) 
     {
      light_1=true;
      value.data=true;
    modbus(1,MOD8I8O_W_R_OUTPUT_BIT1,PIN_SET,1);// Modbus relay 1 Enable

      ROS_INFO("Front top Light :%d",light_1);
     }
      /* light variable true at the same time front light flag is two 
     publish light off command send to arduino */
     if(front_light_flag==2 && light_1==true)
     {
       
       light_1=false;
        value.data=false;
      modbus(1,MOD8I8O_W_R_OUTPUT_BIT1,PIN_CLR,1);// Modbus relay 1 Disable
      ROS_INFO("Front top Light:%d",light_1);
      front_light_flag=0;
     }
 
   }
   /* Callback function of front fog/white light and ROS MOBILE App and publishing to arduino rosserial node */
   void callback_button2(const std_msgs::Bool& msg) 
   {
        	
     bool light_2=false;
     std_msgs::Int16 value;
      light_2= msg.data;// ROS Mobile data stored in Integer variable
     if(light_2==true)
     {
       front_fog_light_flag++; // light variable true means count the front light flag
     } 
   /* fog light variable true at the same time front light flag is one 
     publish white light on command send to arduino */
     if(front_fog_light_flag==1 && light_2==true)
     {
      light_2=true;
      value.data=1;
    modbus(1,MOD8I8O_W_R_OUTPUT_BIT2,PIN_SET,1);// Modbus relay 2 Enable
    modbus(1,MOD8I8O_W_R_OUTPUT_BIT3,PIN_CLR,1);// Modbus relay 3 Disable
      ROS_INFO("White color :%d",light_2);
     }
     /* fog light variable true at the same time front light flag is one 
     publish fog light on command send to arduino */
     if(front_fog_light_flag==2 && light_2==true)
     {
       
       light_2=true;
        value.data=2;
    modbus(1,MOD8I8O_W_R_OUTPUT_BIT2,PIN_CLR,1);// Modbus relay 2 Disable
    modbus(1,MOD8I8O_W_R_OUTPUT_BIT3,PIN_SET,1);// Modbus relay 3 Enable
      ROS_INFO("Yellow color :%d",light_2);
     
     }
    /* fog light variable true at the same time front light flag is one 
     publish white/fog light off command send to arduino */
    if(front_fog_light_flag==3 && light_2==true)
     {
       
       light_2=false;
        value.data=0;
    modbus(1,MOD8I8O_W_R_OUTPUT_BIT2,PIN_CLR,1);// Modbus relay 2 Disable
    modbus(1,MOD8I8O_W_R_OUTPUT_BIT3,PIN_CLR,1);// Modbus relay 3 Disable
      ROS_INFO("Lights off:%d",light_2);
       front_fog_light_flag=0;
     }
   }  
};

int main (int argc, char **argv)
{ 
    ros::init(argc, argv, "teleop");// initialize teleop node
    ros::NodeHandle nh;//A NodeHandle is an object which represents your ROS node
    RemoteTeleop nc = RemoteTeleop(&nh); // creating class object
    ros::spin();//will not return until the node has been shutdown, either through a call to ros::shutdown()
}

