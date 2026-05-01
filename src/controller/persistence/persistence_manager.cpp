#include "controller/persistence/persistence_manager.hpp"
#include <iostream>

namespace controller {

PersistenceManager::PersistenceManager()
{
    std::cout << "PersistenceManager constructed" << std::endl;
}

PersistenceManager::~PersistenceManager()
{
    std::cout << "PersistenceManager destructed" << std::endl;
}

} // namespace controller