#include "controller/controller.hpp"
#include <iostream>

namespace controller {

Controller::Controller()
{
	std::cout << "Controller constructed" << std::endl;
}

Controller::~Controller()
{
	std::cout << "Controller destructed" << std::endl;
}

void Controller::handleInput(const InputState &input)
{
	std::cout << "Handling input: "
	          << "Left: " << input.left << ", "
	          << "Right: " << input.right << ", "
	          << "Up: " << input.up << ", "
	          << "Down: " << input.down << ", "
	          << "Mouse Left: " << input.mouseLeft << ", "
	          << "Mouse Right: " << input.mouseRight << ", "
	          << "Mouse Middle: " << input.mouseMiddle << ", "
	          << "Mouse X: " << input.mouseX << ", "
	          << "Mouse Y: " << input.mouseY << std::endl;
}

void controller::Controller::update(float dt)
{
	std::cout << "Updating controller with dt: " << dt << std::endl;
}

} // namespace controller
