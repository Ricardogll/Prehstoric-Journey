#include "p2Defs.h"
#include "p2Log.h"
#include "Player.h"
#include "p2List.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Input.h"
#include "j1Map.h"

#include "j1Collision.h"
#include "j1Audio.h"
#include "j1Window.h"
#include "j1FadeToBlack.h"



Player::Player(int x, int y, pugi::xml_node& config, EntityTypes type) : Entity(x, y, type)
{
	pugi::xml_node node_player = config.child("player");

	Awake(node_player);
	Start();

}

Player::~Player()
{

}

bool Player::Awake(pugi::xml_node& config)
{
	LOG("Loading Player");
	bool ret = true;

	if (config != NULL) {
		LoadVariablesXML(config);

		pugi::xml_node animations = config.child("animations");
		SetAnimations(animations.child("idle").child("animation"), idle);
		idle.speed = animations.child("idle").attribute("speed").as_float();
		idle.loop = animations.child("idle").attribute("loop").as_bool();

		SetAnimations(animations.child("run").child("animation"), run);
		run.speed = animations.child("run").attribute("speed").as_float();
		run.loop = animations.child("run").attribute("loop").as_bool();

		SetAnimations(animations.child("jump").child("animation"), jump);
		jump.speed = animations.child("jump").attribute("speed").as_float();
		jump.loop = animations.child("jump").attribute("loop").as_bool();

		SetAnimations(animations.child("climb").child("animation"), climbing);
		climbing.speed = animations.child("climb").attribute("speed").as_float();
		climbing.loop = animations.child("climb").attribute("loop").as_bool();

		SetAnimations(animations.child("climbidle").child("animation"), climbing_idle);
		climbing_idle.speed = animations.child("climbidle").attribute("speed").as_float();
		climbing_idle.loop = animations.child("climbidle").attribute("loop").as_bool();

		SetAnimations(animations.child("attack").child("animation"), attack);
		attack.speed = animations.child("attack").attribute("speed").as_float();
		attack.loop = animations.child("attack").attribute("loop").as_bool();

		SetAnimations(animations.child("death").child("animation"), death);
		death.speed = animations.child("death").attribute("speed").as_float();
		death.loop = animations.child("death").attribute("loop").as_bool();
	}

	return ret;
}

bool Player::Start()
{


	LOG("starting player");
	bool ret = true;

	jump_fx = App->audio->LoadFx(jump_fx_folder.GetString());
	lose_fx = App->audio->LoadFx(lose_fx_folder.GetString());
	hit_fx = App->audio->LoadFx(hit_fx_folder.GetString());
	chicken_sound = App->audio->LoadFx(chicken_fx_folder.GetString());

	texture = App->tex->Load(spritesheet.GetString());
	state = IDLE;
	position.x = App->map->spawn_pos.x;
	position.y = App->map->spawn_pos.y;
	entity_x_dir = RIGHT;

	last_saved_pos.x = App->map->spawn_pos.x;
	last_saved_pos.y = App->map->spawn_pos.y;

	speed = { 0.0f,0.0f };
	acceleration = { 0.0f, 0.0f };

	player_rect = { (int)position.x + collider_offset.x,(int)position.y + collider_offset.y,collider_dimensions.x,collider_dimensions.y };
	collider = App->collision->AddCollider(player_rect, COLLIDER_PLAYER, (j1Module*)App->entities);

	return ret;
}

