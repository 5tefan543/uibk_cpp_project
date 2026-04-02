#pragma once

#include <memory>
#include <vector>

#include "controller/state/state.hpp"

namespace controller {

class StateStack {
  private:
    std::vector<std::unique_ptr<BaseState>> states;

  public:
    void push(std::unique_ptr<BaseState> state);
    void pop();
    BaseState &current();
    bool empty() const;
    void clear();
    void replaceCurrent(std::unique_ptr<BaseState> state);
    void printStack() const;
};

} // namespace controller