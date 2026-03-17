#include "mylib/foo.hpp"
#include <SFML/Graphics.hpp>

namespace mylib {

int foo()
{

	sf::RenderWindow window(sf::VideoMode({200, 200}), "SFML works!");
	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);

	while (window.isOpen()) {
		while (const std::optional event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>())
				window.close();
		}

		window.clear();
		window.draw(shape);
		window.display();
	}
	return 42;
}

int foo2()
{
	return 42;
}

} // end namespace mylib
