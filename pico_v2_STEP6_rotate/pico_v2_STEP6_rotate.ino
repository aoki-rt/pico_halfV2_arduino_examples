// Copyright 2023 RT Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#define LED0 42
#define LED1 41
#define LED2 15
#define LED3 16

#define SW_L 13
#define SW_R 14

#define MOTOR_EN 12
#define CW_R 21
#define CW_L 11
#define PWM_R 45
#define PWM_L 46

#define MIN_HZ 40
#define TIRE_DIAMETER (24.70)
#define PULSE (TIRE_DIAMETER * PI / (35.0 / 10.0 * 20.0 * 4.0))
#define MIN_SPEED (MIN_HZ * PULSE)
#define TREAD_WIDTH (31.5)

typedef enum {
  front,
  right,
  rear,
  left,
  unknown,
} t_local_dir;

hw_timer_t * g_timer0 = NULL;
hw_timer_t * g_timer2 = NULL;
hw_timer_t * g_timer3 = NULL;

portMUX_TYPE g_timer_mux = portMUX_INITIALIZER_UNLOCKED;

unsigned short g_step_hz_r = MIN_HZ;
unsigned short g_step_hz_l = MIN_HZ;

volatile unsigned int g_step_r, g_step_l;
double g_max_speed;
double g_min_speed;
double g_accel = 0.0;
volatile double g_speed = MIN_SPEED;

volatile bool g_motor_move = 0;

//割り込み
//目標値の更新周期1kHz
void IRAM_ATTR onTimer0(void)
{
  portENTER_CRITICAL_ISR(&g_timer_mux);  //割り込み禁止
  controlInterrupt();
  portEXIT_CRITICAL_ISR(&g_timer_mux);  //割り込み許可
}

//Rモータの周期数割り込み
void IRAM_ATTR isrR(void)
{
  portENTER_CRITICAL_ISR(&g_timer_mux);  //割り込み禁止
  if (g_motor_move) {
    timerAlarmWrite(g_timer2, 2000000 / g_step_hz_r, true);
    digitalWrite(PWM_R, HIGH);
    for (int i = 0; i < 100; i++) {
      asm("nop \n");
    }
    digitalWrite(PWM_R, LOW);
    g_step_r++;
  }
  portEXIT_CRITICAL_ISR(&g_timer_mux);  //割り込み許可
}

//Lモータの周期数割り込み
void IRAM_ATTR isrL(void)
{
  portENTER_CRITICAL_ISR(&g_timer_mux);  //割り込み禁止
  if (g_motor_move) {
    timerAlarmWrite(g_timer3, 2000000 / g_step_hz_l, true);
    digitalWrite(PWM_L, HIGH);
    for (int i = 0; i < 100; i++) {
      asm("nop \n");
    };
    digitalWrite(PWM_L, LOW);
    g_step_l++;
  }
  portEXIT_CRITICAL_ISR(&g_timer_mux);  //割り込み許可
}

void setup()
{
  // put your setup code here, to run once:
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  pinMode(SW_L, INPUT);
  pinMode(SW_R, INPUT);

  //motor disable
  pinMode(MOTOR_EN, OUTPUT);
  pinMode(CW_R, OUTPUT);
  pinMode(CW_L, OUTPUT);
  pinMode(PWM_R, OUTPUT);
  pinMode(PWM_L, OUTPUT);

  digitalWrite(MOTOR_EN, LOW);
  digitalWrite(CW_R, LOW);
  digitalWrite(CW_L, LOW);
  digitalWrite(PWM_R, LOW);
  digitalWrite(PWM_L, LOW);

  g_timer0 = timerBegin(0, 80, true);  //1us
  timerAttachInterrupt(g_timer0, &onTimer0, true);
  timerAlarmWrite(g_timer0, 1000, true);  //1kHz
  timerAlarmEnable(g_timer0);

  g_timer2 = timerBegin(2, 40, true);  //0.5us
  timerAttachInterrupt(g_timer2, &isrR, true);
  timerAlarmWrite(g_timer2, 13333, true);  //150Hz
  timerAlarmEnable(g_timer2);

  g_timer3 = timerBegin(3, 40, true);  //0.5us
  timerAttachInterrupt(g_timer3, &isrL, true);
  timerAlarmWrite(g_timer3, 13333, true);  //150Hz
  timerAlarmEnable(g_timer3);
}

void loop()
{
  // put your main code here, to run repeatedly:
  while (digitalRead(SW_L) & digitalRead(SW_R)) {
    continue;
  }
  digitalWrite(MOTOR_EN, HIGH);
  delay(1000);
  rotate(right, 1);
  delay(1000);
  rotate(left, 1);
  delay(1000);
  rotate(right, 2);
  delay(1000);
  rotate(left, 2);
  delay(1000);
  digitalWrite(MOTOR_EN, LOW);
}