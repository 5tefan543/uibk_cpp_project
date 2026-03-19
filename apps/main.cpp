#include "controller/input_state.hpp"
#include "ui/ui.hpp"
#include <cstdlib>
#include <iostream>

void printInputState(const controller::InputState &input)
{
	std::cout << "Input State: "
	          << "Left: " << input.left << ", "
	          << "Right: " << input.right << ", "
	          << "Up: " << input.up << ", "
	          << "Down: " << input.down << ", "
	          << "Mouse Left: " << input.mouse_left << ", "
	          << "Mouse Right: " << input.mouse_right << ", "
	          << "Mouse Middle: " << input.mouse_middle << ", "
	          << "Mouse X: " << input.mouse_x << ", "
	          << "Mouse Y: " << input.mouse_y << std::endl;
}

int main()
{
	ui::UI ui;

	while (ui.isOpen()) {
		controller::InputState input = ui.pollInput();
		printInputState(input);
		ui.render();
	}

	return EXIT_SUCCESS;
}