void Player::Update(float dt)
{
	dt_current = dt;

	AnimationsApplyDt();

	if (!App->scene->on_main_menu) {

		App->render->camera.x = -position.x - player_rect.w / 2 + App->win->width / 2;
		if (App->render->camera.x > start_map)
			App->render->camera.x = start_map;
		if (App->render->camera.x < limit_map)
			App->render->camera.x = limit_map;


		

		if (dt_current != 0) {

			if (App->input->GetKey(SDL_SCANCODE_F10) == KEY_DOWN)
				god_mode = !god_mode;

			if (App->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN) {
				if (App->map->debug_camera_culling == 0)
					App->map->debug_camera_culling = 1;
				else
					App->map->debug_camera_culling = 0;
			}


			key_w_pressed = false;
			if (App->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
			{
				key_w_pressed = true;
			}

			if (colliding_with_liana == false) {

				on_liana = false;
			}
			if (on_liana) {
				if (App->input->GetKey(SDL_SCANCODE_D) == KEY_UP)
				{
					state = LIANA_IDLE;
				}
				else if (App->input->GetKey(SDL_SCANCODE_A) == KEY_UP)
				{
					state = LIANA_IDLE;
				}
				else if (App->input->GetKey(SDL_SCANCODE_S) == KEY_UP)
				{
					state = LIANA_IDLE;
				}
				else if (App->input->GetKey(SDL_SCANCODE_W) == KEY_UP)
				{
					state = LIANA_IDLE;
				}


				if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
				{
					position.x += liana_speed * dt_current;
					entity_x_dir = RIGHT;
					state = LIANA;
				}
				if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
				{
					position.x -= liana_speed * dt_current;
					entity_x_dir = LEFT;
					state = LIANA;
				}
				if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
				{
					position.y += liana_speed * dt_current;
					state = LIANA;
				}
				if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
				{
					position.y -= liana_speed * dt_current;
					state = LIANA;
				}



				if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && god_mode == false) {
					jump.Reset();
					state = JUMP;
					speed.y = jump_force_liana;

					jumping = true;


					on_ground = false;
					just_landed = false;
					on_liana = false;

					if (entity_x_dir == LEFT) {
						speed.x = -max_speed_x;
						acceleration.x = -max_acc_x;
					}
					if (entity_x_dir == RIGHT) {
						speed.x = max_speed_x;
						acceleration.x = max_acc_x;
					}

					App->audio->PlayFx(jump_fx);
				}

			}
			else {

				if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT && attacking == false)
				{

					acceleration.x += acceleration_x;
					if (jumping == false) {
						state = RUN;
					}
					else {
						state = JUMP;
					}


					if (entity_x_dir == LEFT)
						run.Reset();

					entity_x_dir = RIGHT;
					key_d_pressed = true;
				}
				else if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT && attacking == false)
				{
					acceleration.x -= acceleration_x;
					if (jumping == false) {
						state = RUN;
					}
					else {
						state = JUMP;
					}

					if (entity_x_dir == RIGHT) {
						run.Reset();

						entity_x_dir = LEFT;
					}
				}

				if (App->input->GetKey(SDL_SCANCODE_D) == KEY_UP)
				{
					if (jumping == false) {
						state = IDLE;
					}
					acceleration.x = 0.0f;
					speed.x = 0.0f;
				}

				if (App->input->GetKey(SDL_SCANCODE_A) == KEY_UP)
				{
					if (jumping == false) {
						state = IDLE;
					}
					acceleration.x = 0.0f;
					speed.x = 0.0f;

				}

				if (App->input->GetKey(SDL_SCANCODE_C) == KEY_DOWN && attacking == false && on_ground == true)
				{
					App->audio->PlayFx(hit_fx);
					attack.Reset();
					if (entity_x_dir == RIGHT)
						attack_player_rect = { (int)position.x + collider_attack_offset.x + collider->rect.w, (int)position.y - collider_attack_offset.y, collider_attack_dimensions.x, collider_attack_dimensions.y };
					else
						attack_player_rect = { (int)position.x + collider_attack_offset.x - collider->rect.w + collider_attack_offset.y, (int)position.y - collider_attack_offset.y, collider_attack_dimensions.x, collider_attack_dimensions.y };
					player_attack = App->collision->AddCollider(attack_player_rect, COLLIDER_PLAYER_ATTACK, (j1Module*)App->entities);
					start_attack = SDL_GetTicks();
					attacking = true;
				}

				if (start_attack + attack_time < SDL_GetTicks() && attacking == true)
				{
					state = IDLE;
					App->collision->EraseCollider(player_attack);
					start_attack = 0;
					attacking = false;
				}

				if (attacking == true)
				{
					state = ATTACK;
					acceleration.x = 0.0f;
					speed.x = 0.0f;
				}

				key_d_pressed = false;

			}


			if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && on_ground && !god_mode && !attacking)
			{

				jump.Reset();
				state = JUMP;
				speed.y = jump_force;

				jumping = true;


				on_ground = false;
				just_landed = false;

				App->audio->PlayFx(jump_fx);
			}



			if (on_ground == false) {
				acceleration.y = gravity;
				just_landed = false;
			}
			else
			{
				on_ground = true;
				jumping = false;

				if (just_landed == false) {

					just_landed = true;

					if (speed.x != 0.0f)
						state = RUN;
					else
						state = IDLE;
				}
			}

			if (god_mode == true)
			{
				acceleration.y = 0.0f;
				if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
				{
					acceleration.x += acceleration_x;
				}
				if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
				{
					acceleration.x -= acceleration_x;
				}
				if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
				{

					speed.y = acceleration_x;
				}

				if (App->input->GetKey(SDL_SCANCODE_S) == KEY_UP)
				{
					speed.y = 0.0f;
				}
				if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
				{
					speed.y = -acceleration_x;
				}
				if (App->input->GetKey(SDL_SCANCODE_W) == KEY_UP)
				{
					speed.y = 0.0f;
				}
			}

			if (on_liana)
				acceleration.y = 0.0f;

			position.x += speed.x * dt_current;
			position.y += speed.y * dt_current;
			speed.x += acceleration.x * dt_current;
			speed.y += acceleration.y * dt_current;

			colliding_with_liana = false;

			if (speed.x > max_speed_x)
				speed.x = max_speed_x;

			if (acceleration.x > max_acc_x)
				acceleration.x = max_acc_x;


			if (speed.x < -max_speed_x)
				speed.x = -max_speed_x;

			if (acceleration.x < -max_acc_x)
				acceleration.x = -max_acc_x;




			collider->SetPos(position.x + collider_offset.x, position.y + collider_offset.y);
			//player_attack->SetPos(position.x + collider_offset.x, position.y + collider_offset.y);
			player_rect = { (int)position.x + collider_offset.x, (int)position.y + collider_offset.y, collider_dimensions.x, collider_dimensions.y };

		}
	}

}



