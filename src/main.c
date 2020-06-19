
#include "utils.h"
#include "window.h"
#include "dbgmsg.h"
#include "random.h"
#include "shaders.h"
#include "universe.h"
#include "settings.h"
#include <math.h>
#include <stdio.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>

int main(int argc, const char** argv)
{
	(void)argc; (void)argv; /* Unused for now... */

	if (init_g_graphics() != 0)
	{
		return -1;
	}
	enable_opengl_dbgmsg();

	GLuint vao_id;
	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	shprog_build_all();

	g_rg = rg_create_timeseeded(0);

	universe_info_t info = {0};
	info.type_number = rg_uint(g_rg, 1, 6);
	unsigned int tn = info.type_number;
	unsigned int tnu = (rg_uint(g_rg, 0, 1) == 0) ?
		2 : rg_uint(g_rg, 1, (tn > 3) ? 3 : tn);

	GLuint buf_info_id;
	glGenBuffers(1, &buf_info_id);
	glBindBuffer(GL_ARRAY_BUFFER, buf_info_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(universe_info_t),
		&info, GL_STATIC_DRAW);

	part_type_t* type_table = xmalloc(tn * sizeof(part_type_t));
	
	for (unsigned int i = 0; i < tn; ++i)
	{
		type_table[i].br = rg_float(g_rg, 0.0f, 1.0f);
		type_table[i].bg = rg_float(g_rg, 0.0f, 1.0f);
		type_table[i].bb = rg_float(g_rg, 0.0f, 1.0f);
		type_table[i].sr = rg_float(g_rg, -100.0f, 100.0f);
		type_table[i].sg = rg_float(g_rg, -100.0f, 100.0f);
		type_table[i].sb = rg_float(g_rg, -100.0f, 100.0f);
		type_table[i].pr = rg_float(g_rg, -100.0f, 100.0f);
		type_table[i].pg = rg_float(g_rg, -100.0f, 100.0f);
		type_table[i].pb = rg_float(g_rg, -100.0f, 100.0f);
	}

	for (unsigned int i = 0; i < tn; ++i)
	for (unsigned int j = 0; j < CHANGE_TYPE_LAW_NUMBER; ++j)
	{
		change_type_law_t* ctl = &type_table[i].change_type_law_array[j];
		ctl->used = (tn > 1) * rg_uint(g_rg, 0, 1);
		if (!ctl->used) continue;
		ctl->has_speed_min = rg_uint(g_rg, 0, 1);
		ctl->has_speed_max = rg_uint(g_rg, 0, 1);
		ctl->has_pressure_min = rg_uint(g_rg, 0, 1);
		ctl->has_pressure_max = rg_uint(g_rg, 0, 1);
		ctl->has_age_min = rg_uint(g_rg, 0, 1);
		ctl->has_age_max = 0;//rg_uint(g_rg, 0, 1);
		ctl->speed_min = rg_float(g_rg, 0.0f, 0.02f);
		ctl->speed_max = rg_float(g_rg, 0.0f, 0.02f);
		ORDER(float, ctl->speed_min, ctl->speed_max);
		ctl->pressure_min = rg_float(g_rg, 0.0f, 1.5f);
		ctl->pressure_max = rg_float(g_rg, 0.0f, 1.5f);
		ORDER(float, ctl->pressure_min, ctl->pressure_max);
		ctl->age_min = rg_uint(g_rg, 0, 60*1);
		ctl->age_max = rg_uint(g_rg, 0, 60*1);
		ORDER(unsigned int, ctl->age_min, ctl->age_max);
		do {
			ctl->new_type = rg_uint(g_rg, 0, tn-1);
		} while (ctl->new_type == i);
		ctl->probability = rg_float(g_rg, 0.0f, 0.1f);
	}

	GLuint buf_type_id;
	glGenBuffers(1, &buf_type_id);
	glBindBuffer(GL_ARRAY_BUFFER, buf_type_id);
	glBufferData(GL_ARRAY_BUFFER, tn * sizeof(part_type_t),
		type_table, GL_STATIC_DRAW);

	/* Randomize colors */
	#if 0
	for (int i = 0; i < type_number; ++i)
	{
		type_table[i].br = rg_float(g_rg, 0.0f, 1.0f);
		type_table[i].bg = rg_float(g_rg, 0.0f, 1.0f);
		type_table[i].bb = rg_float(g_rg, 0.0f, 1.0f);
		type_table[i].sr = rg_float(g_rg, -1.0f, 1.0f);
		type_table[i].sg = rg_float(g_rg, -1.0f, 1.0f);
		type_table[i].sb = rg_float(g_rg, -1.0f, 1.0f);
		type_table[i].pr = rg_float(g_rg, -1.0f, 1.0f);
		type_table[i].pg = rg_float(g_rg, -1.0f, 1.0f);
		type_table[i].pb = rg_float(g_rg, -1.0f, 1.0f);
	}
	glBindBuffer(GL_ARRAY_BUFFER, buf_type_id);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		type_number * sizeof(part_type_t), type_table);
	#endif

	pil_set_t* pil_set_table =
		xmalloc(tn*tn * sizeof(pil_set_t));

	int continuous = 1;//rg_int(g_rg, 1, 5);
	for (unsigned int i = 0; i < tn; ++i)
	for (unsigned int j = 0; j < tn; ++j)
	{
		pil_set_t* pil_set = &pil_set_table[i * tn + j];

		/* attraction */
		pil_set->attraction.steps[0].offset = rg_int(g_rg, 0, 8) == 0 ?
			rg_float(g_rg, -0.0010f, 0.0010f) :
			rg_float(g_rg, 0.0003f, 0.0005f);
		pil_set->attraction.steps[0].slope = 0.0f;
		for (int s = 1; s < PISL_STEP_NUMBER; ++s)
		{
			if (rg_int(g_rg, 0, continuous) == 0)
			{
				if (rg_int(g_rg, 0, s) > 5)
				{
					pil_set->attraction.steps[s].offset = 0.0f;
					pil_set->attraction.steps[s].slope = 0.0f;
				}
				else
				{
					float e = (rg_int(g_rg, 0, 15) == 0) ?
						0.0007f / ((float)(s)) :
						0.0007f / ((float)(s*s));
					pil_set->attraction.steps[s].offset =
						rg_float(g_rg, -e/* *0.75f */, e);
					pil_set->attraction.steps[s].slope = 0.0f;
				}
			}
			else
			{
				pil_set->attraction.steps[s].offset =
					pil_set->attraction.steps[s-1].offset;
				pil_set->attraction.steps[s].slope = 0.0f;
			}
		}

		/* angle */
		pil_set->angle.steps[0].offset = 0.0f;
		pil_set->angle.steps[0].slope = 0.0f;
		for (int s = 1; s < PISL_STEP_NUMBER; ++s)
		{
			if (rg_int(g_rg, 0, continuous) == 0)
			{
				if (rg_int(g_rg, 0, 5) != 0)
				{
					pil_set->angle.steps[s].offset = 0.0f;
					pil_set->angle.steps[s].slope = 0.0f;
				}
				else
				{
					float e = (rg_int(g_rg, 0, 4) == 0) ?
						TAU/4.0f : 
						TAU/10.0f;
					pil_set->angle.steps[s].offset =
						rg_float(g_rg, -e, e);
					pil_set->angle.steps[s].slope = 0.0f;
				}
			}
			else
			{
				pil_set->angle.steps[s].offset =
					pil_set->angle.steps[s-1].offset;
				pil_set->angle.steps[s].slope = 0.0f;
			}
		}

		/* speed */
		pil_set->speed.steps[0].offset = (rg_int(g_rg, 0, 4) == 0) ?
			rg_float(g_rg, -0.0010f, 0.0010f) : 
			0.0f;
		pil_set->speed.steps[0].slope = 0.0f;
		for (int s = 1; s < PISL_STEP_NUMBER; ++s)
		{
			if (rg_int(g_rg, 0, continuous) == 0)
			{
				if (rg_int(g_rg, 0, 10) != 0)
				{
					pil_set->speed.steps[s].offset = 0.0f;
					pil_set->speed.steps[s].slope = 0.0f;
				}
				else
				{
					float e = (rg_int(g_rg, 0, 4) == 0) ?
						0.0007f / ((float)(s)) :
						0.0007f / ((float)(s*s));
					pil_set->speed.steps[s].offset =
						rg_float(g_rg, -e, e);
					pil_set->speed.steps[s].slope = 0.0f;
				}
			}
			else
			{
				pil_set->speed.steps[s].offset =
					pil_set->speed.steps[s-1].offset;
				pil_set->speed.steps[s].slope = 0.0f;
			}
		}
	}

	GLuint buf_pil_set_id;
	glGenBuffers(1, &buf_pil_set_id);
	glBindBuffer(GL_ARRAY_BUFFER, buf_pil_set_id);
	glBufferData(GL_ARRAY_BUFFER, tn*tn * sizeof(pil_set_t),
		pil_set_table, GL_STATIC_DRAW);

	#define PARTICLE_NUMBER (256 * 6)
	part_t part_array_a[PARTICLE_NUMBER] = {0};
	part_t part_array_b[PARTICLE_NUMBER] = {0};

	for (int i = 0; i < PARTICLE_NUMBER; ++i)
	{
		part_array_a[i].x = rg_float(g_rg, -1.0f, 1.0f);
		part_array_a[i].y = rg_float(g_rg, -1.0f, 1.0f);
		part_array_a[i].speed = 0.0f;
		part_array_a[i].angle = rg_float(g_rg, 0.0f, TAU);
		part_array_a[i].type = rg_uint(g_rg, 0, tnu-1);
		part_array_a[i].age = 0;
	}

	for (int i = 0; i < PARTICLE_NUMBER; ++i)
	{
		part_array_b[i] = part_array_a[i];
	}

	GLuint buf_part_curr_id;
	glGenBuffers(1, &buf_part_curr_id);
	glBindBuffer(GL_ARRAY_BUFFER, buf_part_curr_id);
	glBufferData(GL_ARRAY_BUFFER, PARTICLE_NUMBER * sizeof(part_t),
		part_array_a, GL_DYNAMIC_DRAW);

	GLuint buf_part_next_id;
	glGenBuffers(1, &buf_part_next_id);
	glBindBuffer(GL_ARRAY_BUFFER, buf_part_next_id);
	glBufferData(GL_ARRAY_BUFFER, PARTICLE_NUMBER * sizeof(part_t),
		part_array_b, GL_DYNAMIC_DRAW);

	#define WORK_GROUP_SIZE 256

	setting_set_fade_factor(0.05f);

	int running = 1;
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					running = 0;
				break;
				case SDL_MOUSEBUTTONDOWN:
					setting_set_fade_factor(((float)event.button.x)/800.0f);
				break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							running = 0;
						break;
						case SDLK_c:
							;
						break;
					}
				break;
			}
		}

		glViewport(0, 0, 800, 800);
		glUseProgram(g_shprog_comp_iteruniv);
		
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buf_part_curr_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buf_part_next_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buf_type_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, buf_pil_set_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, buf_info_id);

		glDispatchCompute(PARTICLE_NUMBER / WORK_GROUP_SIZE, 1, 1);

		glUseProgram((GLuint)0);

		glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

		SWAP(GLuint, buf_part_curr_id, buf_part_next_id);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
		glBlendFunc(GL_ONE, GL_ONE);
		glUseProgram(g_shprog_draw_fade);
		glDrawArrays(GL_POINTS, 0, 1);
		glUseProgram((GLuint)0);
		glDisable(GL_BLEND);

		#if 0
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		#endif

		#define ATTRIB_LOCATION_POS ((GLuint)0)
		#define ATTRIB_LOCATION_COLOR ((GLuint)1)
		#define ATTRIB_LOCATION_ANGLE ((GLuint)2)
		#define ATTRIB_LOCATION_OLDPOS ((GLuint)3)

		glViewport(0, 0, 800, 800);
		glUseProgram(g_shprog_draw_particles);
		glEnableVertexAttribArray(ATTRIB_LOCATION_POS);
		glEnableVertexAttribArray(ATTRIB_LOCATION_COLOR);
		glEnableVertexAttribArray(ATTRIB_LOCATION_ANGLE);
		glEnableVertexAttribArray(ATTRIB_LOCATION_OLDPOS);
		
		glBindBuffer(GL_ARRAY_BUFFER, buf_part_curr_id);
		glVertexAttribPointer(ATTRIB_LOCATION_POS, 2, GL_FLOAT,
			GL_FALSE, sizeof(part_t), (void*)offsetof(part_t, x));
		glVertexAttribPointer(ATTRIB_LOCATION_COLOR, 3, GL_FLOAT,
			GL_FALSE, sizeof(part_t), (void*)offsetof(part_t, r));
		glVertexAttribPointer(ATTRIB_LOCATION_ANGLE, 1, GL_FLOAT,
			GL_FALSE, sizeof(part_t), (void*)offsetof(part_t, angle));
		glVertexAttribPointer(ATTRIB_LOCATION_OLDPOS, 2, GL_FLOAT,
			GL_FALSE, sizeof(part_t), (void*)offsetof(part_t, oldx));

		glDrawArrays(GL_POINTS, 0, PARTICLE_NUMBER);
		
		glDisableVertexAttribArray(ATTRIB_LOCATION_POS);
		glDisableVertexAttribArray(ATTRIB_LOCATION_COLOR);
		glDisableVertexAttribArray(ATTRIB_LOCATION_ANGLE);
		glDisableVertexAttribArray(ATTRIB_LOCATION_OLDPOS);
		glUseProgram((GLuint)0);

		#undef ATTRIB_LOCATION_POS
		#undef ATTRIB_LOCATION_COLOR
		#undef ATTRIB_LOCATION_ANGLE
		#undef ATTRIB_LOCATION_OLDPOS

		SDL_GL_SwapWindow(g_window);
	}

	rg_destroy(g_rg);
	cleanup_g_graphics();
	return 0;
}
