/**
 * @file Enemy.h
 * @brief Enemy AI helper declarations for the snake game.
 *
 * This file declares the EnemyAI class, which is used to determine
 * enemy snake movement on high difficulty modes.
 *
 * @author Atabong Eyambe Mokom
 */
#pragma once

#include "Model.h"

/**
 * @brief Provides AI movement logic for an enemy snake.
 *
 * The EnemyAI class computes a suggested movement direction for the
 * enemy snake, using pathfinding and fallback safety rules.
 *
 * @author Atabong Eyambe Mokom
 */
class EnemyAI {
public:
    /**
     * @brief Chooses the next direction for the enemy snake.
     *
     * This method is intended to use BFS shortest path logic to move
     * toward food while avoiding both snakes. If no path exists, it
     * falls back to a safe movement that maximizes available space.
     *
     * @param board The current game board.
     * @param enemy The enemy snake.
     * @param player The player snake.
     * @param food The position of the food.
     * @param wrapWalls Whether wall wrapping is enabled.
     * @return The direction the enemy snake should move next.
     *
     * @author Atabong Eyambe Mokom
     */
    static Dir ChooseDir(const Board& board,
                         const Snake& enemy,
                         const Snake& player,
                         const Point& food,
                         bool wrapWalls);
};