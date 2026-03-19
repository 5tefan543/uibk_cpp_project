#pragma once

namespace controller {

struct InputState {
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	bool mouse_left = false;
	bool mouse_right = false;
	bool mouse_middle = false;
	float mouse_x = 0.0f;
	float mouse_y = 0.0f;
};

} // namespace controller