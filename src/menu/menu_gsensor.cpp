#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "menu.h"
#include "sysconfig.h"
#include "sysdeps.h"
#include "uae.h"
#include "options.h"
#include "sound.h"
#include "joystick.h"

#include <SDL.h>

extern SDL_Joystick *uae4all_joy0, *uae4all_joy1, *gcw0_gsensor;

static const char *text_str_title_gsensor_menu="G-sensor options";
static const char *text_str_status="Status:";
static const char *text_str_off="off";
static const char *text_str_not_detected="not detected";
static const char *text_str_enable="Enable g-sensor";
static const char *text_str_disable="Disable g-sensor";
static const char *text_str_deadzone="Dead-zone";
static const char *text_str_small="small";
static const char *text_str_medium="medium";
static const char *text_str_large="large";
static const char *text_str_calibrate="Calibrate";
static const char *text_str_back="Main Menu (B)";
static const char *text_str_separator="------------------------------";

enum GsensorMenuEntry {
	GSENSOR_MENU_ENTRY_NONE = -1 /* pseudo-entry */,
	GSENSOR_MENU_ENTRY_ACTIVATE,
	GSENSOR_MENU_ENTRY_DEADZONE,
	GSENSOR_MENU_ENTRY_CALIBRATE,
	GSENSOR_MENU_ENTRY_BACK,
	GSENSOR_MENU_ENTRY_COUNT, /* the number of entries to be shown */
};

int gsensorMenu_vpos=1;

int gsensorMenu_enabled = 0;
int gsensorMenu_deadzone = 1;


static void draw_gsensorMenu(enum GsensorMenuEntry c)
{
	static int frame = 0;
	int flash = frame / 3;
	int row = 4, column = 0;
	char coords[30];

	text_draw_background();

	text_draw_window(40,28,260,192,text_str_title_gsensor_menu);

	write_text(6, row, text_str_status);
	if (gsensorMenu_enabled)
	{
		// Update the g-sensor position information.
		gsensor_pos_x = SDL_JoystickGetAxis (gcw0_gsensor, 0) - gsensor_center_x;
		gsensor_pos_y = SDL_JoystickGetAxis (gcw0_gsensor, 1) - gsensor_center_y;

		sprintf(coords, "on  x:%6d y:%6d\n", gsensor_pos_x, gsensor_pos_y);
		write_text(14, row++, coords);
	}
	else
	{
		if(gcw0_gsensor)
			write_text(14, row++, text_str_off);
		else
			write_text(14, row++, text_str_not_detected);
	}
	row++;

	row += 2;

	if (c == GSENSOR_MENU_ENTRY_ACTIVATE && flash)
	{
		if (gsensorMenu_enabled)
			write_text_inv(6, row++, text_str_disable);
		else
			write_text_inv(6, row++, text_str_enable);
	}
	else
	{
		if (gsensorMenu_enabled)
			write_text(6, row++, text_str_disable);
		else
			write_text(6, row++, text_str_enable);
	}

	row++;

	if (gsensorMenu_enabled)
	{
		write_text(6, row, text_str_deadzone);

		column = 17;

		if ((gsensorMenu_deadzone == 0) && (c != GSENSOR_MENU_ENTRY_DEADZONE || flash))
			write_text_inv(column, row, text_str_small);
		else
			write_text(column, row, text_str_small);
		column += strlen(text_str_small) + 1;
		if ((gsensorMenu_deadzone == 1) && (c != GSENSOR_MENU_ENTRY_DEADZONE || flash))
			write_text_inv(column, row, text_str_medium);
		else
			write_text(column, row, text_str_medium);
		column += strlen(text_str_medium) + 1;
		if ((gsensorMenu_deadzone == 2) && (c != GSENSOR_MENU_ENTRY_DEADZONE || flash))
			write_text_inv(column, row++, text_str_large);
		else
			write_text(column, row++, text_str_large);
		column += strlen(text_str_large) + 1;
	}
	else
	{
		row++;
	}

	row++;

	if (gsensorMenu_enabled)
	{
		if (c == GSENSOR_MENU_ENTRY_CALIBRATE && flash)
			write_text_inv(6, row, text_str_calibrate);
		else
			write_text(6, row, text_str_calibrate);

		sprintf(coords, "(cx:%d cy:%d)\n", gsensor_center_x, gsensor_center_y);
		write_text(16, row++, coords);
	}
	else
	{
		row++;
	}

	write_text(6, row++, text_str_separator);

	if (c == GSENSOR_MENU_ENTRY_BACK && flash)
		write_text_inv(6, row++, text_str_back);
	else
		write_text(6, row++, text_str_back);

	text_flip();
	frame = (frame + 1) % 6;
}

static enum GsensorMenuEntry key_gsensorMenu(enum GsensorMenuEntry *sel)
{
	SDL_Event event;

