#include "../cosmoslib64/coscam.h"
#include "../cosmoslib64/coslite.h"
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
    model_cache_create(2);
    load_model_into_cache("rom:/mdl_jevil.t3dm", "Jevil");
    CachedModel* TestCutscene = load_model_into_cache("rom:/mdl_jevil_scene_test.t3dm", "TestCutscene");
    SkinnedAnimation TestCutsceneAnim;
    TestCutsceneAnim.animation = t3d_anim_create(TestCutscene->model, "TestCutscene");
    t3d_anim_set_looping(&TestCutsceneAnim.animation, false);
    t3d_anim_set_playing(&TestCutsceneAnim.animation, false);

    Actor* jevil = malloc(sizeof(Actor));
    actor_init(jevil, "ENEMYJevil");

    Mesh3DM* jevil_mesh = malloc(sizeof(Mesh3DM));
    mesh3dm_create(jevil_mesh, "Jevil", 1, 1);
    jevil_mesh->animations[0].animation = t3d_anim_create(jevil_mesh->model->model, "Dance");
    strcpy(jevil_mesh->animations[0].name, "Dance");
    t3d_anim_attach(&jevil_mesh->animations[0].animation, &jevil_mesh->skeletons[0]);
    t3d_anim_attach(&TestCutsceneAnim.animation, &jevil_mesh->skeletons[0]);
    jevil_mesh->looping = &jevil_mesh->animations[0];
    actor_add_module(jevil, (Module*)jevil_mesh, false);
    Trans3DM* jevil_trans = &jevil_mesh->render.transform;

    Camera3DM* jevil_camera = malloc(sizeof(Camera3DM));
    camera3dm_create(jevil_camera, "JevilCam");
    actor_add_module(jevil, (Module*)jevil_camera, false);
    Trans3DM* jevil_cam_trans = &jevil_camera->render.transform;
    jevil_cam_trans->position.y = 50.f;
    jevil_cam_trans->position.z = 140.f;
    trans3dm_update_matrix(jevil_cam_trans);

    DirLite3DM* test_light = malloc(sizeof(DirLite3DM));
    dirlite3dm_create(test_light, "TestLight");
    actor_add_module(jevil, (Module*)test_light, false);
    Trans3DM* test_light_trans = &test_light->render.transform;
    test_light_trans->rotation.x = T3D_DEG_TO_RAD(-75.f);
    trans3dm_update_matrix(test_light_trans);

    uint8_t colorAmbient[4] = {0x16, 0x11, 0x22, 0xFF};

    rdpq_font_t* fnt = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
    rdpq_font_style(fnt, 1, &(rdpq_fontstyle_t){RGBA32(0xAA, 0xAA, 0xFF, 0xFF)});
    rdpq_text_register_font(FONT_BUILTIN_DEBUG_MONO, fnt);

    float lastTime = get_time_s() - (1.0f / 60.0f);

    audio_init(44100, 4);
    mixer_init(10);
    xm64player_t xm;
    xm64player_open(&xm, "rom:/trk_jevil_battle.xm64");
    xm64player_play(&xm, 2);

    wav64_t spin_sound;
    wav64_open(&spin_sound, "rom:/snd_hypnosis_ch1.wav64");

    for (uint32_t frame=0; ;++frame) {
        joypad_poll();
        if (audio_can_write()) {
            // Select an audio buffer that we can write to
            short *buf = audio_write_begin();
            // Write to the audio buffer from the mixer
            mixer_poll(buf, audio_get_buffer_length());
            // Tell the audio system that the buffer has
            // been filled and is ready for playback
            audio_write_end();
        }
        joypad_inputs_t joypad = joypad_get_inputs(JOYPAD_PORT_1);
        joypad_buttons_t btn = joypad_get_buttons_pressed(JOYPAD_PORT_1);

        if ((btn.a || btn.b ) && !TestCutsceneAnim.animation.isPlaying) {
            t3d_anim_set_playing(&TestCutsceneAnim.animation, true);
            t3d_anim_set_time(&TestCutsceneAnim.animation, 0.0f);
            jevil_mesh->oneshot = &TestCutsceneAnim;
            wav64_play(&spin_sound, 0);
        }

        float newTime = get_time_s();
        float deltaTime = newTime - lastTime;
        lastTime = newTime;

        uint32_t matrixIdx = frame % display_get_num_buffers();

        coslite_new_frame();

        /*jevil_trans->rotation.y += deltaTime * 2.f;
        trans3dm_update_matrix(jevil_trans);*/

        // Update stages here
        actor_life(jevil, deltaTime);

        rdpq_attach(display_get(), display_get_zbuf());
        t3d_frame_start();

        // Draw Camera here
        jevil_camera->render.draw(&jevil_camera->render, deltaTime, matrixIdx);

        t3d_screen_clear_color(RGBA32(0x07, 0x05, 0x11, 0xFF));
        t3d_screen_clear_depth();

        t3d_light_set_count(coslite_get_count());
        // Draw lights here
        test_light->render.draw(&test_light->render, deltaTime, matrixIdx);

        t3d_light_set_ambient(colorAmbient);

        // Draw meshes here
        jevil_mesh->render.draw(&jevil_mesh->render, deltaTime, matrixIdx);

        rdpq_sync_pipe();

        rdpq_set_mode_standard();
        
        /* Code we can use to overlay colors on screen
        rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
        rdpq_mode_combiner(RDPQ_COMBINER_FLAT);
        rdpq_set_prim_color(RGBA32(0xFF,0xFF,0xFF,0xFF));
        rdpq_fill_rectangle(0, 0, display_get_width(), display_get_height());*/

        rdpq_detach_show();
    }

    actor_kill(jevil);
    t3d_destroy();
    joypad_close();
    rdpq_close();
    display_close();

    return 0;
}