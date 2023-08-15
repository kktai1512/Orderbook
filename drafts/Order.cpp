#include "Order.h"
#include "Limit.h"

Order::Order(int id, int side, int quantity, int limit)
    : id(id), side(side), quantity(quantity), limit(limit),
      nextOrder(nullptr), prevOrder(nullptr), parentLimit(nullptr) {}

void Order::setNextOrder(Order* order) {
    nextOrder = order;
}

void Order::setPrevOrder(Order* order) {
    prevOrder = order;
}

void Order::setParentLimit(Limit* limit) {
    parentLimit = limit;
}