bool Player::PostUpdate()
{
	
	return true;
}

bool Player::CleanUp()
{
	LOG("Destroying player");
	bool ret = true;

	

	return ret;
}


bool Player::Load(pugi::xml_node& node)
{
	

	last_saved_pos.x = node.child("player_position").attribute("x").as_int();
	last_saved_pos.y = node.child("player_position").attribute("y").as_int();
	position.x = last_saved_pos.x;
	position.y = last_saved_pos.y;

	return true;
}

bool Player::Save(pugi::xml_node& node) const
{
	
	pugi::xml_node position_node = node.append_child("player_position");
	position_node.append_attribute("x").set_value(position.x);
	position_node.append_attribute("y").set_value(position.y);
	return true;
}


void Player::Draw()
{

	switch (state)
	{
	case IDLE:
		current_animation = &idle;
		break;

	case RUN:
		current_animation = &run;
		break;

	case JUMP:
		current_animation = &jump;
		break;

	case LIANA:
		current_animation = &climbing;
		break;

	case LIANA_IDLE:
		current_animation = &climbing_idle;
		break;

	case ATTACK:
		current_animation = &attack;
		break;

	default:
		current_animation = &idle;


	}
	//SDL_Rect r = current_animation->GetCurrentFrame();
	if (entity_x_dir == LEFT && on_liana == false) {
		App->render->Blit(texture, (int)position.x + App->render->camera.x - collider->rect.w / 2, (int)position.y, &(current_animation->GetCurrentFrame()), NULL, NULL, SDL_FLIP_HORIZONTAL, 0, 0);
	}
	else {
		App->render->Blit(texture, position.x, position.y, &(current_animation->GetCurrentFrame()));
	}
}

