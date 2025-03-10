#include "st7789.h"
#include "esp_timer.h"
#include <stdio.h>
#include <string.h>
#include <math.h>


uint8_t font_data[(FONT_END - FONT_START) * FONT_HEIGHT]; 


#define TEST_DURATION_SEC 999
#define M_PI 3.14159265358979323846
void draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);

uint16_t colors[] = {
    0x0000, 0xFFFF, 0xF800, 0x07E0, 0x001F,
    0xFFE0, 0xF81F, 0x07FF, 0xAAAA, 0x5555
};

static uint16_t frame_buffer[TFT_WIDTH * TFT_HEIGHT];



static uint16_t frame_buffer[TFT_WIDTH * TFT_HEIGHT];

void draw_tunnel_3d() {
    uint32_t start_time = esp_timer_get_time() / 1000000;
    while ((esp_timer_get_time() / 1000000 - start_time) < 5) {  
        clear_frame_buffer(0x0000);
        float t = (esp_timer_get_time() / 1000000 - start_time) * 0.5f;  
        for (int y = 0; y < TFT_HEIGHT; y++) {
            for (int x = 0; x < TFT_WIDTH; x++) {
                float fx = (x - TFT_WIDTH / 2.0f) / (TFT_WIDTH / 2.0f);
                float fy = (y - TFT_HEIGHT / 2.0f) / (TFT_HEIGHT / 2.0f);
                float radius = sqrtf(fx * fx + fy * fy);
                float value = logf(radius * 20 + 1) + t;
                int color_index = ((int)(value * 10)) % 10;
                if (color_index < 0) color_index = -color_index;
                draw_pixel(x, y, colors[color_index]);
            }
        }
        flush_frame_buffer();
    }
}


