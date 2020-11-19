#include "n64_audio.h"
#include "n64_input.h"
#include "n64_renderer.h"

#include "../filesystem.h"
#include "../game.h"

#include <libdragon.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

static volatile int tick_count = 0;

void timer_tick(int overflow) {
    tick_count += 1;
}

int main(void)
{
    init_interrupts();
    display_init( RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE );
    filesystem_init(NULL);
    rdp_init();
    controller_init();
    timer_init();


    Audio* audio = n64_audio_create();
    Input* input = n64_input_create();
    Renderer* renderer = n64_renderer_create(SCREEN_WIDTH, SCREEN_HEIGHT);

    Game* game = game_create(audio, input, renderer);

    new_timer(TIMER_TICKS(1000), TF_CONTINUOUS, timer_tick);
    //unsigned long prev_time = get_ticks_ms();


    /* Main loop test */
    while(1) 
    {
        //unsigned long current_time = get_ticks_ms();
        //float time_delta = (current_time - prev_time) / 1000.0f;
        //(void)time_delta;

        n64_audio_update(audio);

        if (tick_count >= 16) {
            float time_delta = 60.0f / 1000.0f;
            n64_input_update(input);
            game_update(game, time_delta);

            tick_count = 0;
        }

        /* Grab a render buffer */
        while( !(renderer->display_context = display_lock()) );
       
        /*Fill the screen */
        graphics_fill_screen( renderer->display_context, renderer->clear_color);

        /* Assure RDP is ready for new commands */
        rdp_sync( SYNC_PIPE );

        /* Remove any clipping windows */
        rdp_set_default_clipping();

        /* Attach RDP to display */
        rdp_attach_display( renderer->display_context );

        rdp_sync( SYNC_PIPE );

        game_draw(game);

        /* Inform the RDP we are finished drawing and that any pending operations should be flushed */
        rdp_sync( SYNC_PIPE );
        rdp_detach_display();

        /* Force backbuffer flip */
        display_show(renderer->display_context);

        //prev_time = current_time;
    }
}
