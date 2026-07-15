/**
 * @file Model.h
 * @brief Core gameplay model definitions for the snake game.
 *
 * This file contains the fundamental data structures and helper functions
 * used by the snake game, including positions, movement directions, board
 * boundaries, snake state, and snake type configuration.
 *
 * @author Atabong Eyambe Mokom
 */
#pragma once

#include <deque>
#include <algorithm>

/**
 * @brief Represents a single point on the game board.
 *
 * A Point stores the x and y coordinates of an object on the grid,
 * such as a snake segment or food item.
 *
 * @author Atabong Eyambe Mokom
 */
struct Point {
    int x = 0; ///< X-coordinate on the board.
    int y = 0; ///< Y-coordinate on the board.

    /**
     * @brief Compares two points for equality.
     *
     * Two points are equal if both their x and y coordinates match.
     *
     * @param o The point to compare with.
     * @return true if both points have the same coordinates.
     * @return false otherwise.
     *
     * @author Atabong Eyambe Mokom
     */
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
};

/**
 * @brief Represents the four possible movement directions for a snake.
 *
 * @author Atabong Eyambe Mokom
 */
enum class Dir { Up, Down, Left, Right };

/**
 * @brief Checks whether two directions are opposites.
 *
 * This is useful to prevent the snake from reversing directly into itself.
 *
 * @param a The first direction.
 * @param b The second direction.
 * @return true if the directions are opposites.
 * @return false otherwise.
 *
 * @author Atabong Eyambe Mokom
 */
static inline bool IsOpposite(Dir a, Dir b) {
    return (a == Dir::Up && b == Dir::Down) || (a == Dir::Down && b == Dir::Up) ||
           (a == Dir::Left && b == Dir::Right) || (a == Dir::Right && b == Dir::Left);
}

/**
 * @brief Represents the dimensions of the game board.
 *
 * The Board structure stores the width and height of the playable area
 * and provides a helper function for boundary checking.
 *
 * @author Atabong Eyambe Mokom
 */
struct Board {
    int w = 30; ///< Width of the board in cells.
    int h = 20; ///< Height of the board in cells.

    /**
     * @brief Checks whether a point lies inside the board.
     *
     * @param p The point to test.
     * @return true if the point is within the board bounds.
     * @return false otherwise.
     *
     * @author Atabong Eyambe Mokom
     */
    bool IsInside(const Point& p) const { return p.x >= 0 && p.y >= 0 && p.x < w && p.y < h; }
};

/**
 * @brief Represents the snake and its state.
 *
 * The Snake structure tracks the snake body, its current movement direction,
 * and whether it should grow on the next move.
 *
 * @author Atabong Eyambe Mokom
 */
struct Snake {
    std::deque<Point> body; ///< Body segments of the snake, with the head at the front.
    Dir dir = Dir::Right;   ///< Current movement direction.
    bool growPending = false; ///< Indicates whether the snake should grow on the next move.

    /**
     * @brief Checks whether the snake occupies a given point.
     *
     * @param p The point to check.
     * @return true if the point is occupied by the snake.
     * @return false otherwise.
     *
     * @author Atabong Eyambe Mokom
     */
    bool Occupies(const Point& p) const {
        return std::find(body.begin(), body.end(), p) != body.end();
    }

    /**
     * @brief Returns the current head position of the snake.
     *
     * @return The first segment in the snake body.
     *
     * @author Atabong Eyambe Mokom
     */
    Point Head() const { return body.front(); }

    /**
     * @brief Calculates the next head position based on the current direction.
     *
     * @return The next position the snake head will move to.
     *
     * @author Atabong Eyambe Mokom
     */
    Point NextHead() const {
        Point n = Head();
        switch (dir) {
            case Dir::Up:    n.y -= 1; break;
            case Dir::Down:  n.y += 1; break;
            case Dir::Left:  n.x -= 1; break;
            case Dir::Right: n.x += 1; break;
        }
        return n;
    }

    /**
     * @brief Sets the movement direction of the snake.
     *
     * The direction is only changed if the new direction is not the
     * opposite of the current one.
     *
     * @param nd The new direction.
     *
     * @author Atabong Eyambe Mokom
     */
    void SetDir(Dir nd) { if (!IsOpposite(dir, nd)) dir = nd; }

    /**
     * @brief Marks the snake to grow on its next advance.
     *
     * @author Atabong Eyambe Mokom
     */
    void Grow() { growPending = true; }

    /**
     * @brief Advances the snake to a new head position.
     *
     * A new head segment is added to the front of the body. If growth
     * is not pending, the tail segment is removed. Otherwise, the snake
     * increases in length by one segment.
     *
     * @param newHead The new head position.
     *
     * @author Atabong Eyambe Mokom
     */
    void AdvanceTo(const Point& newHead) {
        body.push_front(newHead);
        if (!growPending) body.pop_back();
        else growPending = false;
    }
};

/**
 * @brief Represents the available snake types.
 *
 * These types determine movement speed, growth rate, special abilities,
 * and other gameplay properties.
 *
 * @author Atabong Eyambe Mokom
 */
enum class SnakeType { Classic, Speedy, Tank, Ghost };

/**
 * @brief Stores gameplay information for a snake type.
 *
 * SnakeTypeInfo defines how a particular snake type behaves, including
 * movement speed, growth amount, wall wrapping, and special forgiveness
 * rules for collisions.
 *
 * @author Atabong Eyambe Mokom
 */
struct SnakeTypeInfo {
    int baseTickMs = 120;      ///< Base speed in milliseconds before difficulty scaling.
    int growthPerFood = 1;     ///< Number of segments gained per food item.
    bool defaultWrap = false;  ///< Whether this snake type wraps through walls by default.

    int armorWallHits = 0;     ///< Number of wall collisions the snake can survive.
    int selfBiteForgives = 0;  ///< Number of self-collisions forgiven.
    int wallScorePenalty = 0;  ///< Score penalty for surviving a wall collision.

    const char* desc = "Balanced."; ///< Short description of the snake type.
};

/**
 * @brief Returns the configuration information for a snake type.
 *
 * @param t The selected snake type.
 * @return A SnakeTypeInfo structure containing the type settings.
 *
 * @author Atabong Eyambe Mokom
 */
static inline SnakeTypeInfo GetSnakeTypeInfo(SnakeType t) {
    switch (t) {
        case SnakeType::Classic:
            return {120, 1, false, 0, 0, 0, "Balanced. Standard speed and growth."};
        case SnakeType::Speedy:
            return { 80, 1, false, 0, 0, 0, "Fast movement (shorter tick). Harder to control."};
        case SnakeType::Tank:
            return {150, 2, false, 1, 1, 3, "Slow & sturdy. +2 growth. 1 wall hit (-3) and 1 self-bite warning."};
        case SnakeType::Ghost:
            return {120, 1, true,  0, 0, 0, "Wraps through walls by default."};
    }
    return {120, 1, false, 0, 0, 0, "Balanced."};
}

/**
 * @brief Returns the display name of a snake type.
 *
 * @param t The snake type.
 * @return A string containing the name of the snake type.
 *
 * @author Atabong Eyambe Mokom
 */
static inline const char* SnakeTypeName(SnakeType t) {
    switch (t) {
        case SnakeType::Classic: return "Classic";
        case SnakeType::Speedy:  return "Speedy";
        case SnakeType::Tank:    return "Tank";
        case SnakeType::Ghost:   return "Ghost";
    }
    return "Classic";
}