void Player::OnCollision(Collider* c1, Collider* c2) {




	if (App->map->data.layers.end != nullptr) {


		MapLayer* layer_coll = App->map->data.layers.end->data;
		iPoint down_right = App->map->WorldToMap(player_rect.x + player_rect.w - collider_offset.x, player_rect.y + player_rect.h);
		iPoint down_left = App->map->WorldToMap(player_rect.x - collider_offset.x, player_rect.y + player_rect.h);

		int down_right_gid = 0;
		if (down_right.x >= 0 && down_right.x <= App->map->data.width && down_right.y >= 0 && down_right.y <= App->map->data.height)//If tile is outside the map dont ->Get(...) or error
			down_right_gid = layer_coll->Get(down_right.x, down_right.y);

		int down_left_gid = 0;
		if (down_left.x >= 0 && down_left.x <= App->map->data.width && down_left.y >= 0 && down_left.y <= App->map->data.height)
			down_left_gid = layer_coll->Get(down_left.x, down_left.y);

		iPoint up_right = App->map->WorldToMap(player_rect.x + player_rect.w - collider_offset.x, player_rect.y);
		iPoint up_left = App->map->WorldToMap(player_rect.x , player_rect.y);

		int up_right_gid = 0;
		if (up_right.x >= 0 && up_right.x <= App->map->data.width && up_right.y >= 0 && up_right.y <= App->map->data.height)
			up_right_gid = layer_coll->Get(up_right.x, up_right.y);

		int up_left_gid = 0;
		if (up_left.x >= 0 && up_left.x <= App->map->data.width && up_left.y >= 0 && up_left.y <= App->map->data.height)
			up_left_gid = layer_coll->Get(up_left.x, up_left.y);



		if (c2->type == COLLIDER_WALL || c2->type == COLLIDER_LEDGE)
		{ //Using "(int)speed" to see if in the next update player will be inside the wall. Using +1 in case the float is shortened and we end up going inside the wall.
			//Touching floor
			if (c1->rect.y + c1->rect.h + (int)speed.y * dt_current + 1 > c2->rect.y && on_ground == false && c1->rect.y < c2->rect.y && c1->rect.y - c2->rect.y < -c2->rect.h/2 &&(down_right_gid == 48 || down_right_gid == 63 || down_right_gid == 62 || down_right_gid == 198 || down_right_gid == 213 || down_right_gid == 212) && (down_left_gid == 48 || down_left_gid == 63 || down_left_gid == 62 || down_left_gid == 198 || down_left_gid == 213 || down_left_gid == 212)) {

				acceleration.y = 0.0f;
				speed.y = 0.0f;
				
				position.y = c2->rect.y - c1->rect.h - collider_offset.y + 1;
				on_ground = true;
				
			}
			else {

				//Touching ceiling
				if (c1->rect.y + (int)speed.y * dt_current - 1 <= c2->rect.y + c2->rect.h && on_ground == false && c1->rect.y + c1->rect.h > c2->rect.y + c2->rect.h && (up_right_gid == 48 || up_right_gid == 63 || up_right_gid == 62 || up_right_gid == 198 || up_right_gid == 213 || up_right_gid == 212) && (up_left_gid == 48 || up_left_gid == 63 || up_left_gid == 62 || up_left_gid == 198 || up_left_gid == 213 || up_left_gid == 212)) {
					if (speed.y < 0.0f) {
						speed.y = -speed.y;
					}
					acceleration.y = gravity;
					position.y = c2->rect.y + c2->rect.h + 1;

				}

				//Touching left
				if (c1->rect.x + (int)speed.x * dt_current - 1 < c2->rect.x + c2->rect.w && (c1->rect.y > c2->rect.y || c1->rect.y > c2->rect.y - c1->rect.h * knee_height) && entity_x_dir == LEFT && c1->rect.x > c2->rect.x) {

					acceleration.x = 0.0f;
					speed.x = 0.0f;
					if (!on_ground)
						position.x++;


				}//Touching right
				else if (c1->rect.x + c1->rect.w + (int)speed.x * dt_current + 1 > c2->rect.x && (c1->rect.y > c2->rect.y || c1->rect.y > c2->rect.y - c1->rect.h * knee_height) && entity_x_dir == RIGHT && abs(c1->rect.x) < abs(c2->rect.x)) {

					acceleration.x = 0.0f;
					speed.x = 0.0f;
					if (!on_ground)
						position.x--;

				}
			}

			if (c2->type == COLLIDER_LEDGE && c1->rect.x + (int)speed.x * dt_current + 1 > c2->rect.x + c2->rect.w && entity_x_dir == RIGHT && (down_right_gid == 62 || down_right_gid== 212|| down_right_gid == 0 || down_left_gid == 61 || down_right_gid == 211)) {
 				on_ground = false;
				
			}
			else if (c2->type == COLLIDER_LEDGE && c1->rect.x + c1->rect.w + (int)speed.x * dt_current - 1 < c2->rect.x && entity_x_dir == LEFT && (down_left_gid == 62 || down_right_gid == 212 || down_left_gid == 0 || down_left_gid == 61 || down_right_gid == 211)) {
				on_ground = false;
				
			}
		}




		//****LIANAS
		if (c2->type == COLLIDER_LIANA) {
			colliding_with_liana = true;
		}


		if (c2->type == COLLIDER_LIANA && key_w_pressed) {
			acceleration = { 0.0f,0.0f };
			speed = { 0.0f,0.0f };
			on_liana = true;
		}

		//****** LEVEL END

		if (c2->type == COLLIDER_LEVEL_END && change_map == false) {
			change_map = true;
			if (App->scene->curr_map == 1) {
				App->scene->c_score_2nd_lvl = App->scene->c_score;
				App->scene->score_2nd_lvl = App->scene->score;
				App->scene->time_2nd_lvl = App->scene->time + App->scene->timer.ReadSec();
			}
		}
		

		if ((c2->type == COLLIDER_DEAD || c2->type == COLLIDER_ENEMY) && player_died == false && god_mode == false && App->scene->on_main_menu==false) {
			player_died = true;
			App->audio->PlayFx(lose_fx);
			speed = { 0.0f,0.0f };
			acceleration = { 0.0f,0.0f };
			App->scene->lifes--;
			if (App->scene->lifes < 0 || App->scene->curr_map == 1) {
				App->scene->c_score = 0;
				App->scene->score = 0;
				App->scene->multiplier = 1;
				App->scene->time = 0.0f;
				App->scene->timer.Start();
			}
			else {
				App->scene->c_score = App->scene->c_score_2nd_lvl;
				App->scene->score = App->scene->score_2nd_lvl;
				App->scene->time = App->scene->time_2nd_lvl;
				App->scene->timer.Start();
				App->scene->multiplier = 1;
				if (App->scene->c_score >= 5 && App->scene->c_score < 10)
					App->scene->multiplier = 2;
				else if (App->scene->c_score >= 10)
					App->scene->multiplier = 3;
			}
			
			position.x = App->map->spawn_pos.x;
			position.y = App->map->spawn_pos.y;
		}


		if (c2->type == COLLIDER_COLLECTABLE && App->scene->on_main_menu == false) {
			App->audio->PlayFx(chicken_sound);
		}

	}

}


