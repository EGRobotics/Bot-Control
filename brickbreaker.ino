#include "U8glib.h"


U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST); // initialize u8g library for our display


  long startTime = 0; // millis() must be greater than this to run program (hard pause)
  long pauseAfter = 0; // program will pause for 1000ms when millis() reaches this number
  


  float x = 128 / 2; // ball X coord (center default)
  float y = 64 / 2; // ball Y coord (center default)
  int dir[] = {-1, 1}; // array to hold ball directions
  float dx = 0; // ball horizontal direction
  float dy = 1; // ball vertical direction
  int br = 4; // ball radius
  
  float vprime = 2; // default velocity
  float v = vprime; // active velocity (initialized to default)
  float dv = 0.1; // delta velocity (rate of ball accelleration after paddle hit)
  float mv = 10; // max velocity
  float ytweak = 0; 

  float psv = 2; // paddle size variance (subtracted from paddle size on ball hit)
  int psm = 3; // paddle size minimum (paddle wont get smaller than this)
  
  float p1s = 20; // paddle 1 size (size of player 1's paddle) initialized to default
  float p1y = 15; // paddle 1 Y coord (vertical position of player 1's paddle)


  float brick1s = 20; // brick 1 size initialized to default
  float brick1y = 2; // brick 2 Y coord 
  float brick2s = 20; // brick 1 size initialized to default
  float brick2y = 22; // brick 2 Y coord
  float brick3s = 20; // brick 1 size initialized to default
  float brick3y = 44; // brick 2 Y coord

  int p1score = 3; // player 1's score
  float p1time = 0; // player 2's time

  int DPAD_PUSH = 7, DPAD_DOWN = 8, DPAD_RIGHT = 9, DPAD_UP = 10, DPAD_LEFT = 11, A = 12, B = 13, POT_HIGH = 2, POT_LOW = 3, POT = 3, potRead = 0;
  bool pushIsPressed = false, downIsPressed = false, rightIsPressed = false, upIsPressed = false, leftIsPressed = false, aIsPressed = false, bIsPressed = false;

void setup() {
  pinMode(POT_HIGH, OUTPUT);
  pinMode(POT_LOW, OUTPUT);
  pinMode(DPAD_PUSH, INPUT_PULLUP);
  pinMode(DPAD_DOWN, INPUT_PULLUP);
  pinMode(DPAD_RIGHT, INPUT_PULLUP);
  pinMode(DPAD_UP, INPUT_PULLUP);
  pinMode(DPAD_LEFT, INPUT_PULLUP);
  pinMode(A, INPUT_PULLUP);
  pinMode(B, INPUT_PULLUP);
  Serial.begin(9600);
  // randomSeed is neccessary to get varying random numbers each time program starts
  int rand = analogRead(0);
  randomSeed(rand);
  // get ball horizontal direction from direction array
  dx = dir[random(2)];
  // enable pins for indicator LEDs
  //pinMode(12, OUTPUT);
  //pinMode(11, OUTPUT);
}


void hook(int buttonPin, bool& buttonState, String buttonName){
  if(digitalRead(buttonPin) == LOW && !buttonState){
    buttonState = true;
    Serial.println(buttonName + " pressed.");
  }
  if(digitalRead(buttonPin) == HIGH && buttonState){
    buttonState = false;
    Serial.println(buttonName + " released.");
  }
}


void buttonHooks(){
  hook(DPAD_PUSH, pushIsPressed, "DPAD");
  hook(DPAD_DOWN, downIsPressed, "DPAD DOWN");
  hook(DPAD_RIGHT, rightIsPressed, "DPAD RIGHT");
  hook(DPAD_UP, upIsPressed, "DPAD UP");
  hook(DPAD_LEFT, leftIsPressed, "DPAD LEFT");
  hook(A, aIsPressed, "A");
  hook(B, bIsPressed, "B");
}


void potHook(){
  int reading = analogRead(POT);
  if(reading - potRead > 5 || reading - potRead < -5){
    potRead = reading;
    Serial.println("Pot set to " + String(reading) + ".");
  }
  digitalWrite(POT_HIGH, HIGH);
  digitalWrite(POT_LOW, LOW);
}


void draw(void) {
  u8g.drawHLine(0, 0, 128);
  u8g.drawHLine(0, 63, 128);
  // set score keeping font
  u8g.setFont(u8g_font_5x7);
  // move text input position
  u8g.setPrintPos(10, 9);
  // print player 1's score
  u8g.print(p1score);
  // move text input position
  u8g.setPrintPos(80, 9);
  // print player time
  u8g.print(p1time);
  // draw paddle for player 1
  u8g.drawBox(0, round(p1y), 2, round(p1s));
  // draw brick 1
  if(brick1s >=1) {
    u8g.drawBox(128 - 2, round(brick1y), 2, round(brick1s));
  }
// draw brick 2
  if(brick2s >=1){
  u8g.drawBox(128 - 2, round(brick2y), 2, round(brick2s));
  }
// draw brick 3
  if(brick3s >= 1){
    u8g.drawBox(128 - 2, round(brick3y), 2, round(brick3s));
  }
  // draw ball
  u8g.drawDisc(round(x), round(y), br);
}


void drawwin(void) {
  p1s = 60; 
  // move text input position
  u8g.setFont(u8g_font_osb18);
  int temp = (128-u8g.getStrWidth("You Win"))/2;  
  u8g.setPrintPos(temp, 50);
  // print player time
  u8g.print("You Win!");
  u8g.setFont(u8g_font_5x7);
  u8g.setPrintPos(x,y); 
  u8g.print("Press B to Continue.."); 
}

