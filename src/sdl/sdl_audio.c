#include "sdl_audio.h"

#include "../filesystem.h"

#include <SDL.h>

#include <stdio.h>
#include <stdlib.h>

Audio* sdl_audio_create() {
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    int result = Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 );
    if (result == -1) {
        const char* error = Mix_GetError();
        puts(error);
        return NULL;
    }

    Audio* audio = calloc(1, sizeof(Audio));
    return audio;
}

void sdl_audio_destroy(Audio* audio) {
    free(audio);
    Mix_CloseAudio();
    SDL_AudioQuit();
}

static char* music_extensions[2] = {"mod", "ogg"};

Music* audio_load_music(Audio* audio, const char* path) {
    const char* base_path = filesystem_get_asset_base_path();
    char* music_path = malloc(strlen(base_path) + strlen(path) + 12);

    Mix_Music* sdl_music = NULL;
    for (int i = 0; i < 2; i++) {
        sprintf(music_path, "%s/%s.%s", base_path, path, music_extensions[i]);
        sdl_music= Mix_LoadMUS(music_path);

        if (sdl_music) break;
    }

    free(music_path);

    if (!sdl_music)
        return NULL;

    Music* music = malloc(sizeof(Music));
    music->sdl_music = sdl_music;

    return music;
}

void audio_destroy_music(Audio* audio, Music* music) {
    if (audio->current_music == music)
        audio->current_music = NULL;

    Mix_FreeMusic(music->sdl_music);
    free(music);
}

void audio_play_music(Audio* audio, Music* music) {
    Mix_PlayMusic( music->sdl_music, -1);
    audio->current_music = music;
}

void audio_pause_music(Audio* audio) {
    Mix_PauseMusic();
    (void)audio;
}

void audio_resume_music(Audio* audio) {
    Mix_ResumeMusic();
    (void)audio;
}

void audio_restart_music(Audio* audio) {
    Mix_RewindMusic();
    Mix_ResumeMusic();
    (void)audio;
}