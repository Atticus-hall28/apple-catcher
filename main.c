#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#ifdef _WIN32
    #include <windows.h>
#else
  #include <unistd.h>
#endif


const double TARGET_FPS = 60.0;
const double TARGET_FRAME_TIME = 1.0 / TARGET_FPS; // Time per frame in seconds




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

    
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

     if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_FLAC) != (MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_FLAC) ) {
        fprintf(stderr, "Mix_Init Error: %s\n", Mix_GetError());
        SDL_Quit();
        return 1;
     }


    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
     }

    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048) < 0) {
        fprintf(stderr, "Mix_OpenAudio Error: %s\n", Mix_GetError());
        Mix_Quit();
        SDL_Quit();
        return 1;
     }
    
    int width = 640;
    int hight = 480;
    unsigned int seed = time(0);
    srand(seed);
    int score =0;

    char str_score[32];
     int numChars = snprintf(str_score, sizeof(str_score), "%d", score);
       if(numChars > sizeof(str_score) - 1){
         fprintf(stderr,"Error: The string buffer was too small, the integer might have been truncated! \n");
         return 1;
       }

    SDL_Window *window = SDL_CreateWindow("game",
                                         SDL_WINDOWPOS_UNDEFINED,
                                         SDL_WINDOWPOS_UNDEFINED,
                                         width, hight,
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

    //load the collect sound effect
     Mix_Chunk *sound = Mix_LoadWAV("assets/sounds/collect.wav");
       if (sound == NULL) {
         fprintf(stderr, "Mix_LoadMP3 Error: %s\n", Mix_GetError());
          Mix_CloseAudio();
            SDL_DestroyRenderer(renderer);
             SDL_DestroyWindow(window);
           Mix_Quit();
         SDL_Quit();
          return 1;
        }

    // Load a font
    TTF_Font *font = TTF_OpenFont("assets/fonts/DejaVuSansMono.ttf", 20);
    if (font == NULL) {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
       SDL_DestroyRenderer(renderer);
       SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }




    SDL_Color textColor = {255, 255, 255, 255}; // White color
 


    SDL_Surface *textSurface = NULL;
    SDL_Texture *textTexture = NULL;


    int quit = 0;

   
    SDL_Point apple_pos = {width/2,40};
    SDL_Rect apple = {apple_pos.x,apple_pos.y,10,10};

    SDL_Point player_pos = {100,hight-50};
    SDL_Rect player = {player_pos.x,player_pos.y,100,50};




    //The key mapings are w a s d space

    bool keys_down[8];

    for(int i = 0;i < (sizeof(keys_down) / sizeof(keys_down[0])); i++ ){
        keys_down[i] = 0;
    }


    SDL_Event event;



    int frame_since_last_drop = 0;
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


            double apple_speed = 8;
            
            if(frame_since_last_drop == 30) {
                apple_pos.x = rand() % width-1;
                apple_pos.y = 40;
                apple.h = 10;
                apple.w = 10;
            }
            else if(frame_since_last_drop >= 60) {

                if(SDL_HasIntersection(&apple,&player)) {

                    frame_since_last_drop = 0;
                    score ++;
                    Mix_PlayChannel(-1, sound, 0); // Play once on first available channel
                    apple.w = 0;
                    apple.h = 0;
                }else if(apple_pos.y >= hight-5){
                    frame_since_last_drop = 0;
                    score --;
                    apple.w = 0;
                    apple.h = 0;
                }

                move(&apple_pos,90,apple_speed);
            } 
                frame_since_last_drop ++;
            



            apple.x = apple_pos.x;
            apple.y = apple_pos.y;

            numChars = snprintf(str_score, sizeof(str_score), "%d", score);
            if(numChars > sizeof(str_score) - 1){
                fprintf(stderr,"Error: The string buffer was too small, the integer might have been truncated! \n");
                return 1;
            }

       double speed = 8.0;
 
      if(keys_down[1] == true && player_pos.x > 0){
        move(&player_pos, 180.0, speed); // left
      }
    
        if(keys_down[3] == true && player_pos.x < width-player.w){
          move(&player_pos, 0.0, speed); // right
      }
        // Update the player's drawn position from it's origin's position
       player.x = player_pos.x;



       
       // --- End of Game Logic ---






        // --- Rendering ---



        if(textSurface != NULL){
            SDL_FreeSurface(textSurface);
        }
        if(textTexture != NULL){
            SDL_DestroyTexture(textTexture);
        }

       // Render text to an SDL_Surface
        textSurface = TTF_RenderText_Blended(font, str_score, textColor);
        if (textSurface == NULL) {
            fprintf(stderr, "TTF_RenderText_Blended Error: %s\n", TTF_GetError());
            TTF_CloseFont(font);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            TTF_Quit();
            SDL_Quit();
            return 1;
        }

                  // Create a texture from the surface
                 textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    if (textTexture == NULL) {
                       fprintf(stderr, "SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
                     SDL_FreeSurface(textSurface);
                    TTF_CloseFont(font);
                    SDL_DestroyRenderer(renderer);
                    SDL_DestroyWindow(window);
                     TTF_Quit();
                    SDL_Quit();
                   return 1;
                 }
           
      




        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 39, 84, 100, 1); 
        SDL_RenderClear(renderer); // Clear the entire screen




        if(textSurface != NULL){
              int textWidth = textSurface->w;
            int textHeight = textSurface->h;

            SDL_Rect textRect = { ((width/2)-(textWidth/2)), 10, textWidth, textHeight };
             // Draw the texture to the renderer
             if(textTexture != NULL)
             {
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            }
        }


        // Draw the player
        SDL_SetRenderDrawColor(renderer,118, 75, 30, 100);
        SDL_RenderFillRect(renderer, &player); 

        //Draw the player
        SDL_SetRenderDrawColor(renderer, 214, 5, 5, 1);
        SDL_RenderFillRect(renderer, &apple); 







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


      //Clean up
    Mix_FreeChunk(sound);
    Mix_CloseAudio();
    if(textSurface != NULL)
         SDL_FreeSurface(textSurface);
     if(textTexture != NULL)
        SDL_DestroyTexture(textTexture);
     TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    Mix_Quit();
    SDL_Quit();
}