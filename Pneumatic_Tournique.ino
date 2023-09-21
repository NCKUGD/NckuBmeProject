  #include <SoftwareSerial.h>
  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>
  #define addbuttonPin 7 
  #define subbuttonPin 8 
  #define etrbuttonPin 9 
  #define sensorPin A5
  #define pumpPin 34
  #define svalvePin 26
  
  LiquidCrystal_I2C lcd(0x27, 16, 2); 
  
  int Systolic_pressure;
  int Diastolic_pressure;
  int BPM = 0;
  int SetTime = 0;
  float sensor_value =0;
  float volt, kpa, p_mmHg;
  int int_p_mmHg = 0;
  int ResidualTime;
  int OptimizedPressure = 0;
  
  void lcdDisplay_pressure(int pressure){
    if(pressure>=0 && pressure<10){ //個位
    lcd.setCursor(13,0); lcd.print(" "); 
    lcd.setCursor(14,0); lcd.print(" ");
    lcd.setCursor(15,0); lcd.print(pressure);      
    delay(200);
    }
    else if(pressure>=10 && pressure<100){ //十位
    lcd.setCursor(13,0); lcd.print(" "); 
    lcd.setCursor(14,0); lcd.print(pressure);
    delay(200);
    }
    else{ //百位
    lcd.setCursor(13,0); lcd.print(pressure); 
    delay(200);
    }
  }
  
  void lcdDisplay_time(int sec){
    if(sec>=0 && sec<10){ 
    lcd.setCursor(9,1); lcd.print(" ");
    lcd.setCursor(10,1); lcd.print(" "); 
    lcd.setCursor(11,1); lcd.print(" ");
    lcd.setCursor(12,1); lcd.print(sec);      
    delay(200);
    }
    else if(sec>=10 && sec<100){ 
    lcd.setCursor(9,1); lcd.print(" ");
    lcd.setCursor(10,1); lcd.print(" "); 
    lcd.setCursor(11,1); lcd.print(sec);
    delay(200);
    }
    else if(sec>=100 && sec<1000){ 
    lcd.setCursor(9,1); lcd.print(" ");
    lcd.setCursor(10,1); lcd.print(sec); 
    delay(200);
    }
    else{ 
    lcd.setCursor(9,1); lcd.print(sec);
    }
  }
  
  void(*resetFunc)(void) = 0;
  
  void sensor_value_detect(){
     sensor_value = analogRead(sensorPin);
     volt = (sensor_value * 5.03/ 1023.0);
     kpa = (volt - 0.5) * 10;
     p_mmHg = kpa * 7.5;
     int_p_mmHg = (int) p_mmHg;
  }
  
  
  void setup() {
    pinMode(sensorPin, INPUT);
    pinMode(pumpPin, OUTPUT);
    pinMode(svalvePin, OUTPUT);
    pinMode(addbuttonPin, INPUT);
    pinMode(subbuttonPin, INPUT);
    pinMode(etrbuttonPin, INPUT);
    Serial.begin(115200);
    Serial2.begin(115200);
    lcd.init();
    lcd.backlight();
    lcd.print("Hello, sir!");
    delay(1000);
    lcd.clear();
  }
  
  
  void loop(){
  	lcd.setCursor(0, 0);
    lcd.print("Enter Pressure!");
    delay(2000);
    lcd.clear();
    
    while(1){ //讓程式一直執行這個while迴圈不被打破
    	lcd.setCursor(0,0);lcd.print("Data Loading...");
    	
    	if(Serial2.available()==3){
     		lcd.clear();
       
        	Systolic_pressure = Serial2.read();
       		lcd.setCursor(1, 0); lcd.print(Systolic_pressure);
       
       		Diastolic_pressure = Serial2.read();
       		lcd.setCursor(5,0); lcd.print(Diastolic_pressure);
  
       		BPM = Serial2.read();
       		lcd.setCursor(8,0); lcd.print("BPM:");
       		lcd.setCursor(13,0); lcd.print(BPM);
       
       		if(BPM!=0){
       			OptimizedPressure = Systolic_pressure *0.3917 + 2.0686;   //回歸模型otp3
        		lcd.setCursor(0,1); lcd.print("PressureSet:");
        		lcd.setCursor(12,1); lcd.print(OptimizedPressure);
        		delay(5000);
        		lcd.clear();
       		}
     	}
     	
     	SetTime = 0;
     
    	while(OptimizedPressure != 0){ //收到藍芽傳值並計算最佳止血壓力後
       		while(digitalRead(etrbuttonPin) == 1){
         		lcd.setCursor(0, 0); lcd.print("SetTime:");
         		lcd.setCursor(8, 0); lcd.print(SetTime);
         		lcd.setCursor(10, 0); lcd.print("Min");
         	
         		if(digitalRead(addbuttonPin)==0){
           			SetTime += 1; 
           			lcd.setCursor(8,0); lcd.print(SetTime);
           			delay(200);
         		}
         		if(digitalRead(subbuttonPin)==0){
           			SetTime -= 1;
           		
           			if(SetTime < 0){
             			SetTime = 0;
           			}
           		
            	lcd.setCursor(8,0); lcd.print(SetTime);
           		delay(200);
         		}
         	
         	ResidualTime = SetTime*60;
      		}
      
            sensor_value_detect();
		
		    if(SetTime > 0){
                lcd.clear();
            }
            while((SetTime != 0) && (int_p_mmHg < OptimizedPressure) && digitalRead(etrbuttonPin) == 0){
                if((digitalRead(addbuttonPin) == 0)||(digitalRead(subbuttonPin) == 0)){
                    digitalWrite(pumpPin, 0);
                    digitalWrite(svalvePin, 1);
                    delay(1000);
                    digitalWrite(svalvePin, 0);
                    resetFunc();
                }
                
                sensor_value_detect();
                digitalWrite(pumpPin, 1);
                lcd.setCursor(0,0); lcd.print("Goal:");
                lcd.setCursor(5,0); lcd.print(OptimizedPressure);
                lcd.setCursor(9,0); lcd.print("now:");
                lcd.setCursor(14,0); lcd.print(int_p_mmHg);
                delay(30);
            }
            
            if((SetTime > 0) && (int_p_mmHg >= OptimizedPressure)){
                digitalWrite(pumpPin,0);
            }
            
            while((SetTime != 0) && digitalRead(etrbuttonPin)==0){//按下啟動鈕
                if((digitalRead(addbuttonPin)==0) ||(digitalRead(subbuttonPin)==0)){
                    digitalWrite(pumpPin, 0);
                    digitalWrite(svalvePin, 1);
                    delay(1000);
                    digitalWrite(svalvePin, 0);
                    resetFunc();
                }
                
                while((int_p_mmHg < OptimizedPressure)){
                    if((digitalRead(addbuttonPin)==0) ||(digitalRead(subbuttonPin)==0)){
                        digitalWrite(pumpPin, 0);
                        digitalWrite(svalvePin, 1);
                        delay(1000);
                       digitalWrite(svalvePin, 0);
                        resetFunc();
                    }
                
                sensor_value_detect();
                digitalWrite(pumpPin, 1);
                lcd.setCursor(0,0); lcd.print("Goal:");
                lcd.setCursor(5,0); lcd.print(OptimizedPressure);
                lcd.setCursor(9,0); lcd.print("now:");
                lcd.setCursor(14,0); lcd.print(int_p_mmHg);
                delay(30);
                }
            
                if((int_p_mmHg >= OptimizedPressure)){
                    digitalWrite(pumpPin,0);
                }
             
                sensor_value_detect();
                ResidualTime -= 1;
                lcd.setCursor(9,0); lcd.print("now:");
                lcdDisplay_pressure(int_p_mmHg);
                lcd.setCursor(0,1); lcd.print("Remaining");
                lcdDisplay_time(ResidualTime);
                lcd.setCursor(13,1); lcd.print("sec");
                
                if(ResidualTime == (SetTime*60-30)){ // After 3 mins -20mmHg
                    if(int_p_mmHg > OptimizedPressure-20){ //若當前壓力大於目標壓力則執行洩氣
                        OptimizedPressure =  OptimizedPressure-20;
                        digitalWrite(svalvePin, 1);
                        delay(500);
                        digitalWrite(svalvePin, 0);
                        sensor_value_detect();
                        
                        while(int_p_mmHg < OptimizedPressure){//洩氣後重新加壓到目標壓力
                            if((digitalRead(addbuttonPin)==0) ||(digitalRead(subbuttonPin)==0)){
                                digitalWrite(pumpPin, 0);
                                digitalWrite(svalvePin, 1);
                                delay(1000);
                                digitalWrite(svalvePin, 0);
                                resetFunc();
                            }
                            
                            sensor_value_detect();
                            digitalWrite(pumpPin,1);
                            lcd.setCursor(0,0); lcd.print("Goal:");
                            lcd.setCursor(5,0); lcd.print(OptimizedPressure);
                            lcd.setCursor(7,0); lcd.print(" ");
                            lcd.setCursor(9,0); lcd.print("now:");
                            lcd.setCursor(14,0); lcd.print(int_p_mmHg);
                            delay(50);
                        }
                        
                        if(int_p_mmHg >= OptimizedPressure-20){//加壓到目標壓力後停止加壓
                            digitalWrite(pumpPin,0);
                        }
                    }
                    
                    else if(int_p_mmHg < OptimizedPressure-20){//若當前壓力小於目標壓力則加壓
                        OptimizedPressure =  OptimizedPressure-20;
                        
                        while(int_p_mmHg < OptimizedPressure){//執行加壓
                            sensor_value_detect();
                            
                            if((digitalRead(addbuttonPin)==0) ||(digitalRead(subbuttonPin)==0)){
                                digitalWrite(pumpPin, 0);
                                digitalWrite(svalvePin, 1);
                                delay(1000);
                                digitalWrite(svalvePin, 0);
                                resetFunc();
                            }
                            
                        digitalWrite(pumpPin,1);
                        lcd.setCursor(0,0); lcd.print("Goal:");
                        lcd.setCursor(5,0); lcd.print(OptimizedPressure);
                        lcd.setCursor(7,0); lcd.print(" ");
                        lcd.setCursor(9,0); lcd.print("now:");
                        lcd.setCursor(14,0); lcd.print(int_p_mmHg); 
                        delay(50);
                        }
                        
                        if(int_p_mmHg >= OptimizedPressure-20){//加壓到目標壓力後停止加壓
                             digitalWrite(pumpPin,0);
                            lcdDisplay_pressure(int_p_mmHg);
                        }
                    }
               
                break;
                }
                
            delay(1000);
            }
        }
    }
}
  
