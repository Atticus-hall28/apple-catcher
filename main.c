#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#ifdef _WIN32
    #include <windows.h>
#else
  #include <unistd.h>
#endif


    const double TARGET_FPS = 60.0;
    const double TARGET_FRAME_TIME = 1.0 / TARGET_FPS; // Time per frame in seconds

   typedef struct {
    double x;
    double y;
} acu_point;


double degrees_to_radians(double degrees) {

    return degrees * M_PI / 180.0;
}

void move(SDL_Point* point, double degrees, double distance) {

    //C nessesitates conversion from deg to rad for it's trig functions
    double rad = degrees_to_radians(degrees);

    double dx = cos(rad);
    double dy = sin(rad);


    point->x += dx * distance;
    point->y += dy * distance;
}

// Cross platform sleep function
void cross_platform_sleep(double seconds) {
    #ifdef _WIN32
        Sleep(seconds * 1000);
    #else //POSIX 
      struct timespec req, rem;
      req.tv_sec = (time_t)seconds;
      req.tv_nsec = (seconds - req.tv_sec) * 1000000000L;
      int ret;
      do {
        ret = nanosleep(&req, &rem);
        if (ret == -1) {
            req = rem;
        }
      } while(ret == -1);
    #endif
}
int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    //acu_point player_pos = {100,100};
    SDL_Point player_pos = {100,100};
    SDL_Rect player = {player_pos.x,player_pos.y,100,200};

    SDL_Window *window = SDL_CreateWindow("game",
                                         SDL_WINDOWPOS_UNDEFINED,
                                         SDL_WINDOWPOS_UNDEFINED,
                                         640, 480,
                                         SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
     if (renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int quit = 0;


    //The key mapings are w a s d space

    bool keys_down[8];

    for(int i = 0;i < (sizeof(keys_down) / sizeof(keys_down[0])); i++ ){
        keys_down[i] = 0;
    }


    SDL_Event event;




    while (!quit) {
        double frame_start_time = (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();

        // --- Input Handling ---
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else if (event.type == SDL_KEYDOWN) {
                
                SDL_Keycode keycode = event.key.keysym.sym;

                if(keycode == SDLK_w) {
                    keys_down[0] = true;
                }
                if(keycode == SDLK_a) {
                    keys_down[1] = true;
                }
                if(keycode == SDLK_s) {
                    keys_down[2] = true;
                }
                if(keycode == SDLK_d) {
                    keys_down[3] = true;
                }
                if(keycode == SDLK_SPACE) {
                    keys_down[4] = true;
                }

            } else if (event.type == SDL_KEYUP) {
                SDL_Keycode keycode = event.key.keysym.sym;

                if(keycode == SDLK_w) {
                    keys_down[0] = false;
                }
                if(keycode == SDLK_a) {
                    keys_down[1] = false;
                }
                if(keycode == SDLK_s) {
                    keys_down[2] = false;
                }
                if(keycode == SDLK_d) {
                    keys_down[3] = false;
                }
                if(keycode == SDLK_SPACE) {
                    keys_down[4] = false;
                }

            }


        }
        // --- End of input handling ---


           // --- Game Logic ---
       double speed = 2.0;
      if(keys_down[0] == true){
        move(&player_pos, -90.0, speed); // up
      }
      if(keys_down[1] == true){
        move(&player_pos, 180.0, speed); // left
      }
        if(keys_down[2] == true){
        move(&player_pos, 90.0, speed); // down
        }
        if(keys_down[3] == true){
          move(&player_pos, 0.0, speed); // right
      }
        // Update the player's draw position from it's more acurate position
       player.x = player_pos.x;
       player.y = player_pos.y;
       // --- End of Game Logic ---

        // --- Rendering ---
        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set the color to black
        SDL_RenderClear(renderer); // Clear the entire screen


        //Set the color to white, to draw the filled rectangle
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);


        SDL_RenderFillRect(renderer, &player); // Draw a white rectangle


        SDL_RenderPresent(renderer); // Update the window

      // --- End of Rendering ---


    // Frame Rate Limiting
        double frame_end_time = (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
        double elapsed_time = frame_end_time - frame_start_time;
        double sleep_time = TARGET_FRAME_TIME - elapsed_time;
        if(sleep_time > 0)
      {
        cross_platform_sleep(sleep_time);
      }





    }


    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}