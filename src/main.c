#include "../cosmoslib64/cosams.h"
#include "../cosmoslib64/cosdebug.h"

float get_time_s()  { return (float)((double)get_ticks_ms() / 1000.0); }

int main(void)
{
    asset_init_compression(2);
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2,
        GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);
    rdpq_init();
    int dfs_handle = dfs_init(DFS_DEFAULT_LOCATION);
    joypad_init();

    sprite_t *test_sprite = sprite_load("rom:/test.ci4.sprite");

    rdpq_font_t* fnt = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
    rdpq_font_style(fnt, 1, &(rdpq_fontstyle_t){RGBA32(0xAA, 0xAA, 0xFF, 0xFF)});
    rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, fnt);

    debug_log_init();

    for (;;) {
        rdpq_attach(display_get(), NULL);
        
        rdpq_set_mode_standard();
        debug_log_draw();
        rdpq_detach_show();
    }

    sprite_free(test_sprite);
    test_sprite = NULL;

    joypad_close();
    dfs_close(dfs_handle);
    rdpq_close();
    display_close();

    return 0;
}