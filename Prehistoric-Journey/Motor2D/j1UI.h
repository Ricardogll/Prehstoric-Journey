#ifndef __j1UI_H__
#define __j1UI_H__

#include "j1Module.h"
#include "p2DynArray.h"
#include "j1Fonts.h"
#include "p2SString.h"
#include "j1Render.h"

#define CURSOR_WIDTH 2


enum UIType {

	LABEL,
	BUTTON,
	IMAGE,
	WINDOW,
	SLIDER,
	NONE_UI

};

class UIElement;

class j1UI : public j1Module
{

public:

	j1UI();

	// Destructor
	virtual ~j1UI();

	// Called when before render is available
	bool Awake(pugi::xml_node&);

	// Call before first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	bool Update(float dt);

	// Called after all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	void DeleteUIElements();
	void DeleteUIElementChildren(UIElement* ui_el);
	void SetVisibleChildren(UIElement* ui_el);

	// Gui creation functions
	UIElement* CreateLabel(int x, int y, const char* text, int width = -1, int size = DEFAULT_FONT_SIZE, SDL_Color color = { 255,255,255,255 }, const char* font = DEFAULT_FONT, UIElement* parent = nullptr);
	UIElement* CreateImage(int x, int y, SDL_Rect rect, UIElement* parent = nullptr);
	UIElement* CreateButton(int x, int y, SDL_Rect rect, SDL_Rect hovering, SDL_Rect clicked, UIElement* parent = nullptr);
	UIElement* CreateSlider(int x, int y, float cur_value, float max_value, SDL_Rect bar, SDL_Rect ball, UIElement* parent = nullptr);

	const SDL_Texture* GetAtlas() const;


public:

	uint fx_click = 0u;

private:
	bool debug = false;

	p2DynArray<UIElement*> ui_elements;
	SDL_Texture* atlas=nullptr;

	p2SString atlas_file_name;
};

#endif //