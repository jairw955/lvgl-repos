#include "main.h"

#define FRUIT_SIZE      150
#define MAX_FRUITS      5
#define GRAVITY         0.5f
#define INITIAL_LIVES   3
#define SPAWN_INTERVAL  50
#define PARTICLE_COUNT  8
#define PARTICLE_SIZE   50
#define PARTICLE_LIFETIME  500

typedef struct
{
    lv_obj_t *obj;
    float x_vel;
    float y_vel;
    float x_pos;
    float y_pos;
    bool active;
} Fruit;

typedef struct
{
    lv_obj_t *obj;
    float x_pos;
    float y_pos;
    float x_vel;
    float y_vel;
    uint32_t create_time;
    bool active;
    lv_color_t color;
} Particle;

static lv_obj_t *game_screen;
static lv_obj_t *score_label;
static lv_obj_t *lives_label;
static Fruit fruits[MAX_FRUITS];
static int score = 0;
static int lives = INITIAL_LIVES;
static lv_timer_t *game_timer;
static int spawn_counter = 0;
static lv_obj_t *restart_btn;
static int screen_width;
static int screen_height;

static Particle particles[MAX_FRUITS *PARTICLE_COUNT];
static uint32_t last_tick;

static void create_game_screen(void);
static void spawn_fruit(Fruit *fruit);
static void update_fruits(lv_timer_t *timer);

static void restart_game(lv_event_t *e)
{
    score = 0;
    lives = INITIAL_LIVES;
    spawn_counter = 0;

    lv_label_set_text_fmt(score_label, "score: %d", score);
    lv_label_set_text_fmt(lives_label, "lives: %d", lives);

    for (int i = 0; i < MAX_FRUITS; i++)
    {
        if (fruits[i].obj)
        {
            //unregister_hitbox(fruits[i].obj);
            lv_obj_del(fruits[i].obj);
            fruits[i].obj = NULL;
        }
        fruits[i].active = false;
    }

    for (int i = 0; i < MAX_FRUITS * PARTICLE_COUNT; i++)
    {
        if (particles[i].obj)
        {
            lv_obj_del(particles[i].obj);
            particles[i].obj = NULL;
        }
        particles[i].active = false;
    }

    lv_obj_add_flag(restart_btn, LV_OBJ_FLAG_HIDDEN);
    //unregister_hitbox(restart_btn);

    game_timer = lv_timer_create(update_fruits, 30, NULL);

    spawn_fruit(&fruits[0]);
}

static void init_particle_system(void)
{
    for (int i = 0; i < MAX_FRUITS * PARTICLE_COUNT; i++)
    {
        particles[i].obj = NULL;
        particles[i].active = false;
    }
    last_tick = lv_tick_get();
}