void drawlose(void) {
  // move text input position
  p1s = 60; 
  u8g.setFont(u8g_font_osb18);
  int temp = (128-u8g.getStrWidth("Try Again"))/2;  
  u8g.setPrintPos(temp, 50);
  // print player time
  u8g.print("Try Again");
  u8g.setFont(u8g_font_5x7);
  u8g.setPrintPos(x,y); 
  u8g.print("Press B to Continue.."); 
}


void reset(int direct){ // reset ball with direction toward point scorer
  ytweak = random(1, 10); // randomly tweak ball vertical speed and direction to adjust ball flight angle
  dy = (ytweak / 11) * dir[random(2)];
  v = vprime; // reset ball velocity
  dx = direct; // direct ball horizontally toward point scorer
  x = 128 / 2; // center ball horizontally
  y = 64 / 2; // center ball vertically
  startTime = millis() + 500; // pause for 500ms (before ball reset is drawn on screen)
  pauseAfter = millis() + 520; // pause for 1000ms after 520ms (after ball reset is drawn on screen)
}

void resetgame(void){
   p1s = 20; // paddle 1 size (size of player 1's paddle) initialized to default
   brick1s = 20; // brick 1 size initialized to default
   brick2s = 20; // brick 1 size initialized to default
   brick3s = 20; // brick 1 size initialized to default
   brick1y = 2; // brick 2 Y coord 
   brick2y = 22; // brick 2 Y coord
   brick3y = 44; // brick 2 Y coord

   
   
   p1score = 3; // player 1's score
   p1time = 0; // player 2's time
}






void loop(void) {
  buttonHooks();
  potHook();
  // if the time has come to start
  if(millis() > startTime){
    // if players' paddles are too small, set them to the paddle size minimum
    if(p1s < psm){
      p1s = psm;
    }
    
    //p1s =  map(analogRead(4), 0, 1023, 10, 50); // get paddle size for player 1 from pot on A4
    //p2s =  map(analogRead(5), 0, 1023, 10, 50); // get paddle size for player 2 from pot on A5
    //dv = map(analogRead(3), 0, 1023, 1, 10); // get acceleration value from pot on A3
    //dv = dv / 10; // convert acceleration to appropriate float value
    p1y = map(potRead, 0, 1023, 0, 64 - round(p1s)); // get player 1 paddle vertical position from pot on A5
   
    // u8g picture loop
    u8g.firstPage();
    do {
      if ((brick1s + brick2s + brick3s) <= 0){
drawwin();
}else if ( p1score < 0){
drawlose ();
}else {
draw();}

    } while( u8g.nextPage() );
  
    // startTime = millis() + (1000/60); // delay next draw to reduce framerate
    x+=dx*v; // adjust ball horizontal position by direction X * velocity
    y+=dy*v; // adjust ball vertical position by direction Y * velocity
    if(x <= 2 + br){ // ball is less than (ball radius + paddle width) from left edge
      if(y <= p1y + round(p1s) + br && y >= p1y - br){ // ball is colliding with paddle
        dx = 1; // switch ball direction
         ytweak = random(1, 10); // randomly tweak ball vertical speed and direction to adjust ball flight angle
         dy = (ytweak / 11) * dir[random(2)];
        v += dv; // accelerate velocity
        Serial.println("Player 1 Hit!");
      } else { // ball misses paddle
        Serial.println("Player 1 Miss!");
         p1score -= 1; // player 1 loses a chance
         p1s -= psv; // decrease paddle size
        //digitalWrite(12, HIGH); // indicate with LED's
        //digitalWrite(11, LOW);
        reset(1); // reset ball position to center
      }
    }
    if(x >= (128 - br) - 2){ // if ball is closer to right edge than (paddle width + ball radius)
      
  if(brick1s >=1 && y <= brick1y + round(brick1s) + br && y >= brick1y - br){ // ball is colliding with brick1
        dx = -1; // switch horizontal ball direction
        v += dv; // accelerate velocity
        brick1s -= psv; // shrink paddle
        brick1y += psv/2; 
        Serial.println("Player Hit Brick 1!");
      } else if(brick2s >= 1 && y <= brick2y + round(brick2s) + br && y >= brick2y - br){ // ball is colliding with brick 2
        dx = -1; // switch horizontal ball direction
        v += dv; // accelerate velocity
        brick2s -= psv; // shrink paddle
        brick2y += psv/2; 
        Serial.println("Player Hit Brick 2!");
} else if(brick3s >= 1 && y <= brick3y + round(brick3s) + br && y >= brick3y - br){ // ball is colliding with brick 3
        dx = -1; // switch horizontal ball direction
        v += dv; // accelerate velocity
        brick3s -= psv; // shrink paddle
        brick3y += psv/2; 
        Serial.println("Player Hit Brick 3!");
}else { // ball misses
dx = -1; // switch horizontal ball direction
      }
    }

p1time += .1;

if(bIsPressed){
if ((brick1s + brick2s + brick3s) <= 0 || p1score < 0){
resetgame(); 
reset(1); 
}
    
    }

    if(y <= br || y >= 64 - br){ // if center of ball is less than (ball radius) from top or bottom edge of screen
      dy *= -1; // switch vertical direction
    }
    if(v >= mv){ v = mv; } // if ball is going too fast, set velocity to max velocity
    if(pauseAfter > 0 && millis() > pauseAfter){ // if pauseAfter is set, and we have reached the time to pause after
      startTime = millis() + 1000; // delay program for 1000ms
      pauseAfter = 0; // reset pauseAfter
    }
  }
}
