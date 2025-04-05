#include "../cosmoslib64/coslib.h"

float get_time_s()  { return (float)((double)get_ticks_ms() / 1000.0); }

int main(void)
{
    coslib_init(2, RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);
    
    model_cache_create(2);

    Stage* main_stage = malloc(sizeof(Stage));
    stage_init(main_stage, 0);
    main_stage->ambient_color = RGBA32(0x16, 0x11, 0x22, 0xFF);
    coslib_set_back_color(RGBA32(0x07, 0x05, 0x11, 0xFF));
    coslib_add_stage(main_stage);

    CachedModel* jevil_model = load_model_into_cache("rom:/mdl_jevil.t3dm", 0, false);
    CachedModel* TestCutscene = load_model_into_cache("rom:/mdl_jevil_scene_test.t3dm", 1, false);

    Actor* jevil = malloc(sizeof(Actor));
    actor_init(jevil, 0);
    stage_add_actor(main_stage, jevil, -1);

    Mesh3DM* jevil_mesh = malloc(sizeof(Mesh3DM));
    mesh3dm_create(main_stage, jevil_mesh, 0, 1);
    actor_add_module(jevil, (Module*)jevil_mesh, -1);

    StateM* jevil_state = malloc(sizeof(StateM));
    statem_create(jevil_state, 1, sizeof(AnimSt));
    jevil_state->main_skel = &jevil_mesh->skeletons[0];
    jevil_state->blend_skel = &jevil_mesh->skeletons[1];
    AnimSt* idle_state = animst_create(jevil_state, malloc(sizeof(AnimSt)), 0, 0, 0, jevil_model->model, "Dance");
    actor_add_module(jevil, (Module*)jevil_state, -1);

    Camera3DM* jevil_camera = malloc(sizeof(Camera3DM));
    camera3dm_create(jevil_camera);
    main_stage->camera = jevil_camera;
    jevil_camera->fov = T3D_DEG_TO_RAD(85.0f);
    jevil_camera->near = 1.f;
    actor_add_module(jevil, (Module*)jevil_camera, -1);
    Trans3DM* jevil_cam_trans = &jevil_camera->render.transform;
    jevil_cam_trans->position.y = 0.8f;
    jevil_cam_trans->position.z = 2.2f;
    trans3dm_update_matrix(jevil_cam_trans);

    DirLite3DM* test_light = malloc(sizeof(DirLite3DM));
    dirlite3dm_create(main_stage, test_light);
    actor_add_module(jevil, (Module*)test_light, -1);
    Trans3DM* test_light_trans = &test_light->render.transform;
    t3d_quat_rotate_euler(&test_light_trans->rotation, (float[3]){1, 0, 0}, T3D_DEG_TO_RAD(75.f));
    trans3dm_update_matrix(test_light_trans);

    float lastTime = get_time_s() - (1.0f / 60.0f);

    cossnd_init(32000, 4, 10);
    
    xm64player_t xm;
    xm64player_open(&xm, "rom:/trk_jevil_battle.xm64");
    xm64player_play(&xm, 2);

    wav64_t spin_sound;
    wav64_open(&spin_sound, "rom:/snd_hypnosis_ch1.wav64");

    for (uint32_t frame=0; ;++frame) {
        float newTime = get_time_s();
        float deltaTime = newTime - lastTime;
        lastTime = newTime;

        coslib_life(frame, deltaTime);

        /*if ((btn.a || btn.b ) && !TestCutsceneAnim.animation.isPlaying) {
            t3d_anim_set_playing(&TestCutsceneAnim.animation, true);
            t3d_anim_set_time(&TestCutsceneAnim.animation, 0.0f);
            jevil_mesh->oneshot = &TestCutsceneAnim;
            wav64_play(&spin_sound, 0);
        }*/

        coslib_draw(frame, deltaTime);
    }

    coslib_kill();

    return 0;
}