#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include "pitch.h"

#define TX_PIN 13
#define RX_PIN 12
SoftwareSerial mySerial(RX_PIN, TX_PIN); // khai báo mở cổng serial mềm

#define ledPin A3 // chân đèn
// #define buzzerPin 3        // chân buzzer
// #define tripwirePin A3     // chân tripwire

#define btnOpenPin A2  // chân công tắc báo hiệu mở cửa
#define btnClosePin A1 // chân công tắc báo hiệu đóng cửa
#define servoPin A0    // chân servo

#define buttonManualPin 3 // chân nút mở cửa thủ công
#define buttonSwitchPin 2 // chân nút chuyển chế độ

// lệnh lấy từ/gửi lên esp8266
String get;
String setMode;
String setOpen;

char mode = '0'; // chế độ manual/auto
char open = '0'; // check cửa mở
String regconize;

Servo myservo;
// int tripwireState;
int isDoorOpen = 0;
int pirInput;
int servoPosition = 120;
int degree = 0;
int closeDegree = -3;
int openDegree = 3;
int btnOpenState;
int btnCloseState;
int melody[] = {NOTE_C4, NOTE_C4, NOTE_C4, NOTE_C4, NOTE_C4, NOTE_C4, NOTE_C4, NOTE_C4, NOTE_C4};
int noteDurations[] = {6, 6, 6, 2, 2, 2, 6, 6, 6};

const int ROW_NUM = 4;
const int COLUMN_NUM = 4;
char keys[ROW_NUM][COLUMN_NUM] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte pin_rows[ROW_NUM] = {4, 5, 6, 7};
byte pin_column[COLUMN_NUM] = {8, 9, 10, 11};
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);
LiquidCrystal_I2C lcd(0x27, 16, 2);
int cursorColumn = 0;
String pass = "";
// String mk = "123456";
String mk = "";
int len = 6;

void millisDelay(long int delayTime)
{
    long int start_time = millis();
    while (millis() - start_time < delayTime)
        ;
}

// void bussss()
// {
//     int i = 0;
//     while (true)
//     {
//         int noteDuration = 1000 / noteDurations[i];
//         tone(buzzerPin, melody[i], noteDuration);
//         i += 1;
//         int pauseBetweenNotes = noteDuration * 1.30;
//         millisDelay(pauseBetweenNotes);
//         if (i == 9)
//         {
//             break;
//         }
//     }
// }

void servoSweep()
{
    millisDelay(20);
    servoPosition += degree;
    if (servoPosition > 180)
        servoPosition = 180;
    if (servoPosition < 0)
        servoPosition = 0;
    myservo.write(servoPosition);
}

void closeDoor()
{
    degree = closeDegree;
    while (true)
    {
        btnCloseState = digitalRead(btnClosePin);
        if (btnCloseState == LOW)
        {
            break;
        }
        servoSweep();
    }
    digitalWrite(ledPin, LOW);
}

void openDoor()
{
    degree = openDegree;
    digitalWrite(ledPin, HIGH);
    while (true)
    {
        btnOpenState = digitalRead(btnOpenPin);
        if (btnOpenState == LOW)
        {
            break;
        }
        servoSweep();
    }
}

void holdAutoDoor()
{
    degree = 0;
    millisDelay(1000);
}

void control()
{
    if (isDoorOpen == 0)
    {
        if (mode == '0')
        {
            mode = '1';
            setMode = "MODE_" + String(mode);
            // digitalWrite(ledChangeModePin, LOW);
            mySerial.print(setMode);
            Serial.println(setMode);
        }
        else if (mode == '1')
        {
            mode = '0';
            setMode = "MODE_" + String(mode);
            // digitalWrite(ledChangeModePin, HIGH);
            mySerial.print(setMode);
            Serial.println(setMode);
        }
    }
}

void setup()
{
    Serial.begin(9600);
    lcd.init();
    myservo.attach(servoPin);
    pinMode(ledPin, OUTPUT);
    pinMode(buttonSwitchPin, INPUT_PULLUP);
    attachInterrupt(0, control, RISING);
    pinMode(btnOpenPin, INPUT_PULLUP);
    pinMode(btnClosePin, INPUT_PULLUP);
    // pinMode(tripwirePin, INPUT);
    pinMode(buttonManualPin, INPUT_PULLUP);
    openDoor();
    millisDelay(1000);
    // bussss();
    closeDoor();
    mySerial.begin(9600);
}

