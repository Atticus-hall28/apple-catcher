#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
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

typedef struct {
    double hue;        // 0-360
    double saturation; // 0-1
    double value;      // 0-1
    double alpha;      // 0-1
} HSV_Color;


SDL_Color hsvToRgb(HSV_Color hsv) {
    double h = hsv.hue;
    double s = hsv.saturation;
    double v = hsv.value;

    SDL_Color rgb;
    rgb.a = hsv.alpha;

    if (s == 0) { // Grayscale
        rgb.r = rgb.g = rgb.b = (int)(v * 255.0);
    } else {
        double c = v * s;
        double h_prime = h / 60.0;
        double x = c * (1 - fabs(fmod(h_prime, 2) - 1));
        double r1, g1, b1;

        if (h_prime < 1) {
            r1 = c;  g1 = x;  b1 = 0;
        } else if (h_prime < 2) {
            r1 = x;  g1 = c;  b1 = 0;
        } else if (h_prime < 3) {
            r1 = 0;  g1 = c;  b1 = x;
        } else if (h_prime < 4) {
            r1 = 0;  g1 = x;  b1 = c;
        } else if (h_prime < 5) {
            r1 = x;  g1 = 0;  b1 = c;
        } else {
            r1 = c;  g1 = 0;  b1 = x;
        }

        double m = v - c;
        rgb.r = (int)((r1 + m) * 255.0);
        rgb.g = (int)((g1 + m) * 255.0);
        rgb.b = (int)((b1 + m) * 255.0);

        // Clamp values
        if (rgb.r < 0)   rgb.r = 0;
        if (rgb.r > 255) rgb.r = 255;
        if (rgb.g < 0)   rgb.g = 0;
        if (rgb.g > 255) rgb.g = 255;
        if (rgb.b < 0)   rgb.b = 0;
        if (rgb.b > 255) rgb.b = 255;
    }
    return rgb;
}

double degrees_to_radians(double degrees) {
    return degrees * M_PI / 180.0;
}

void move(SDL_Point* point, double degrees, double distance) {
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
    int score = 0;

    char str_score[32];
    int numChars = snprintf(str_score, sizeof(str_score), "%d", score);
    if (numChars > sizeof(str_score) - 1) {
        fprintf(stderr, "Error: The string buffer was too small, the integer might have been truncated! \n");
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


    SDL_Color text_color = {255, 255, 255, 255}; // White color
    HSV_Color apple_hsv = {0, .977, 0.834,1};
    SDL_Color apple_color = hsvToRgb(apple_hsv);
    SDL_Color background_color = {39, 84, 100, 1}; // blueish
    SDL_Color player_color = {118, 75, 30, 255}; // broawn


    SDL_Surface *textSurface = NULL;
    SDL_Texture *textTexture = NULL;

    int quit = 0;

    SDL_Point apple_pos = {width / 2, 40};
    SDL_Rect apple = {apple_pos.x, apple_pos.y, 10, 10};

    SDL_Point player_pos = {100, hight - 50};
    SDL_Rect player = {player_pos.x, player_pos.y, 100, 50};

    //The key mapings are w a s d space
    bool keys_down[8];

    for (int i = 0; i < (sizeof(keys_down) / sizeof(keys_down[0])); i++) {
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

                if (keycode == SDLK_w)     keys_down[0] = true;
                if (keycode == SDLK_a)     keys_down[1] = true;
                if (keycode == SDLK_s)     keys_down[2] = true;
                if (keycode == SDLK_d)     keys_down[3] = true;
                if (keycode == SDLK_SPACE) keys_down[4] = true;
            } else if (event.type == SDL_KEYUP) {
                SDL_Keycode keycode = event.key.keysym.sym;

                if (keycode == SDLK_w)     keys_down[0] = false;
                if (keycode == SDLK_a)     keys_down[1] = false;
                if (keycode == SDLK_s)     keys_down[2] = false;
                if (keycode == SDLK_d)     keys_down[3] = false;
                if (keycode == SDLK_SPACE) keys_down[4] = false;
            }
        }
        // --- End of input handling ---

        // --- Game Logic ---
        double apple_speed = 8;

        if (frame_since_last_drop == 30) {
            apple_pos.x = rand() % width - 1;
            apple_pos.y = 40;
            apple.h = 10;
            apple.w = 10;
        } else if (frame_since_last_drop >= 60) {
            if (SDL_HasIntersection(&apple, &player)) {
                frame_since_last_drop = 0;
                score++;
                Mix_PlayChannel(-1, sound, 0); // Play once on first available channel
                apple.w = 0;
                apple.h = 0;
            } else if (apple_pos.y >= hight - 5) {
                frame_since_last_drop = 0;
                score--;
                apple.w = 0;
                apple.h = 0;
            }
            move(&apple_pos, 90, apple_speed);
        }
        frame_since_last_drop++;

        apple.x = apple_pos.x;
        apple.y = apple_pos.y;

        numChars = snprintf(str_score, sizeof(str_score), "%d", score);
        if (numChars > sizeof(str_score) - 1) {
            fprintf(stderr, "Error: The string buffer was too small, the integer might have been truncated! \n");
            return 1;
        }

        double speed = 8.0;

        if (keys_down[1] == true && player_pos.x > 0) {
            move(&player_pos, 180.0, speed); // left
        }

        if (keys_down[3] == true && player_pos.x < width - player.w) {
            move(&player_pos, 0.0, speed);  // right
        }

        // Update the player's drawn position from it's origin's position
        player.x = player_pos.x;

        /*
        un comment for rainbow apple
        if (frame_since_last_drop >= 30) {
            if (apple_hsv.hue < 360) {
                apple_hsv.hue++;
            } else {
                apple_hsv.hue = 0;
            }
            apple_color = hsvToRgb(apple_hsv);
        }

        */

        // --- End of Game Logic ---

        // --- Rendering ---
        if (textSurface != NULL) {
            SDL_FreeSurface(textSurface);
        }
        if (textTexture != NULL) {
            SDL_DestroyTexture(textTexture);
        }

        // Render text to an SDL_Surface
        textSurface = TTF_RenderText_Blended(font, str_score, text_color);
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
        SDL_SetRenderDrawColor(renderer, background_color.r, background_color.g, background_color.b, background_color.a);
        SDL_RenderClear(renderer); // Clear the entire screen

        if (textSurface != NULL) {
            int textWidth = textSurface->w;
            int textHeight = textSurface->h;

            SDL_Rect textRect = {((width / 2) - (textWidth / 2)), 10, textWidth, textHeight};
            // Draw the texture to the renderer
            if (textTexture != NULL) {
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            }
        }

        // Draw the player
        SDL_SetRenderDrawColor(renderer, player_color.r, player_color.g, player_color.b, player_color.a);
        SDL_RenderFillRect(renderer, &player);

        //Draw the apple
        SDL_SetRenderDrawColor(renderer, apple_color.r, apple_color.g, apple_color.b, apple_color.a);
        SDL_RenderFillRect(renderer, &apple);

        SDL_RenderPresent(renderer); // Update the window

        // --- End of Rendering ---

        // Frame Rate Limiting
        double frame_end_time = (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
        double elapsed_time = frame_end_time - frame_start_time;
        double sleep_time = TARGET_FRAME_TIME - elapsed_time;

        if (sleep_time > 0) {
            cross_platform_sleep(sleep_time);
        }
    }

    // Clean up
    Mix_FreeChunk(sound);
    Mix_CloseAudio();
    if (textSurface != NULL)
        SDL_FreeSurface(textSurface);
    if (textTexture != NULL)
        SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    Mix_Quit();
    SDL_Quit();
    return 0;
}