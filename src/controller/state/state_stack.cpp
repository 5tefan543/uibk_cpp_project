#include <iostream>
#include <utility>

#include "controller/state/state_stack.hpp"

namespace controller {

void StateStack::push(std::unique_ptr<BaseState> state)
{
    states.push_back(std::move(state));
    printStack();
}

void StateStack::pop()
{
    states.pop_back();
    printStack();
}

BaseState &StateStack::current()
{
    return *states.back();
}

bool StateStack::empty() const
{
    return states.empty();
}

void StateStack::clear()
{
    states.clear();
    printStack();
}

void StateStack::replaceCurrent(std::unique_ptr<BaseState> state)
{
    states.back() = std::move(state);
    printStack();
}

void StateStack::printStack() const
{
    std::cout << "State Stack: [";
    for (auto it = states.begin(); it != states.end(); ++it) {

        std::cout << toString((*it)->type);

        if (std::next(it) != states.end()) {
            std::cout << " -> ";
        }
    }
    std::cout << "]\n";
}

} // namespace controller