void draw_matrix_effect() {
    char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int columns = TFT_WIDTH / 8;
    int drops[columns];
    for (int i = 0; i < columns; i++) drops[i] = rand() % TFT_HEIGHT;
    uint32_t start_time = esp_timer_get_time() / 1000000;
    while ((esp_timer_get_time() / 1000000 - start_time) < 10) {
        clear_frame_buffer(0x0000);
        for (int i = 0; i < columns; i++) {
            int char_index = rand() % (sizeof(charset) - 1);
            draw_char_scaled(i * 8, drops[i], charset[char_index], 0x07E0, 1, font_data);
            if (++drops[i] * 8 >= TFT_HEIGHT) drops[i] = 0;
        }
        flush_frame_buffer();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void draw_bouncing_ball() {
    float x = TFT_WIDTH / 2, y = TFT_HEIGHT / 2, vx = 2, vy = 2;
    float radius = 10;
    uint32_t start_time = esp_timer_get_time() / 1000000;
    while ((esp_timer_get_time() / 1000000 - start_time) < 15) {
        clear_frame_buffer(0x0000);
        x += vx; y += vy;
        if (x - radius < 0 || x + radius > TFT_WIDTH) vx = -vx;
        if (y - radius < 0 || y + radius > TFT_HEIGHT) vy = -vy;
          draw_circle(x, y, radius, 0xF800);
          flush_frame_buffer();
          vTaskDelay(pdMS_TO_TICKS(33));
      }
  }
  

  void draw_water_filling() {
      const int NUM_PARTICLES = 50;
      
      struct Particle {
          int x;          
          float y;
          float speed;
          bool active;    
      };
      struct Particle particles[NUM_PARTICLES];
  

      int water_level[TFT_WIDTH];
      for (int x = 0; x < TFT_WIDTH; x++) {
          water_level[x] = TFT_HEIGHT;
      }
      
      for (int i = 0; i < NUM_PARTICLES; i++) {
          particles[i].x = rand() % TFT_WIDTH;
          particles[i].y = 0;
          particles[i].speed = (rand() % 3) + 4;
          particles[i].active = true;
      }
      
      uint32_t start_time = esp_timer_get_time() / 1000000;
      
      while ((esp_timer_get_time() / 1000000 - start_time) < 30) {
          clear_frame_buffer(0x0000);
          
          for (int x = 0; x < TFT_WIDTH; x++) {
              if (water_level[x] < TFT_HEIGHT) {
                  draw_rectangle(x, water_level[x], x, TFT_HEIGHT - 1, 0x001F);
              }
          }
          
          for (int i = 0; i < NUM_PARTICLES; i++) {
              if (particles[i].active) {
                  particles[i].y += particles[i].speed;
                  int col = particles[i].x;
                  if ((int)particles[i].y >= water_level[col] - 1) {
                      particles[i].y = water_level[col] - 1;
                      if (water_level[col] > 0) {
                          water_level[col] = (int)particles[i].y;
                      }
                      particles[i].active = false;
                  } else {
                      draw_pixel(particles[i].x, (int)particles[i].y, 0x001F);
                  }
              } else {
                  int col = rand() % TFT_WIDTH;
                  if (water_level[col] > 0) {
                      particles[i].x = col;
                      particles[i].y = 0;
                      particles[i].speed = (rand() % 3) + 4;
                      particles[i].active = true;
                  }
              }
          }
          
          flush_frame_buffer();
          vTaskDelay(pdMS_TO_TICKS(33));
      }
  }
  

  void draw_plasma_effect() {
    uint32_t start_time = esp_timer_get_time() / 1000000;
    while ((esp_timer_get_time() / 1000000 - start_time) < 15) {
        clear_frame_buffer(0x0000);
        
        float t = (esp_timer_get_time() / 1000000 - start_time) * 0.1f;
        
        for (int y = 0; y < TFT_HEIGHT; y++) {
            for (int x = 0; x < TFT_WIDTH; x++) {
                float v = sinf(x / 10.0f + t) + sinf(y / 15.0f + t) + sinf((x + y) / 20.0f + t);
                int color_index = ((int)((v + 3) * 1.5f)) % 10;
                if (color_index < 0) color_index = -color_index;
                draw_pixel(x, y, colors[color_index]);
            }
        }
        flush_frame_buffer();

    }
}


void draw_fireworks() {
    struct Particle {
        float x, y, vx, vy;
        uint16_t color;
    } particles[50];

    for (int i = 0; i < 50; i++) {
        particles[i].x = TFT_WIDTH / 2;
        particles[i].y = TFT_HEIGHT / 2;
        particles[i].vx = (rand() % 200 - 100) / 50.0;
        particles[i].vy = (rand() % 200 - 100) / 50.0;
        particles[i].color = colors[rand() % 10];
    }
    
    uint32_t start_time = esp_timer_get_time() / 1000000;
    while ((esp_timer_get_time() / 1000000 - start_time) < 10) {
        clear_frame_buffer(0x0000);
        for (int i = 0; i < 50; i++) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            particles[i].vy += 0.05;  
            if (particles[i].y > TFT_HEIGHT) {
                particles[i].x = TFT_WIDTH / 2;
                particles[i].y = TFT_HEIGHT / 2;
                particles[i].vx = (rand() % 200 - 100) / 50.0;
                particles[i].vy = (rand() % 200 - 100) / 50.0;
            }
            draw_pixel((int)particles[i].x, (int)particles[i].y, particles[i].color);
        }
        flush_frame_buffer();
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}

void draw_chessboard(uint16_t square_size, uint16_t color1, uint16_t color2) {
    for (uint16_t y = 0; y < TFT_HEIGHT; y += square_size) {
        for (uint16_t x = 0; x < TFT_WIDTH; x += square_size) {
            uint16_t color = ((x / square_size) % 2 == (y / square_size) % 2) ? color1 : color2;
            draw_rectangle(x, y, x + square_size - 1, y + square_size - 1, color);
        }
    }
    flush_frame_buffer();
}

void draw_moving_dots() {
    for (int t = 0; t < 100; t++) {
        clear_frame_buffer(0x0000);
        for (int i = 0; i < 20; i++) {
            int x = (sin(t * 0.1 + i) * 40) + (TFT_WIDTH / 2);
            int y = (cos(t * 0.1 + i) * 40) + (TFT_HEIGHT / 2);
            draw_circle(x, y, 2, 0xFFFF);
        }
        flush_frame_buffer();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void draw_pixel_explosion() {
    clear_frame_buffer(0x0000);
    for (int i = 0; i < 333; i++) {
        int x = TFT_WIDTH / 2 + (rand() % (i + 1) - i / 2);
        int y = TFT_HEIGHT / 2 + (rand() % (i + 1) - i / 2);
        draw_pixel(x, y, 0xFFFF);
        flush_frame_buffer();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void draw_moire_pattern() {
    clear_frame_buffer(0x0000);
    for (int i = 0; i < 90; i++) {
        for (int angle = 0; angle < 360; angle += 10) {
            int x = TFT_WIDTH / 2 + (i * cos(angle * M_PI / 180));
            int y = TFT_HEIGHT / 2 + (i * sin(angle * M_PI / 180));
            draw_pixel(x, y, 0xFFFF);
        }
        for (int angle = 0; angle < 360; angle += 10) {
            int x = TFT_WIDTH / 3 + (i * cos(angle * M_PI / 270));
            int y = TFT_HEIGHT / 3 + (i * sin(angle * M_PI / 270));
            draw_pixel(x, y, rgb888_to_rgb565(210, 135,220));
        }
        flush_frame_buffer();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void stress_test() {
    uint32_t start_time = esp_timer_get_time() / 1000000;
    uint8_t test_pattern = 0;
    const uint8_t TOTAL_PATTERNS = 15;
    uint16_t colors[] = {
        0x0000, 0xFFFF, 0xF800, 0x07E0, 0x001F,
        0xFFE0, 0xF81F, 0x07FF, 0xAAAA, 0x5555
    };

    while((esp_timer_get_time() / 1000000 - start_time) < TEST_DURATION_SEC) {
        uint32_t pattern_start = esp_timer_get_time();
        
        switch(test_pattern) {
            case 0: 
                for(int i = 0; i < 120; i++) {  
                    if((esp_timer_get_time()/1000 - pattern_start) > 2000) break;
                    
                    clear_frame_buffer(0x0000);
                    uint16_t phase = (i * 3) % TFT_WIDTH;
                    for(uint16_t x = 0; x < TFT_WIDTH; x++) {
                        uint16_t color = colors[(abs(x - phase)/20) % (sizeof(colors)/sizeof(uint16_t))];
                        draw_rectangle(x, 0, x, TFT_HEIGHT-1, color);
                    }
                    flush_frame_buffer();
                    vTaskDelay(pdMS_TO_TICKS(16));  
                }
                break;


            case 1: 
                for(int i = 0; i < 360*2; i += 5) {  
                    if((esp_timer_get_time()/1000 - pattern_start) > 4000) break;
                    
                    clear_frame_buffer(0x0000);
                    uint16_t center_x = TFT_WIDTH/2;
                    uint16_t center_y = TFT_HEIGHT/2;
                    
                    for(int r = 0; r < 60; r += 2) {
                        float angle = (i + r*3) * M_PI / 180.0;
                        uint16_t x = center_x + r * cos(angle);
                        uint16_t y = center_y + r * sin(angle);
                        draw_circle(x, y, 5, colors[r % (sizeof(colors)/sizeof(uint16_t))]);
                    }
                    flush_frame_buffer();
                    vTaskDelay(pdMS_TO_TICKS(30));
                }
                break;



            case 2: 
                for(int i = 0; i < 20; i++) {  
                    if((esp_timer_get_time()/1000 - pattern_start) > 5000) break;
                    
                    clear_frame_buffer(0xFFFF);
                    
                    
                    for(int n = 0; n < 8; n++) {
                        uint16_t pos = rand() % TFT_WIDTH;
                        draw_rectangle(pos, 0, pos, TFT_HEIGHT-1, 0x0000);
                        pos = rand() % TFT_HEIGHT;
                        draw_rectangle(0, pos, TFT_WIDTH-1, pos, 0x0000);
                    }
                    

                    for(int n = 0; n < 15; n++) {
                        uint16_t color = colors[rand() % 4 + 2];  
                        uint16_t x1 = rand() % (TFT_WIDTH-20);
                        uint16_t y1 = rand() % (TFT_HEIGHT-20);
                        draw_rectangle(x1, y1, x1 + rand()%30 +10, y1 + rand()%30 +10, color);
                    }
                    flush_frame_buffer();
                    vTaskDelay(pdMS_TO_TICKS(250));
                }
                break;


            case 3: 
                clear_frame_buffer(0x0000);
                for(int i = 0; i < 1; i++) {  
                    for(int y_off = -20; y_off < TFT_HEIGHT; y_off += 2) {
                        clear_frame_buffer(0x0000);
                        draw_text_scaled(TFT_WIDTH/4, y_off, "STRESS TEST", 
                                      rgb888_to_rgb565(255, 255, 255), 1, font_data);
                        flush_frame_buffer();
                        vTaskDelay(pdMS_TO_TICKS(30));
                    }
                }
                break;
            case 4:
                draw_chessboard(20, 0x0000, 0xFFFF);
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;
            case 5:
                draw_moving_dots();
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;
            case 6:
                draw_pixel_explosion();
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;
            case 7:
                draw_moire_pattern();
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;
            case 8:
                draw_fireworks();
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;
            
            case 9:
                draw_tunnel_3d();
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;
            
            case 10: 
                draw_plasma_effect();
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;
            case 11: 
                draw_water_filling();
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;
            case 12: 
                draw_matrix_effect();
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;
            case 13: 
                draw_bouncing_ball();
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;



            
        }

        for(int i = 0; i < 3; i++) {
            clear_frame_buffer(0x0000);
            flush_frame_buffer();
            vTaskDelay(pdMS_TO_TICKS(50));
            clear_frame_buffer(0xFFFF);
            flush_frame_buffer();
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        
        test_pattern = (test_pattern + 1) % TOTAL_PATTERNS;
        if((esp_timer_get_time() / 1000000 - start_time) >= TEST_DURATION_SEC) break;
    }
}
void draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
    int x = r;
    int y = 0;
    int err = 0;

    while(x >= y) {
        draw_pixel(x0 + x, y0 + y, color);
        draw_pixel(x0 + y, y0 + x, color);
        draw_pixel(x0 - y, y0 + x, color);
        draw_pixel(x0 - x, y0 + y, color);
        draw_pixel(x0 - x, y0 - y, color);
        draw_pixel(x0 - y, y0 - x, color);
        draw_pixel(x0 + y, y0 - x, color);
        draw_pixel(x0 + x, y0 - y, color);

        if(err <= 0) {
            y += 1;
            err += 2*y + 1;
        }
        if(err > 0) {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}


void app_main() {
    mount_spiffs();  
    load_font(font_data);     
    INIT();  
    while (1)
    {
        load_image("/spiffs/1.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/2.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/3.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/4.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/5.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/6.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/7.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/8.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/9.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/10.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/11.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/12.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/13.bin");
        vTaskDelay(pdMS_TO_TICKS(1));
        load_image("/spiffs/14.bin");
        stress_test();
    }

}