static void create_particles(float x, float y, lv_color_t color)
{
    for (int i = 0; i < MAX_FRUITS * PARTICLE_COUNT; i++)
    {
        if (!particles[i].active)
        {
            particles[i].obj = lv_obj_create(game_screen);
            lv_obj_set_size(particles[i].obj, PARTICLE_SIZE, PARTICLE_SIZE);
            lv_obj_set_style_radius(particles[i].obj, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(particles[i].obj, color, 0);

            particles[i].x_pos = x;
            particles[i].y_pos = y;
            float angle = (float)(lv_rand(0, 360)) * 3.14159f / 180.0f;
            float speed = (float)(lv_rand(5, 15));
            particles[i].x_vel = cosf(angle) * speed;
            particles[i].y_vel = sinf(angle) * speed;
            particles[i].create_time = lv_tick_get();
            particles[i].active = true;
            particles[i].color = color;

            lv_obj_set_pos(particles[i].obj, (lv_coord_t)x, (lv_coord_t)y);

            if (i % PARTICLE_COUNT == PARTICLE_COUNT - 1) break;
        }
    }
}

static void update_particles(void)
{
    uint32_t current_tick = lv_tick_get();
    uint32_t delta_time = current_tick - last_tick;
    last_tick = current_tick;

    for (int i = 0; i < MAX_FRUITS * PARTICLE_COUNT; i++)
    {
        if (particles[i].active)
        {
            particles[i].y_vel += GRAVITY * 0.5f;
            particles[i].x_pos += particles[i].x_vel;
            particles[i].y_pos += particles[i].y_vel;

            if (current_tick - particles[i].create_time > PARTICLE_LIFETIME)
            {
                particles[i].active = false;
                lv_obj_del(particles[i].obj);
                particles[i].obj = NULL;
                continue;
            }

            if (particles[i].obj)
            {
                lv_obj_set_pos(particles[i].obj,
                               (lv_coord_t)particles[i].x_pos,
                               (lv_coord_t)particles[i].y_pos);

                uint8_t alpha = 255 - (uint8_t)(((current_tick - particles[i].create_time) * 255) /
                                                PARTICLE_LIFETIME);
                lv_obj_set_style_bg_opa(particles[i].obj, alpha, 0);
            }
        }
    }
}

static void create_game_screen(void)
{
    screen_width = LV_HOR_RES;
    screen_height = LV_VER_RES;

    game_screen = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(game_screen);
    lv_obj_set_size(game_screen, screen_width, screen_height);
    lv_obj_clear_flag(game_screen, LV_OBJ_FLAG_SCROLLABLE);

    score_label = lv_label_create(game_screen);
    lv_label_set_text_fmt(score_label, "score: %d", score);
    lv_obj_align(score_label, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_set_style_text_color(score_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(score_label, &lv_font_montserrat_32, 0);

    lives_label = lv_label_create(game_screen);
    lv_label_set_text_fmt(lives_label, "lives: %d", lives);
    lv_obj_align_to(lives_label, score_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_obj_set_style_text_color(lives_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(lives_label, &lv_font_montserrat_32, 0);
}

static void fruit_touch(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    int i = 0;

    if (code == LV_EVENT_PRESSED)
    {
        // 检查所有水果的碰撞
        for (i = 0; i < MAX_FRUITS; i++)
        {
            if (fruits[i].obj == obj)
            {
                break;
            }
        }
        if (i == MAX_FRUITS)
        {
            return;
        }
        // 获取水果当前位置和颜色
        lv_color_t fruit_color = lv_obj_get_style_bg_color(fruits[i].obj, 0);
        float fruit_x = fruits[i].x_pos;
        float fruit_y = fruits[i].y_pos;

        // 创建切割效果
        create_particles(fruit_x + FRUIT_SIZE / 2, fruit_y + FRUIT_SIZE / 2, fruit_color);

        // 更新分数
        score += 10;
        lv_label_set_text_fmt(score_label, "Score: %d", score);

        //unregister_hitbox(fruits[i].obj);
        // 删除水果
        fruits[i].active = false;
        lv_obj_del(fruits[i].obj);
        fruits[i].obj = NULL;
    }
}
// 生成新水果
static void spawn_fruit(Fruit *fruit)
{
    if (fruit->obj != NULL)
    {
        //unregister_hitbox(fruit->obj);
        lv_obj_del(fruit->obj);
    }

    fruit->obj = lv_obj_create(game_screen);
    lv_obj_add_event_cb(fruit->obj, fruit_touch, LV_EVENT_ALL, NULL);
    //register_hitbox(fruit->obj);
    lv_obj_set_size(fruit->obj, FRUIT_SIZE, FRUIT_SIZE);
    lv_obj_clear_flag(fruit->obj, LV_OBJ_FLAG_SCROLLABLE);

    // 随机设置初始位置和速度
    fruit->x_pos = lv_rand(FRUIT_SIZE, screen_width - FRUIT_SIZE);
    fruit->y_pos = screen_height - FRUIT_SIZE;
    fruit->x_vel = -3 + (float)lv_rand(0, 6);
    fruit->y_vel = -20 - (float)lv_rand(0, 5); // 向上的初始速度

    // 随机颜色
    lv_color_t colors[] =
    {
        lv_color_hex(0xFF0000), // 红色
        lv_color_hex(0xFFA500), // 橙色
        lv_color_hex(0xFFFF00), // 黄色
        lv_color_hex(0x00FF00)  // 绿色
    };
    lv_obj_set_style_bg_color(fruit->obj, colors[lv_rand(0, 3)], 0);
    lv_obj_set_style_bg_opa(fruit->obj, LV_OPA_100, 0);
    lv_obj_set_style_radius(fruit->obj, LV_RADIUS_CIRCLE, 0);

    fruit->active = true;
    lv_obj_set_pos(fruit->obj, (lv_coord_t)fruit->x_pos, (lv_coord_t)fruit->y_pos);
    //LV_LOG_USER("spawn_fruit: %f %f", fruit->x_pos, fruit->y_pos);
    //lv_obj_center(fruit->obj);
}

// 更新水果位置
static void update_fruits(lv_timer_t *timer)
{
    // 添加粒子系统更新
    update_particles();
    // 增加计数器
    spawn_counter++;

    // 确保始终有水果在屏幕上
    bool has_active_fruit = false;

    for (int i = 0; i < MAX_FRUITS; i++)
    {
        if (fruits[i].active)
        {
            has_active_fruit = true;
            // 更新位置
            fruits[i].y_vel += GRAVITY;
            fruits[i].x_pos += fruits[i].x_vel;
            fruits[i].y_pos += fruits[i].y_vel;
            // 检查边界
            if (fruits[i].x_pos < 0 || fruits[i].x_pos > screen_width - FRUIT_SIZE)
            {
                fruits[i].x_vel = -fruits[i].x_vel;
                fruits[i].x_pos = fruits[i].x_pos < 0 ? 0 : screen_width - FRUIT_SIZE;
            }
            // 检查是否落地
            if (fruits[i].y_pos > screen_height)
            {
                fruits[i].active = false;
                //unregister_hitbox(fruits[i].obj);
                lv_obj_del(fruits[i].obj);
                fruits[i].obj = NULL;
                lives--;
                lv_label_set_text_fmt(lives_label, "lives: %d", lives);
                if (lives <= 0)
                {
                    // 游戏结束处理
                    lv_timer_del(game_timer);
                    lv_obj_t *game_over = lv_label_create(game_screen);
                    lv_label_set_text(game_over, "Game Over!");
                    lv_obj_align(game_over, LV_ALIGN_CENTER, 0, -20);  // 调整位置以适应重启按钮
                    lv_obj_set_style_text_color(game_over, lv_color_white(), 0);
                    // 显示重启按钮
                    lv_obj_clear_flag(restart_btn, LV_OBJ_FLAG_HIDDEN);
                    //register_hitbox(restart_btn);
                    return;
                }
            }
            // 更新位置
            if (fruits[i].obj)
            {
                lv_obj_set_pos(fruits[i].obj, (lv_coord_t)fruits[i].x_pos, (lv_coord_t)fruits[i].y_pos);
                //LV_LOG_USER("update_fruits: %f %f", fruits[i].x_pos, fruits[i].y_pos);
            }
        }
    }
    // 如果没有活跃的水果或达到生成间隔，尝试生成新水果
    if ((!has_active_fruit || spawn_counter >= SPAWN_INTERVAL) && lives > 0)
    {
        for (int i = 0; i < MAX_FRUITS; i++)
        {
            if (!fruits[i].active)
            {
                spawn_fruit(&fruits[i]);
                spawn_counter = 0;
                break;
            }
        }
    }
}

// 游戏初始化
void init_fruit_ninja(void)
{
    // 初始化随机数生成器
    srand(time(NULL));
    // 创建游戏屏幕
    create_game_screen();

    // 初始化粒子系统
    init_particle_system();

    // 创建重启按钮
    restart_btn = lv_btn_create(game_screen);
    lv_obj_set_size(restart_btn, 120, 50);
    lv_obj_align(restart_btn, LV_ALIGN_CENTER, 0, 50);
    lv_obj_add_flag(restart_btn, LV_OBJ_FLAG_HIDDEN);  // 初始时隐藏
    lv_obj_add_event_cb(restart_btn, restart_game, LV_EVENT_CLICKED, NULL);

    // 创建按钮标签
    lv_obj_t *btn_label = lv_label_create(restart_btn);
    lv_label_set_text(btn_label, "Restart");
    lv_obj_center(btn_label);

    // 初始化水果数组
    for (int i = 0; i < MAX_FRUITS; i++)
    {
        fruits[i].obj = NULL;
        fruits[i].active = false;
    }
    spawn_fruit(&fruits[0]);
    // 创建游戏定时器
    game_timer = lv_timer_create(update_fruits, 30, NULL);
}
