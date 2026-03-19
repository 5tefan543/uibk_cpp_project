#pragma once

namespace controller {

struct InputState {
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	bool mouseLeft = false;
	bool mouseRight = false;
	bool mouseMiddle = false;
	float mouseX = 0.0f;
	float mouseY = 0.0f;
};

} // namespace controller