void Player::LoadVariablesXML(const pugi::xml_node& player_node) {

	pugi::xml_node variables = player_node.child("variables");
	
	limit_map = variables.child("limit_map").attribute("value").as_int();
	App->scene->limit_map = limit_map;
	gravity = variables.child("gravity").attribute("value").as_float();
	acceleration_x = variables.child("acceleration_x").attribute("value").as_float();
	liana_speed = variables.child("liana_speed").attribute("value").as_float();
	max_acc_x = variables.child("max_acc_x").attribute("value").as_float();
	max_speed_x = variables.child("max_speed_x").attribute("value").as_float();
	jump_force = variables.child("jump_force").attribute("value").as_float();
	jump_force_liana = variables.child("jump_force_liana").attribute("value").as_float();
	collider_offset.x = variables.child("collider_offset").attribute("x").as_int();
	collider_offset.y = variables.child("collider_offset").attribute("y").as_int();
	collider_attack_offset.x = variables.child("collider_attack_offset").attribute("x").as_int();
	collider_attack_offset.y = variables.child("collider_attack_offset").attribute("y").as_int();
	collider_dimensions.x = variables.child("collider_dimensions").attribute("x").as_int();
	collider_dimensions.y = variables.child("collider_dimensions").attribute("y").as_int();
	collider_attack_dimensions.x = variables.child("collider_attack_dimensions").attribute("x").as_int();
	collider_attack_dimensions.y = variables.child("collider_attack_dimensions").attribute("y").as_int();
	spritesheet = variables.child("spritesheet").attribute("location").as_string();
	jump_fx_folder = variables.child("jump_fx_folder").attribute("location").as_string();
	lose_fx_folder = variables.child("lose_fx_folder").attribute("location").as_string();
	hit_fx_folder = variables.child("hit_fx_folder").attribute("location").as_string();
	chicken_fx_folder = variables.child("chicken_fx_folder").attribute("location").as_string();
	attack_time = variables.child("attack_time").attribute("value").as_int();
	knee_height = variables.child("knee_height").attribute("value").as_float();
	

}

void Player::AnimationsApplyDt() {

	if (anim_speed_flag == false) {
		 idle_anim_speed = idle.speed;
		 run_anim_speed = run.speed;
		 jump_anim_speed = jump.speed;
		 climbing_anim_speed = climbing.speed;
		 attack_anim_speed = attack.speed;

		 anim_speed_flag = true;
	}
	else
	{
		idle.speed = idle_anim_speed * dt_current;
		run.speed = run_anim_speed * dt_current;
		jump.speed = jump_anim_speed * dt_current;
		climbing.speed = climbing_anim_speed * dt_current;
		attack.speed = attack_anim_speed * dt_current;
	}
	
}


void Player::SetMovementZero() {

	speed = { 0.0f,0.0f };
	acceleration = { 0.0f,0.0f };

}