	while (SDL_PollEvent(&event) > 0)
	{
		int left = 0, right = 0, up = 0, down = 0,
		    activate = 0, cancel = 0;
		if (event.type == SDL_QUIT)
			return GSENSOR_MENU_ENTRY_BACK;
		else if (event.type == SDL_KEYDOWN)
		{
			uae4all_play_click();
			switch(event.key.keysym.sym)
			{
				case SDLK_d:
				case SDLK_RIGHT: right = 1; break;
				case SDLK_a:
				case SDLK_LEFT: left = 1; break;
				case SDLK_w:
				case SDLK_UP: up = 1; break;
				case SDLK_s:
				case SDLK_DOWN: down = 1; break;
				case SDLK_z:
				case SDLK_RETURN:
				case SDLK_e:
				case SDLK_LCTRL: activate = 1; break;
				case SDLK_q:
				case SDLK_LALT: cancel = 1; break;
			}
			if (cancel)
				return GSENSOR_MENU_ENTRY_BACK;
			else if (up)
			{
				if (*sel > 0) *sel = (enum GsensorMenuEntry) ((*sel - 1) % GSENSOR_MENU_ENTRY_COUNT);
				else *sel = (enum GsensorMenuEntry) (GSENSOR_MENU_ENTRY_COUNT - 1);

				if(!gsensorMenu_enabled && (*sel == GSENSOR_MENU_ENTRY_CALIBRATE || *sel == GSENSOR_MENU_ENTRY_DEADZONE))
				{
					if (*sel > 0) *sel = (enum GsensorMenuEntry) ((*sel - 2) % GSENSOR_MENU_ENTRY_COUNT);
					else *sel = (enum GsensorMenuEntry) (GSENSOR_MENU_ENTRY_COUNT - 1);
				}
			}
			else if (down)
			{
				*sel = (enum GsensorMenuEntry) ((*sel + 1) % GSENSOR_MENU_ENTRY_COUNT);

				if(!gsensorMenu_enabled && (*sel == GSENSOR_MENU_ENTRY_CALIBRATE || *sel == GSENSOR_MENU_ENTRY_DEADZONE))
				{
					*sel = (enum GsensorMenuEntry) ((*sel + 2) % GSENSOR_MENU_ENTRY_COUNT);
				}
			}
			else
			{
				switch (*sel)
				{
					case GSENSOR_MENU_ENTRY_DEADZONE:
						if (left)
							gsensorMenu_deadzone = (gsensorMenu_deadzone > 0)
								? gsensorMenu_deadzone - 1
								: 2;
						else if (right)
							gsensorMenu_deadzone = (gsensorMenu_deadzone < 2)
								? gsensorMenu_deadzone + 1
								: 0;

						// Update deadzones.
						if (gcw0_gsensor)
						{
							struct joy_range *dzone;

							if(gcw0_gsensor == uae4all_joy0)
								dzone = &dzone0;
							else if(gcw0_gsensor == uae4all_joy1)
								dzone = &dzone1;
							else
								break;

							if (gsensorMenu_deadzone == 0)
							{
								dzone->minx = 500;
								dzone->maxx = 500;
								dzone->miny = 500;
								dzone->maxy = 500;
							}
							else if (gsensorMenu_deadzone == 1)
							{
								dzone->minx = 1000;
								dzone->maxx = 1000;
								dzone->miny = 1000;
								dzone->maxy = 1000;
							}
							else if (gsensorMenu_deadzone == 2)
							{
								dzone->minx = 2000;
								dzone->maxx = 2000;
								dzone->miny = 2000;
								dzone->maxy = 2000;
							}
						}
						break;
					case GSENSOR_MENU_ENTRY_ACTIVATE:
					case GSENSOR_MENU_ENTRY_CALIBRATE:
					case GSENSOR_MENU_ENTRY_BACK:
						if (activate)
							return *sel;
						break;
				}
			}
		}
	}

	return GSENSOR_MENU_ENTRY_NONE;
}

static void clear_events(void)
{
	SDL_Event event;
	while (SDL_PollEvent(&event));
}

static void raise_gsensorMenu()
{
	int i;

	text_draw_background();
	text_flip();
	for(i=0;i<10;i+=2)
	{
		text_draw_background();
		text_draw_window(40,(10-i)*24,260,200,text_str_title_gsensor_menu);
		text_flip();
	}
	clear_events();
}

static void unraise_gsensorMenu()
{
	int i;

	for(i=9;i>=0;i-=2)
	{
		text_draw_background();
		text_draw_window(40,(10-i)*24,260,200,text_str_title_gsensor_menu);
		text_flip();
	}
	text_draw_background();
	text_flip();
	clear_events();
}

int run_menuGsensorSel()
{
#if defined(AUTO_RUN) || defined(AUTO_FRAMERATE) || defined(AUTO_PROFILER)
	return 1;
#else
	static enum GsensorMenuEntry c = GSENSOR_MENU_ENTRY_ACTIVATE;

	while (1)
	{
		enum GsensorMenuEntry action = GSENSOR_MENU_ENTRY_NONE;
		raise_gsensorMenu();
		while (action == GSENSOR_MENU_ENTRY_NONE)
		{
			draw_gsensorMenu(c);
			action = key_gsensorMenu(&c);
		}
		unraise_gsensorMenu();
		switch (action)
		{
			case GSENSOR_MENU_ENTRY_ACTIVATE:
				if(gcw0_gsensor)
					gsensorMenu_enabled = !gsensorMenu_enabled;
				break;
			case GSENSOR_MENU_ENTRY_CALIBRATE:
				calibrate_gsensor();
				break;
			case GSENSOR_MENU_ENTRY_BACK:
				return 1; /* leave, returning to main menu */
		}
	}
#endif
}

