#include "../cosmoslib64/cosmesh.h"
#include "../cosmoslib64/cosdebug.h"

float get_time_s()  { return (float)((double)get_ticks_ms() / 1000.0); }

int main(void)
{
    debug_init_isviewer();
    debug_init_usblog();
    dfs_init(DFS_DEFAULT_LOCATION);
    asset_init_compression(2);
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);
    joypad_init();

    rdpq_init();
    t3d_init((T3DInitParams){});
    cosmesh_init();
    model_cache_create(1);
    load_model_into_cache("rom:/mdl_jevil.t3dm", "Jevil");
    
    T3DViewport viewport = t3d_viewport_create();
    
    T3DVec3 camPos    = {{0, 0, 140}};
    T3DVec3 camTarget = {{0, 50, 0}};

    T3DVec3 lightDirVec = {{1.0f, 1.0f, 1.0f}};
    t3d_vec3_norm(&lightDirVec);

    uint8_t colorAmbient[4] = {0xFF, 0xFF, 0xFF, 0xFF};

    Actor* jevil = malloc(sizeof(Actor));
    actor_init(jevil, "ENEMYJevil");
    Module* jevil_mesh = mesh3D_module_create("Jevil");
    actor_add_module(jevil, jevil_mesh, false);
    module_init(jevil_mesh);
    RenderableModule* jevil_render = (RenderableModule*)jevil_mesh->data;
    Trans3DModule* jevil_trans = ((Mesh3DModule*)jevil_render->data)->transform;

    rdpq_font_t* fnt = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
    rdpq_font_style(fnt, 1, &(rdpq_fontstyle_t){RGBA32(0xAA, 0xAA, 0xFF, 0xFF)});
    rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, fnt);

    color_t test_light = (color_t){0xFF, 0x00, 0x00, 0xFF};

    float lastTime = get_time_s() - (1.0f / 60.0f);

    for (uint32_t frame=0; ;++frame) {
        float newTime = get_time_s();
        float deltaTime = newTime - lastTime;
        lastTime = newTime;

        uint32_t matrixIdx = frame % display_get_num_buffers();

        t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(85.0f), 10.0f, 150.0f);
        t3d_viewport_look_at(&viewport, &camPos, &camTarget, &(T3DVec3){{0,1,0}});

        jevil_trans->rotation.y += deltaTime * 1.f;
        trans3D_update_matrix(jevil_trans);

        actor_life(jevil, deltaTime);

        rdpq_attach(display_get(), display_get_zbuf());
        t3d_frame_start();

        t3d_viewport_attach(&viewport);
        t3d_screen_clear_color(RGBA32(140, 227, 237, 0xFF));
        t3d_screen_clear_depth();
        t3d_light_set_count(1);
        t3d_light_set_directional(0, &test_light.r, &(T3DVec3){{ 1.0f,  1.0f, 0.0f }});

        rdpq_set_prim_color((color_t){0x00, 0x00, 0x00, 0xFF});
        t3d_light_set_ambient(colorAmbient);

        jevil_render->draw(jevil_render, deltaTime, matrixIdx);

        rdpq_sync_pipe();

        rdpq_detach_show();
    }

    t3d_destroy();
    joypad_close();
    rdpq_close();
    display_close();

    return 0;
}