void loop()
{
    if (mySerial.available())
    {
        String command;
        get = mySerial.readString();
        get.trim();
        // Serial.println(get);
        if (get.startsWith("PASS_"))
        {
            command = get.substring(5);
            command.trim();
            if (mk != command)
                mk = command;
            Serial.println("PASS_" + String(mk));
        }
        if (get.startsWith("MODE_"))
        {
            command = get.substring(5);
            if (command[0] != (mode))
            {
                mode = command[0];
                Serial.println("MODE_" + String(mode));
            }
        }
        if (get.startsWith("OPEN_") && mode == '1')
        {
            command = get.substring(5);
            // command.trim();
            // Serial.println(command);
            if (command[0] != open)
            {
                open = command[0];
                if (open == '1')
                {
                    isDoorOpen = 1;
                    openDoor();
                    millisDelay(3000);
                    closeDoor();
                    isDoorOpen = 0;
                    open = '0';
                    setOpen = "OPEN_" + String(open);
                    mySerial.print(setOpen);
                    Serial.print(setOpen);
                    millisDelay(500);
                }
            }
        }
        if (get.startsWith("REGCONIZE_") && mode == '0')
        {
            command = get.substring(10);
            if (command != regconize)
            {
                regconize = command;
                Serial.println(regconize);
                if (regconize == "openDoor")
                {
                    isDoorOpen = 1;
                    openDoor();
                    millisDelay(3000);
                    closeDoor();
                    isDoorOpen = 0;
                    setOpen = "OPEN_" + String(open);
                    Serial.print(get);
                    millisDelay(500);
                    regconize = "";
                }
                else if (regconize == "Unknown"){
                    Serial.println(regconize);
                    regconize = "";
                }
            }

        }
    }

    // if (Serial.available())
    // {
    //     String command = Serial.readStringUntil('\n');

    //     if (command.startsWith("SET_MODE:") && isDoorOpen == 0)
    //     {
    //         String newPass = command.substring(9);
    //         newPass.trim();
    //         if (newPass == "AUTO")
    //         {
    //             mode = '0';
    //         }
    //         else if (newPass == "SECURE")
    //         {
    //             mode = '1';
    //         }
    //     }

    //     if (command.startsWith("SET_PASSWORD:"))
    //     {
    //         String newPass = command.substring(13);
    //         newPass.trim();
    //         if (newPass.length() > 0)
    //         {
    //             mk = newPass;
    //         }
    //     }

    //     if (command.startsWith("SET_CSPEED:"))
    //     {
    //         String newSpeed = command.substring(11);
    //         newSpeed.trim();
    //         if (newSpeed.length() > 0)
    //         {
    //             closeDegree = newSpeed.toInt();
    //         }
    //     }

    //     if (command.startsWith("SET_OSPEED:"))
    //     {
    //         String newSpeed = command.substring(11);
    //         newSpeed.trim();
    //         if (newSpeed.length() > 0)
    //         {
    //             openDegree = newSpeed.toInt();
    //         }
    //     }

    //     if (command.startsWith("OPEN_DOOR") && isDoorOpen == 0 && selection == 0)
    //     {
    //         isDoorOpen = 1;
    //         openDoor();
    //         millisDelay(1000);
    //         bussss();
    //         closeDoor();
    //         millisDelay(500);
    //         isDoorOpen = 0;
    //     }
    // }
    // String selection;
    // selection = mode == '0' ? "auto" : "manual";
    // String sendMess = "mode:" + mode + "|" + "pass:" + mk + "|" + "cspeed:" + String(closeDegree) + "|" + "ospeed:" + String(openDegree);
    // Serial.println(sendMess);

    if (mode == '1')
    {
        lcd.backlight();
        // khai báo và gán giá trị nhận được từ keypad cho biến key
        char key = keypad.getKey();
        // nếu tồn tại ký tự key thì hiển thị nó lên màn hình LCD
        if (key)
        {
            delay(100);
            if (key != '*' && key != '#')
            {
                if (cursorColumn <= len)
                {
                    lcd.setCursor(cursorColumn, 0);
                    lcd.print(key);
                    pass += key;
                    // sau khi hiển thị thì tăng số lượng ký tự lên 1
                    cursorColumn++;
                    // nếu số lượng ký tự quá 16 thì xóa đi và reset biến cursorColumn
                }
            }
            if (key == '*')
            {
                if (pass != "")
                {
                    String temp = "";
                    for (int i = 0; i < pass.length() - 1; i++)
                        temp += pass[i];

                    lcd.clear();
                    int i = 0;
                    lcd.setCursor(i, 0);
                    for (i = 0; i < temp.length(); i++)
                    {
                        lcd.print(temp[i]);
                        lcd.setCursor(i + 1, 0);
                    }
                    pass = temp;
                    cursorColumn = pass.length();
                }
            }

            if (key == '#')
            {
                if (pass == mk)
                {
                    Serial.println("Open");
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("mk dung");
                    isDoorOpen = 1;
                    openDoor();
                    millisDelay(3000);
                    // bussss();
                    closeDoor();
                    pass = "";
                    millisDelay(500);
                    isDoorOpen = 0;
                }
                else
                {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("mk sai");
                    pass = "";
                    millisDelay(500);
                }
                lcd.clear();
                cursorColumn = 0;
            }
        }

        int buttonManualState = digitalRead(buttonManualPin);
        if (buttonManualState == LOW && isDoorOpen == 0)
        {
            isDoorOpen = 1;
            openDoor();
            millisDelay(1000);
            // bussss();
            closeDoor();
            millisDelay(500);
            isDoorOpen = 0;
        }
    }
    // else if (mode == '0')
    // {
    //     importSensorState = digitalRead(importSensorPin);
    //     // pirInput = digitalRead(PIRPin);

    //     // if (pirInput == HIGH)
    //     // {
    //     //     openDoor();
    //     //     isDoorOpen = 1;
    //     //     millisDelay(3000);
    //     // }

    //     if (importSensorState == HIGH)
    //     {
    //         holdAutoDoor();
    //         openDoor();
    //         millisDelay(3000);
    //     }

    //     if (importSensorState == LOW && isDoorOpen == 1)
    //     {
    //         closeDoor();
    //         isDoorOpen = 0;
    //     }
    // }
}
