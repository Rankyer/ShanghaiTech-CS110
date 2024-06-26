

#include "lcd/lcd.h"
#include "utils.h"
#include "assembly/example.h"

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_OBSTACLES 3
#define OBSTACLE_WIDTH 1
#define OBSTACLE_SPACE 40 
#define SCREEN_RIGHT 120  
#define SCREEN_LEFT 0
#define SCREEN_TOP 40 
#define SCREEN_BOTTOM 160
#define BIRD_START_X 20
int BIRD_START_Y = 90;
#define BIRD_SIZE 2
#define GRAVITY 0.7
#define JUMP_STRENGTH -4
void Inp_init(void)
{
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_GPIOC);

  gpio_init(GPIOA, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ,
            GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
  gpio_init(GPIOC, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ,
            GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
}

void IO_init(void)
{
  Inp_init(); // inport init
  Lcd_Init(); // LCD init
}

// Here is my DataBase
// for select use
int entry = 0; // 1 代表 easy 模式，2代表 normal 模式，3代表 hard 模式

/////////////////////////////////////////////////////////////////////////////////
// int detectButtonPress()
// {
//   while (1)
//   {
//     if (Get_Button(BUTTON_1))
//     {
//       return 1;
//     }
//     else if (Get_Button(BUTTON_2))
//     {
//       return 2;
//     }
//     delay_1ms(10);
//   }
// }
// void displayWelcomeScreen(int buttonCode)
// {
//   LCD_Clear(BLACK);
//   u16 colors1[] = {0x07E0, 0xF81F, 0x07FF};
//   u16 colors2[] = {0xF800, 0x001F, 0xFFE0};
//   int num_colors = sizeof(colors1) / sizeof(colors1[0]);
//   switch (buttonCode)
//   {
//   case 1:
//     for (int i = 0; i < 30; i++)
//     {
//       u16 cl = colors1[i % num_colors];
//       LCD_Fill(0, 0, 80, 160, cl);
//       delay_1ms(20);
//     }
//     LCD_ShowString(10, 60, "Welcome! This is SW1!", 0xFFFF);
//     break;

//   case 2:
//     for (int i = 0; i < 30; i++)
//     {
//       u16 cl = colors2[(50 - i) % num_colors];
//       LCD_Fill(0, 0, 80, 160, cl);
//       delay_1ms(20);
//     }
//     LCD_ShowString(10, 60, "Welcome! This is SW2!", 0xFFFF);
//     break;
//   }

//   delay_1ms(3000);
// }
/////////////////////////////////////////////////////////////////////////////////
void select()
{
  LCD_Clear(BLACK);
  LCD_ShowString(25, 5, (u8 *)"EASY", WHITE);
  LCD_ShowString(20, 20, (u8 *)"NORMAL", GRAY);
  LCD_ShowString(25, 35, (u8 *)"HARD", GRAY);

// normal
normal:
  while (1)
  {
    if (Get_Button(JOY_LEFT))
    {
      LCD_ShowString(25, 5, (u8 *)"EASY", GRAY);
      LCD_ShowString(20, 20, (u8 *)"NORMAL", WHITE);
      LCD_ShowString(25, 35, (u8 *)"HARD", GRAY);
      int count = 0;
      while (count <= 300)
      {
        count++;
        delay_1ms(1);
        if (Get_Button(JOY_CTR))
          goto select_normal;
      }
      goto hard;
    }
    if (Get_Button(JOY_CTR))
      goto select_easy;
  }

select_normal:
  entry = 2;
  int blink1 = 0;
  while (blink1 < 6)
  {
    blink1++;
    LCD_ShowString(25, 5, (u8 *)"EASY", GRAY);
    LCD_ShowString(20, 20, (u8 *)"NORMAL", GRAY);
    LCD_ShowString(25, 35, (u8 *)"HARD", GRAY);
    delay_1ms(50);
    LCD_ShowString(25, 5, (u8 *)"EASY", GRAY);
    LCD_ShowString(20, 20, (u8 *)"NORMAL", WHITE);
    LCD_ShowString(25, 35, (u8 *)"HARD", GRAY);
    delay_1ms(50);
  }
  return;

// hard
hard:
  while (1)
  {
    if (Get_Button(JOY_LEFT))
    {
      LCD_ShowString(25, 5, (u8 *)"EASY", GRAY);
      LCD_ShowString(20, 20, (u8 *)"NORMAL", GRAY);
      LCD_ShowString(25, 35, (u8 *)"HARD", WHITE);
      int count = 0;
      while (count <= 300)
      {
        count++;
        delay_1ms(1);
        if (Get_Button(JOY_CTR))
          goto select_hard;
      }
      goto easy;
    }
    if (Get_Button(JOY_CTR))
      goto select_normal;
  }
select_hard:
  entry = 3;
  int blink2 = 0;
  while (blink2 < 6)
  {
    blink2++;
    LCD_ShowString(25, 5, (u8 *)"EASY", GRAY);
    LCD_ShowString(20, 20, (u8 *)"NORMAL", GRAY);
    LCD_ShowString(25, 35, (u8 *)"HARD", GRAY);
    delay_1ms(50);
    LCD_ShowString(25, 5, (u8 *)"EASY", GRAY);
    LCD_ShowString(20, 20, (u8 *)"NORMAL", GRAY);
    LCD_ShowString(25, 35, (u8 *)"HARD", WHITE);
    delay_1ms(50);
  }
  return;

// easy
easy:
  while (1)
  {
    if (Get_Button(JOY_LEFT))
    {
      LCD_ShowString(25, 5, (u8 *)"EASY", WHITE);
      LCD_ShowString(20, 20, (u8 *)"NORMAL", GRAY);
      LCD_ShowString(25, 35, (u8 *)"HARD", GRAY);
      int count = 0;
      while (count <= 300)
      {
        count++;
        delay_1ms(1);
        if (Get_Button(JOY_CTR))
          goto select_easy;
      }
      goto normal;
    }
    if (Get_Button(JOY_CTR))
      goto select_hard;
  }
select_easy:
  entry = 1;
  int blink3 = 0;
  while (blink3 < 6)
  {
    blink3++;
    LCD_ShowString(25, 5, (u8 *)"EASY", GRAY);
    LCD_ShowString(20, 20, (u8 *)"NORMAL", GRAY);
    LCD_ShowString(25, 35, (u8 *)"HARD", GRAY);
    delay_1ms(50);
    LCD_ShowString(25, 5, (u8 *)"EASY", WHITE);
    LCD_ShowString(20, 20, (u8 *)"NORMAL", GRAY);
    LCD_ShowString(25, 35, (u8 *)"HARD", GRAY);
    delay_1ms(50);
  }
  return;
}



int OBSTACLE_GAP;
int FRAME_DELAY;

typedef struct
{
  int x;
  int gapY;
  int gapHeight;
} Obstacle;

typedef struct
{
  float y;
  float velocity;
} Bird;

Obstacle obstacles[MAX_OBSTACLES];
Bird bird;
int score = 0;
int lives = 999;


int trace = 0;
int BIRDY1;
int BIRDY2;
int BIRDY3;
int BIRDY4;
int BIRDY5;
int BIRDY6;
int BIRDY7;
int BIRDY8;
int BIRDY9;
int BIRDY10;
int BIRDY11;

void updateBird()
{
  LCD_DrawPoint_big(BIRD_START_X, bird.y, BLACK);
  if (!Get_Button(JOY_LEFT))
  {
    bird.velocity += GRAVITY;
  }
  else
  {
    bird.velocity = JUMP_STRENGTH;
  }
  bird.y += bird.velocity;

  if (trace < 8)
  {
    trace++;
    if (trace == 1)
    {
      BIRDY1 = bird.y;
      LCD_DrawLine(BIRD_START_X - 1, BIRD_START_Y, BIRD_START_X, BIRDY1, GREEN);
    }
    else if (trace == 2)
    {
      BIRDY2 = bird.y;
      LCD_DrawLine(BIRD_START_X - 1, BIRD_START_Y, BIRD_START_X, BIRDY1, BLACK);

      LCD_DrawLine(BIRD_START_X - 2, BIRD_START_Y, BIRD_START_X - 1, BIRDY1, GREEN);
      LCD_DrawLine(BIRD_START_X - 1, BIRDY1, BIRD_START_X, BIRDY2, GREEN);
    }
    else if (trace == 3)
    {
      BIRDY3 = bird.y;
      LCD_DrawLine(BIRD_START_X - 2, BIRD_START_Y, BIRD_START_X - 1, BIRDY1, BLACK);
      LCD_DrawLine(BIRD_START_X - 1, BIRDY1, BIRD_START_X, BIRDY2, BLACK);

      LCD_DrawLine(BIRD_START_X - 3, BIRD_START_Y, BIRD_START_X - 2, BIRDY1, GREEN);
      LCD_DrawLine(BIRD_START_X - 2, BIRDY1, BIRD_START_X - 1, BIRDY2, GREEN);
      LCD_DrawLine(BIRD_START_X - 1, BIRDY2, BIRD_START_X, BIRDY3, GREEN);
    }
    else if (trace == 4)
    {
      BIRDY4 = bird.y;
      LCD_DrawLine(BIRD_START_X - 3, BIRD_START_Y, BIRD_START_X - 2, BIRDY1, BLACK);
      LCD_DrawLine(BIRD_START_X - 2, BIRDY1, BIRD_START_X - 1, BIRDY2, BLACK);
      LCD_DrawLine(BIRD_START_X - 1, BIRDY2, BIRD_START_X, BIRDY3, BLACK);

      LCD_DrawLine(BIRD_START_X - 4, BIRD_START_Y, BIRD_START_X - 3, BIRDY1, GREEN);
      LCD_DrawLine(BIRD_START_X - 3, BIRDY1, BIRD_START_X - 2, BIRDY2, GREEN);
      LCD_DrawLine(BIRD_START_X - 2, BIRDY2, BIRD_START_X - 1, BIRDY3, GREEN);
      LCD_DrawLine(BIRD_START_X - 1, BIRDY3, BIRD_START_X, BIRDY4, GREEN);
    }
    else if (trace == 5)
    {
      BIRDY5 = bird.y;
      LCD_DrawLine(BIRD_START_X - 4, BIRD_START_Y, BIRD_START_X - 3, BIRDY1, BLACK);
      LCD_DrawLine(BIRD_START_X - 3, BIRDY1, BIRD_START_X - 2, BIRDY2, BLACK);
      LCD_DrawLine(BIRD_START_X - 2, BIRDY2, BIRD_START_X - 1, BIRDY3, BLACK);
      LCD_DrawLine(BIRD_START_X - 1, BIRDY3, BIRD_START_X, BIRDY4, BLACK);

      LCD_DrawLine(BIRD_START_X - 5, BIRD_START_Y, BIRD_START_X - 4, BIRDY1, GREEN);
      LCD_DrawLine(BIRD_START_X - 4, BIRDY1, BIRD_START_X - 3, BIRDY2, GREEN);
      LCD_DrawLine(BIRD_START_X - 3, BIRDY2, BIRD_START_X - 2, BIRDY3, GREEN);
      LCD_DrawLine(BIRD_START_X - 2, BIRDY3, BIRD_START_X - 1, BIRDY4, GREEN);
      LCD_DrawLine(BIRD_START_X - 1, BIRDY4, BIRD_START_X, BIRDY5, GREEN);
    }
    else if (trace == 6)
    {
      BIRDY6 = bird.y;
      LCD_DrawLine(BIRD_START_X - 5, BIRD_START_Y, BIRD_START_X - 4, BIRDY1, BLACK);
      LCD_DrawLine(BIRD_START_X - 4, BIRDY1, BIRD_START_X - 3, BIRDY2, BLACK);
      LCD_DrawLine(BIRD_START_X - 3, BIRDY2, BIRD_START_X - 2, BIRDY3, BLACK);
      LCD_DrawLine(BIRD_START_X - 2, BIRDY3, BIRD_START_X - 1, BIRDY4, BLACK);
      LCD_DrawLine(BIRD_START_X - 1, BIRDY4, BIRD_START_X, BIRDY5, BLACK);

      LCD_DrawLine(BIRD_START_X - 6, BIRD_START_Y, BIRD_START_X - 5, BIRDY1, GREEN);
      LCD_DrawLine(BIRD_START_X - 5, BIRDY1, BIRD_START_X - 4, BIRDY2, GREEN);
      LCD_DrawLine(BIRD_START_X - 4, BIRDY2, BIRD_START_X - 3, BIRDY3, GREEN);
      LCD_DrawLine(BIRD_START_X - 3, BIRDY3, BIRD_START_X - 2, BIRDY4, GREEN);
      LCD_DrawLine(BIRD_START_X - 2, BIRDY4, BIRD_START_X - 1, BIRDY5, GREEN);
      LCD_DrawLine(BIRD_START_X - 1, BIRDY5, BIRD_START_X, BIRDY6, GREEN);
    }
    else if (trace == 7)
    {
      BIRDY7 = bird.y;
      LCD_DrawLine(BIRD_START_X - 6, BIRD_START_Y, BIRD_START_X - 5, BIRDY1, BLACK);
      LCD_DrawLine(BIRD_START_X - 5, BIRDY1, BIRD_START_X - 4, BIRDY2, BLACK);
      LCD_DrawLine(BIRD_START_X - 4, BIRDY2, BIRD_START_X - 3, BIRDY3, BLACK);
      LCD_DrawLine(BIRD_START_X - 3, BIRDY3, BIRD_START_X - 2, BIRDY4, BLACK);
      LCD_DrawLine(BIRD_START_X - 2, BIRDY4, BIRD_START_X - 1, BIRDY5, BLACK);
      LCD_DrawLine(BIRD_START_X - 1, BIRDY5, BIRD_START_X, BIRDY6, BLACK);

      LCD_DrawLine(BIRD_START_X - 7, BIRD_START_Y, BIRD_START_X - 6, BIRDY1, GREEN);
      LCD_DrawLine(BIRD_START_X - 6, BIRDY1, BIRD_START_X - 5, BIRDY2, GREEN);
      LCD_DrawLine(BIRD_START_X - 5, BIRDY2, BIRD_START_X - 4, BIRDY3, GREEN);
      LCD_DrawLine(BIRD_START_X - 4, BIRDY3, BIRD_START_X - 3, BIRDY4, GREEN);
      LCD_DrawLine(BIRD_START_X - 3, BIRDY4, BIRD_START_X - 2, BIRDY5, GREEN);
      LCD_DrawLine(BIRD_START_X - 2, BIRDY5, BIRD_START_X - 1, BIRDY6, GREEN);
      LCD_DrawLine(BIRD_START_X - 1, BIRDY6, BIRD_START_X, BIRDY7, GREEN);
    }
  }
  else
  {
    trace = 8;
    BIRDY8 = bird.y;

    LCD_DrawLine(BIRD_START_X - 7, BIRD_START_Y, BIRD_START_X - 6, BIRDY1, BLACK);
    LCD_DrawLine(BIRD_START_X - 6, BIRDY1, BIRD_START_X - 5, BIRDY2, BLACK);
    LCD_DrawLine(BIRD_START_X - 5, BIRDY2, BIRD_START_X - 4, BIRDY3, BLACK);
    LCD_DrawLine(BIRD_START_X - 4, BIRDY3, BIRD_START_X - 3, BIRDY4, BLACK);
    LCD_DrawLine(BIRD_START_X - 3, BIRDY4, BIRD_START_X - 2, BIRDY5, BLACK);
    LCD_DrawLine(BIRD_START_X - 2, BIRDY5, BIRD_START_X - 1, BIRDY6, BLACK);
    LCD_DrawLine(BIRD_START_X - 1, BIRDY6, BIRD_START_X, BIRDY7, BLACK);

    LCD_DrawLine(BIRD_START_X - 7, BIRDY1, BIRD_START_X - 6, BIRDY2, GREEN);
    LCD_DrawLine(BIRD_START_X - 6, BIRDY2, BIRD_START_X - 5, BIRDY3, GREEN);
    LCD_DrawLine(BIRD_START_X - 5, BIRDY3, BIRD_START_X - 4, BIRDY4, GREEN);
    LCD_DrawLine(BIRD_START_X - 4, BIRDY4, BIRD_START_X - 3, BIRDY5, GREEN);
    LCD_DrawLine(BIRD_START_X - 3, BIRDY5, BIRD_START_X - 2, BIRDY6, GREEN);
    LCD_DrawLine(BIRD_START_X - 2, BIRDY6, BIRD_START_X - 1, BIRDY7, GREEN);
    LCD_DrawLine(BIRD_START_X - 1, BIRDY7, BIRD_START_X, BIRDY8, GREEN);

    BIRD_START_Y = BIRDY1;
    BIRDY1 = BIRDY2;
    BIRDY2 = BIRDY3;
    BIRDY3 = BIRDY4;
    BIRDY4 = BIRDY5;
    BIRDY5 = BIRDY6;
    BIRDY6 = BIRDY7;
    BIRDY7 = BIRDY8;
    BIRDY8 = BIRD_START_Y;
  }

  if (bird.y > SCREEN_BOTTOM)
  {
    bird.y = 100;
    bird.velocity = 0;
    lives--;
    trace = 0; /////////////////////////
    LCD_DrawLine(BIRD_START_X - 7, BIRD_START_Y, BIRD_START_X - 6, BIRDY1, BLACK);
    LCD_DrawLine(BIRD_START_X - 6, BIRDY1, BIRD_START_X - 5, BIRDY2, BLACK);
    LCD_DrawLine(BIRD_START_X - 5, BIRDY2, BIRD_START_X - 4, BIRDY3, BLACK);
    LCD_DrawLine(BIRD_START_X - 4, BIRDY3, BIRD_START_X - 3, BIRDY4, BLACK);
    LCD_DrawLine(BIRD_START_X - 3, BIRDY4, BIRD_START_X - 2, BIRDY5, BLACK);
    LCD_DrawLine(BIRD_START_X - 2, BIRDY5, BIRD_START_X - 1, BIRDY6, BLACK);
    LCD_DrawLine(BIRD_START_X - 1, BIRDY6, BIRD_START_X, BIRDY7, BLACK);
  }
  if (bird.y < SCREEN_TOP)
  {
    bird.y = 100;
    bird.velocity = 0;
    lives--;
    trace = 0; /////////////////////////
    LCD_DrawLine(BIRD_START_X - 7, BIRD_START_Y, BIRD_START_X - 6, BIRDY1, BLACK);
    LCD_DrawLine(BIRD_START_X - 6, BIRDY1, BIRD_START_X - 5, BIRDY2, BLACK);
    LCD_DrawLine(BIRD_START_X - 5, BIRDY2, BIRD_START_X - 4, BIRDY3, BLACK);
    LCD_DrawLine(BIRD_START_X - 4, BIRDY3, BIRD_START_X - 3, BIRDY4, BLACK);
    LCD_DrawLine(BIRD_START_X - 3, BIRDY4, BIRD_START_X - 2, BIRDY5, BLACK);
    LCD_DrawLine(BIRD_START_X - 2, BIRDY5, BIRD_START_X - 1, BIRDY6, BLACK);
    LCD_DrawLine(BIRD_START_X - 1, BIRDY6, BIRD_START_X, BIRDY7, BLACK);
  }
  // LCD_Fill(BIRD_START_X, bird.y - BIRD_SIZE, BIRD_START_X + BIRD_SIZE, bird.y + BIRD_SIZE, GREEN);
}

void updateObstacles()
{
  for (int i = 0; i < MAX_OBSTACLES; i++)
  {
    LCD_DrawLine(obstacles[i].x, SCREEN_TOP, obstacles[i].x + OBSTACLE_WIDTH, obstacles[i].gapY, BLACK);
    LCD_DrawLine(obstacles[i].x, obstacles[i].gapY + obstacles[i].gapHeight, obstacles[i].x + OBSTACLE_WIDTH, SCREEN_BOTTOM, BLACK);
    LCD_DrawLine(obstacles[i].x + 1, SCREEN_TOP, obstacles[i].x + OBSTACLE_WIDTH, obstacles[i].gapY, BLACK);
    LCD_DrawLine(obstacles[i].x + 1, obstacles[i].gapY + obstacles[i].gapHeight, obstacles[i].x + OBSTACLE_WIDTH, SCREEN_BOTTOM, BLACK);
    obstacles[i].x -= 1;
    if (obstacles[i].x < SCREEN_LEFT - OBSTACLE_WIDTH)
    {
      obstacles[i].x = 120;
      obstacles[i].gapY = rand() % (SCREEN_BOTTOM - SCREEN_TOP - OBSTACLE_GAP - 10) + 10 + SCREEN_TOP;
      obstacles[i].gapHeight = OBSTACLE_GAP;
      // score++;
    }
  }
}

void draw()
{
  LCD_DrawPoint_big(BIRD_START_X, bird.y, GREEN);
  for (int i = 0; i < MAX_OBSTACLES; i++)
  {
    LCD_DrawLine(obstacles[i].x, SCREEN_TOP, obstacles[i].x + OBSTACLE_WIDTH, obstacles[i].gapY, WHITE);
    LCD_DrawLine(obstacles[i].x, obstacles[i].gapY + obstacles[i].gapHeight, obstacles[i].x + OBSTACLE_WIDTH, SCREEN_BOTTOM, WHITE);
    LCD_DrawLine(obstacles[i].x + 1, SCREEN_TOP, obstacles[i].x + OBSTACLE_WIDTH, obstacles[i].gapY, WHITE);
    LCD_DrawLine(obstacles[i].x + 1, obstacles[i].gapY + obstacles[i].gapHeight, obstacles[i].x + OBSTACLE_WIDTH, SCREEN_BOTTOM, WHITE);
  }
  char buf[20];
  sprintf(buf, "Score:%d", score);
  LCD_ShowString(SCREEN_LEFT, 20, (u8 *)buf, WHITE);
  sprintf(buf, "Lives:%d", lives);
  LCD_ShowString(SCREEN_LEFT, 5, (u8 *)buf, WHITE);
}

void checkCollisions()
{
  for (int i = 0; i < MAX_OBSTACLES; i++)
  {
    // if (BIRD_START_X + BIRD_SIZE >= obstacles[i].x && BIRD_START_X <= obstacles[i].x + OBSTACLE_WIDTH)
    if ((obstacles[i].x >= BIRD_START_X - 2) && (obstacles[i].x <= BIRD_START_X + 2))
    {
      if (bird.y < obstacles[i].gapY || bird.y > obstacles[i].gapY + obstacles[i].gapHeight)
      {
        if (obstacles[i].x == BIRD_START_X)
        {
          lives--;
          bird.y = 90;
          bird.velocity = 0;
          trace = 0; /////////////////////////
          LCD_DrawLine(BIRD_START_X - 7, BIRD_START_Y, BIRD_START_X - 6, BIRDY1, BLACK);
          LCD_DrawLine(BIRD_START_X - 6, BIRDY1, BIRD_START_X - 5, BIRDY2, BLACK);
          LCD_DrawLine(BIRD_START_X - 5, BIRDY2, BIRD_START_X - 4, BIRDY3, BLACK);
          LCD_DrawLine(BIRD_START_X - 4, BIRDY3, BIRD_START_X - 3, BIRDY4, BLACK);
          LCD_DrawLine(BIRD_START_X - 3, BIRDY4, BIRD_START_X - 2, BIRDY5, BLACK);
          LCD_DrawLine(BIRD_START_X - 2, BIRDY5, BIRD_START_X - 1, BIRDY6, BLACK);
          LCD_DrawLine(BIRD_START_X - 1, BIRDY6, BIRD_START_X, BIRDY7, BLACK);
        }
      }
      else
      {
        if (obstacles[i].x == BIRD_START_X)
        {
          if (!(bird.y < obstacles[i].gapY || bird.y > obstacles[i].gapY + obstacles[i].gapHeight))
          {
            score++;
          }
        }
      }
    }
  }
}

int main()
{

  IO_init();
  LCD_Clear(BLACK);

  int buttonCode = detectButtonPress();
  displayWelcomeScreen(buttonCode);
  LCD_Clear(BLACK);
  select(); // select difficulty
  LCD_Clear(BLACK);
  if (entry == 1 || entry == 0)
  {
    OBSTACLE_GAP = 80;
    FRAME_DELAY = 20;
  }
  else if (entry == 2)
  {
    OBSTACLE_GAP = 55;
    FRAME_DELAY = 15;
  }
  else if (entry == 3)
  {
    OBSTACLE_GAP = 40;
    FRAME_DELAY = 10;
  }
  LCD_Clear(BLACK);
  Lcd_Init();
  LCD_Clear(BLACK);
  bird.y = BIRD_START_Y;
  bird.velocity = 5;
  for (int i = 0; i < MAX_OBSTACLES; i++)
  {
    obstacles[i].x = 80 + i * OBSTACLE_SPACE;
    obstacles[i].gapY = rand() % (SCREEN_BOTTOM - SCREEN_TOP - OBSTACLE_GAP - 10) + 10 + SCREEN_TOP;
    obstacles[i].gapHeight = OBSTACLE_GAP;
  }
  LCD_Clear(BLACK);
  while (1)
  {
    uint64_t frame_start_time = get_timer_value();
    if (lives <= 2)
    {
      lives = 999;
    }
    if (score >= 998)
    {
      score = 0;
    }
    LCD_DrawLine(2, 40, 78, 40, GREEN);
    LCD_DrawLine(2, 40, 2, 158, GREEN);
    LCD_DrawLine(2, 158, 78, 158, GREEN);
    LCD_DrawLine(78, 40, 78, 158, GREEN);

    updateBird();
    updateObstacles();

    checkCollisions();

    draw();

    uint64_t frame_end_time = get_timer_value();
    uint32_t compute_time = (frame_end_time - frame_start_time) * 1000 / SystemCoreClock;
    uint32_t delay_time = FRAME_DELAY - compute_time;
    delay_1ms(delay_time);
  }
}